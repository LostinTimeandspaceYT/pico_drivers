#ifndef PICO_DRIVERS_DISPLAY_FRAME_BUFFER_HPP_
#define PICO_DRIVERS_DISPLAY_FRAME_BUFFER_HPP_

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

class FrameBuffer {
  public:
    FrameBuffer() = default;
    explicit FrameBuffer(std::size_t size_bytes) { resize(size_bytes); }

    void resize(std::size_t size_bytes) {
        size_ = size_bytes;
        for (auto &buffer : buffers_) {
            buffer.assign(size_bytes, 0);
        }
        draw_index_ = 0;
        display_index_ = 0;
        double_buffer_enabled_ = false;
    }

    std::size_t size() const { return size_; }

    uint8_t *back_buffer() { return size_ == 0 ? nullptr : buffers_[draw_index_].data(); }

    const uint8_t *back_buffer() const {
        return size_ == 0 ? nullptr : buffers_[draw_index_].data();
    }

    uint8_t *front_buffer() { return size_ == 0 ? nullptr : buffers_[display_index_].data(); }

    const uint8_t *front_buffer() const {
        return size_ == 0 ? nullptr : buffers_[display_index_].data();
    }

    void clear(uint8_t value = 0) {
        if (size_ == 0) {
            return;
        }
        auto &back = buffers_[draw_index_];
        std::fill(back.begin(), back.end(), value);
        if (!double_buffer_enabled_) {
            buffers_[display_index_] = back;
        }
    }

    void enable_double_buffer(bool enable) {
        if (enable == double_buffer_enabled_) {
            return;
        }

        if (enable) {
            double_buffer_enabled_ = true;
            display_index_ = 0;
            draw_index_ = 1;
            for (auto &buffer : buffers_) {
                if (buffer.size() != size_) {
                    buffer.resize(size_);
                }
            }
            buffers_[draw_index_] = buffers_[display_index_];
        } else {
            if (display_index_ != 0 && size_ != 0) {
                buffers_[0] = buffers_[display_index_];
            }
            double_buffer_enabled_ = false;
            display_index_ = 0;
            draw_index_ = 0;
        }
    }

    bool double_buffer_enabled() const { return double_buffer_enabled_; }

    void swap_buffers() {
        if (!double_buffer_enabled_) {
            return;
        }
        std::swap(draw_index_, display_index_);
    }

    void copy_front_to_back() {
        if (size_ == 0) {
            return;
        }
        buffers_[draw_index_] = buffers_[display_index_];
    }

    void copy_back_to_front() {
        if (size_ == 0) {
            return;
        }
        buffers_[display_index_] = buffers_[draw_index_];
    }

  private:
    std::array<std::vector<uint8_t>, 2> buffers_{};
    std::size_t size_ = 0;
    int draw_index_ = 0;
    int display_index_ = 0;
    bool double_buffer_enabled_ = false;
};

#endif // PICO_DRIVERS_DISPLAY_FRAME_BUFFER_HPP_
