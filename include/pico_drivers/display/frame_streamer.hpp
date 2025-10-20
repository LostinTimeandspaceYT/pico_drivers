#ifndef PICO_DRIVERS_DISPLAY_FRAME_STREAMER_HPP_
#define PICO_DRIVERS_DISPLAY_FRAME_STREAMER_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include <pico/assert.h>
#include <pico/platform.h>

#include <pico_drivers/i2c/i2c.hpp>

class FrameTransport {
  public:
    virtual ~FrameTransport() = default;
    virtual void resize(std::size_t max_frame_size) = 0;
    virtual void start_transfer(const uint8_t *frame, uint16_t length) = 0;
};

class FrameStreamer {
  public:
    explicit FrameStreamer(std::unique_ptr<FrameTransport> transport)
        : transport_(std::move(transport)) {
        hard_assert(transport_ != nullptr);
    }

    FrameStreamer(FrameStreamer &&) noexcept = default;
    FrameStreamer &operator=(FrameStreamer &&) noexcept = default;

    FrameStreamer(const FrameStreamer &) = delete;
    FrameStreamer &operator=(const FrameStreamer &) = delete;

    void resize(std::size_t max_frame_size) {
        if (transport_ != nullptr) {
            transport_->resize(max_frame_size);
        }
    }

    void start_transfer(const uint8_t *frame, uint16_t length) {
        if (transport_ != nullptr) {
            transport_->start_transfer(frame, length);
        }
    }

  private:
    std::unique_ptr<FrameTransport> transport_;
};

class I2CFrameTransport : public FrameTransport {
  public:
    I2CFrameTransport(I2C &bus, uint8_t device_address, std::size_t max_frame_size)
        : bus_(bus), device_address_(device_address), dma_frame_buffer_(max_frame_size + 1u, 0) {}

    ~I2CFrameTransport() override { deinit_dma(); }

    void resize(std::size_t max_frame_size) override {
        dma_frame_buffer_.assign(max_frame_size + 1u, 0);
    }

    void start_transfer(const uint8_t *frame, uint16_t length) override {
        if (frame == nullptr || length == 0) {
            return;
        }
        if (!dma_initialized_) {
            init_dma();
        }

        hard_assert(length <= dma_frame_buffer_.size());

        i2c_inst_t *inst = bus_.handle();
        i2c_hw_t *hw = i2c_get_hw(inst);

        while (hw->status & I2C_IC_STATUS_MST_ACTIVITY_BITS) {
            tight_loop_contents();
        }

        (void)hw->clr_tx_abrt;
        hw->tar = device_address_;

        dma_control_word_ = I2C_IC_DATA_CMD_RESTART_BITS | 0x40;

        for (uint16_t i = 0; i < length; ++i) {
            uint16_t word = frame[i];
            if (i == length - 1) {
                word |= I2C_IC_DATA_CMD_STOP_BITS;
            }
            dma_frame_buffer_[i] = word;
        }

        dma_channel_config data_cfg = dma_channel_get_default_config(dma_data_channel_);
        channel_config_set_transfer_data_size(&data_cfg, DMA_SIZE_16);
        channel_config_set_read_increment(&data_cfg, true);
        channel_config_set_write_increment(&data_cfg, false);
        channel_config_set_dreq(&data_cfg, i2c_get_dreq(inst, true));
        dma_channel_configure(dma_data_channel_, &data_cfg, &hw->data_cmd, dma_frame_buffer_.data(),
                              length, false);

        dma_channel_config ctrl_cfg = dma_channel_get_default_config(dma_control_channel_);
        channel_config_set_transfer_data_size(&ctrl_cfg, DMA_SIZE_16);
        channel_config_set_read_increment(&ctrl_cfg, false);
        channel_config_set_write_increment(&ctrl_cfg, false);
        channel_config_set_dreq(&ctrl_cfg, i2c_get_dreq(inst, true));
        channel_config_set_chain_to(&ctrl_cfg, dma_data_channel_);
        dma_channel_configure(dma_control_channel_, &ctrl_cfg, &hw->data_cmd, &dma_control_word_, 1,
                              false);

        dma_start_channel_mask(1u << dma_control_channel_);
        dma_channel_wait_for_finish_blocking(dma_data_channel_);
        dma_channel_wait_for_finish_blocking(dma_control_channel_);

        while (hw->status & I2C_IC_STATUS_MST_ACTIVITY_BITS) {
            tight_loop_contents();
        }
        (void)hw->clr_stop_det;
    }

  private:
    I2C &bus_;
    uint8_t device_address_;
    int dma_control_channel_ = -1;
    int dma_data_channel_ = -1;
    bool dma_initialized_ = false;
    uint16_t dma_control_word_ = 0;
    std::vector<uint16_t> dma_frame_buffer_;

    void init_dma() {
        if (dma_initialized_) {
            return;
        }
        dma_control_channel_ = dma_claim_unused_channel(true);
        dma_data_channel_ = dma_claim_unused_channel(true);
        dma_initialized_ = true;
    }

    void deinit_dma() {
        if (!dma_initialized_) {
            return;
        }
        if (dma_control_channel_ >= 0) {
            dma_channel_unclaim(dma_control_channel_);
            dma_control_channel_ = -1;
        }
        if (dma_data_channel_ >= 0) {
            dma_channel_unclaim(dma_data_channel_);
            dma_data_channel_ = -1;
        }
        dma_initialized_ = false;
    }
};

class SPIFrameTransport : public FrameTransport {
  public:
    SPIFrameTransport(spi_inst_t *spi, uint cs_pin, std::size_t max_frame_size,
                      bool drive_cs = true)
        : spi_(spi), cs_pin_(cs_pin), drive_cs_(drive_cs), staging_(max_frame_size, 0) {
        if (drive_cs_) {
            gpio_init(cs_pin_);
            gpio_set_dir(cs_pin_, GPIO_OUT);
            gpio_put(cs_pin_, 1);
        }
    }

    void resize(std::size_t max_frame_size) override { staging_.assign(max_frame_size, 0); }

    void start_transfer(const uint8_t *frame, uint16_t length) override {
        if (frame == nullptr || length == 0) {
            return;
        }
        if (drive_cs_) {
            gpio_put(cs_pin_, 0);
        }

        spi_write_blocking(spi_, frame, length);

        if (drive_cs_) {
            gpio_put(cs_pin_, 1);
        }
    }

  private:
    spi_inst_t *spi_;
    uint cs_pin_;
    bool drive_cs_;
    std::vector<uint8_t> staging_;
};

#endif // PICO_DRIVERS_DISPLAY_FRAME_STREAMER_HPP_
