/** @file tps25750.hpp
 *
 * @brief this module defines an interface to the TPS25750 USB-C PD controller.
 *
 * @par
 * The TPS25750 is a highly integrated stand-alone USB Type-C and Power Delivery
 * (PD) controller optimized for applications supporting USB-C PD Power. The
 * TPS25750 integrates fully managed power paths with robust protection for a
 * complete USB-C PD solution. The TPS25750 also integrates control for external
 * battery charger ICs for added ease of use and reduced time to market.
 *
 * Link to datasheet:
 * https://www.ti.com/lit/ds/symlink/tps25750.pdf?ts=1714726260022&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTPS25750%253FkeyMatch%253DTPS25750%2526tisearch%253Dsearch-everything%2526usecase%253DGPN-ALT
 *
 * Link to Host Interface Technical Reference Manual:
 * https://www.ti.com/lit/ug/slvuc05a/slvuc05a.pdf?ts=1714750459366&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTPS25750
 */

/*
 * TODO:
 * 1. clearly define the I2C slave address setting.
 * 2. write out the register map.
 *
 *
 * */

#ifndef _TSP25750_H
#define _TSP25750_H

#include <array>
#include <cstddef>
#include <cstdint>

#include "../i2c/i2c.hpp"

static constexpr uint8_t TPS25750_EXTERNAL_EEPROM_ADDRESS = 0x50;

enum TPS25750_SLAVE_ADDRESSES {
  TPS25750_SADDR_1 = 0x20,
  TPS25750_SADDR_2 = 0x21,
  TPS25750_SADDR_3 = 0x22,
  TPS25750_SADDR_4 = 0x23
};

class TPS25750 {
 public:
  static TPS25750 *inst;

  /**
   * @brief Get or create the singleton instance of the driver.
   *
   * @param address I2C address selected via the device ADDR pins.
   * @param sda GPIO used for I2C data.
   * @param scl GPIO used for I2C clock.
   * @param baudrate Desired I2C clock frequency.
   * @return TPS25750* pointer to the singleton instance.
   */
  static TPS25750 *get_instance(
      uint8_t address = TPS25750_SADDR_1,
      uint sda = I2C_DEFAULT_SDA,
      uint scl = I2C_DEFAULT_SCL,
      uint32_t baudrate = I2C_DEFAULT_BAUDRATE);

  /**
   * @brief Small helper representing variable-length bit-fields returned by
   * TPS25750 registers the datasheet labels as "bit field".
   *
   * @tparam N number of bytes in the register.
   */
  template <size_t N>
  struct RegisterArray {
    std::array<uint8_t, N> bytes{};

    /**
     * @brief Access a bit within the raw register.
     * @param index Bit index (LSB = 0).
     * @return true if the bit is set.
     */
    bool test_bit(size_t index) const {
      if (index >= (N * 8)) return false;
      size_t byte_index = index / 8;
      uint8_t bit_index = index % 8;
      return (bytes[byte_index] & (1u << bit_index)) != 0u;
    }
  };

  using InterruptRegister = RegisterArray<11>;
  using StatusRegister = RegisterArray<5>;
  using PowerPathStatusRegister = RegisterArray<5>;
  using RxCapsRegister = RegisterArray<29>;
  using TxCapsRegister = RegisterArray<31>;
  using ActivePdoRegister = RegisterArray<6>;
  using CustomerUseRegister = RegisterArray<8>;
  using GpioStatusRegister = RegisterArray<8>;

  struct PowerStatusDecoded {
    bool sourcing_high_voltage = false;
    bool sourcing_vbus = false;
    bool vbus_present = false;
    bool vbus_detect_enabled = false;
    bool vconn_present = false;
    bool sinking_vbus = false;
  };

  struct PortStatusSnapshot {
    uint32_t mode = 0;
    StatusRegister status{};
    PowerPathStatusRegister power_path{};
    uint16_t power_status = 0;
    uint32_t pd_status = 0;
    uint32_t typec_state = 0;
    GpioStatusRegister gpio_status{};
    InterruptRegister interrupt_event{};
  };

  /**
   * @brief Returns true if the controller responds with the expected interface
   * type identifier.
   */
  bool probe();

  /** @brief Return the currently configured I2C address. */
  uint8_t get_address() const { return address; }

  /** @brief Read the USB_PD_MODE register (0x03). */
  bool read_mode(uint32_t &mode);

  /** @brief Read the USB_PD_TYPE register (0x04). */
  bool read_type(uint32_t &type);

  /** @brief Read the USB_PD_DEVICE_CAPABILITIES register (0x0D). */
  bool read_device_capabilities(uint32_t &caps);

  /** @brief Read the USB_PD_VERSION register (0x0F). */
  bool read_version(uint32_t &version);

  /** @brief Read USB_PD_CUSTUSE register (0x06). */
  bool read_customer_use(CustomerUseRegister &data);

  /** @brief Read ASCII descriptors from the controller. */
  bool read_build_description(char *buffer, size_t length);
  bool read_device_info(char *buffer, size_t length);

  /** @brief Read interrupt status, mask, and clear registers. */
  bool read_interrupt_event1(InterruptRegister &events);
  bool read_interrupt_mask1(InterruptRegister &mask);
  bool write_interrupt_mask1(const InterruptRegister &mask);
  bool write_interrupt_clear1(const InterruptRegister &clear_bits);

  /** @brief Read USB_PD_STATUS register (0x1A). */
  bool read_status(StatusRegister &status);

  /** @brief Read USB_PD_POWER_PATH_STATUS register (0x26). */
  bool read_power_path_status(PowerPathStatusRegister &status);

  /** @brief Read/Write USB_PD_POWER_CONTROL register (0x29). */
  bool read_power_control(uint32_t &control);
  bool write_power_control(uint32_t control);

  /** @brief Read USB_PD_POWER_STATUS register (0x3F). */
  bool read_power_status(uint16_t &status);

  /** @brief Read USB_PD_PD_STATUS register (0x40). */
  bool read_pd_status(uint32_t &status);

  /** @brief Read USB_PD_TYPEC_STATE register (0x69). */
  bool read_typec_state(uint32_t &state);

  /** @brief Read USB_PD_GPIO_STATUS register (0x72). */
  bool read_gpio_status(GpioStatusRegister &status);

  /** @brief Read last received source and sink capabilities. */
  bool read_rx_source_caps(RxCapsRegister &caps);
  bool read_rx_sink_caps(RxCapsRegister &caps);

  /** @brief Read configured transmit source/sink capabilities. */
  bool read_tx_source_caps(TxCapsRegister &caps);
  bool read_tx_sink_caps(RxCapsRegister &caps);

  /** @brief Read active contract PDO/RDO. */
  bool read_active_contract_pdo(ActivePdoRegister &pdo);
  bool read_active_contract_rdo(uint32_t &rdo);

  /** @brief Read a consolidated snapshot of frequently used status registers. */
  bool read_port_status(PortStatusSnapshot &snapshot);

  /** @brief Raw register block read helper exposed for shell/debug utilities. */
  bool read_register_block(uint8_t reg, uint8_t *buffer, size_t length);
  bool write_register_block(uint8_t reg, const uint8_t *buffer, size_t length);

  PowerStatusDecoded decode_power_status(uint16_t raw) const;

  /**
   * @brief Write payload into DATA1 register (0x09).
   * @param payload Data bytes to write (max 64).
   */
  bool write_primary_data(const uint8_t *payload, size_t length);

  /**
   * @brief Read payload from DATA1 register (0x09).
   * @param buffer Destination buffer (max 64 bytes).
   */
  bool read_primary_data(uint8_t *buffer, size_t length);

  /**
   * @brief Issue a primary command by writing to CMD1 register (0x08).
   *
   * The caller is responsible for formatting the 4-byte command exactly as
   * defined in the host interface technical reference manual.
   *
   * @param command Raw 32-bit value to write into CMD1.
   */
  bool write_primary_command(uint32_t command);

 private:
  TPS25750(uint8_t address, uint sda, uint scl, uint32_t baudrate);
  ~TPS25750();
  TPS25750(TPS25750 const &) = delete;
  TPS25750 &operator=(TPS25750 const &) = delete;

  /* private members */
  uint8_t address;
  I2C i2c;

  bool read_bytes(uint8_t reg, uint8_t *buffer, size_t length);
  bool write_bytes(uint8_t reg, const uint8_t *buffer, size_t length);
  bool read_uint16(uint8_t reg, uint16_t &value);
  bool read_uint32(uint8_t reg, uint32_t &value);
  bool read_ascii_string(uint8_t reg, char *buffer, size_t buffer_length,
                         size_t register_length);
};

#endif /* END _TSP25750_H */

/* END OF FILE */
