/** @file tps25750.cpp
 *
 * @brief This module implements the TPS25750 USB-PD controller definitions.
 *
 */

#include "tps25750.hpp"

#include "tps2575x_register_map.hpp"

TPS25750 *TPS25750::inst = nullptr;

namespace {
constexpr size_t PRIMARY_DATA_REGISTER_SIZE = 64;

inline uint32_t compose_le32(const uint8_t *raw) {
  return static_cast<uint32_t>(raw[0]) |
         (static_cast<uint32_t>(raw[1]) << 8) |
         (static_cast<uint32_t>(raw[2]) << 16) |
         (static_cast<uint32_t>(raw[3]) << 24);
}

bool matches_type_signature(const uint8_t *raw, size_t length) {
  if (raw == nullptr || length < 3) {
    return false;
  }

  constexpr char signature[] = {'I', '2', 'C'};
  constexpr size_t signature_len = sizeof(signature);
  for (size_t offset = 0; offset + signature_len <= length; ++offset) {
    bool match = true;
    for (size_t idx = 0; idx < signature_len; ++idx) {
      if (raw[offset + idx] != static_cast<uint8_t>(signature[idx])) {
        match = false;
        break;
      }
    }
    if (match) {
      return true;
    }
  }
  return false;
}
}  // namespace

TPS25750 *TPS25750::get_instance(uint8_t address, uint sda, uint scl,
                                 uint32_t baudrate) {
  if (inst == nullptr) {
    inst = new TPS25750(address, sda, scl, baudrate);
  } else {
    inst->address = address;
  }
  return inst;
}

TPS25750::TPS25750(uint8_t address, uint sda, uint scl, uint32_t baudrate)
    : address(address), i2c(sda, scl, baudrate) {}

TPS25750::~TPS25750() = default;

bool TPS25750::probe() {
  uint8_t raw[4] = {0x00, 0x00, 0x00, 0x00};
  if (!read_bytes(USB_PD_TYPE, raw, sizeof(raw))) {
    return false;
  }

  if (!matches_type_signature(raw, sizeof(raw))) {
    return false;
  }

  // Cache the address that successfully responded.
  return true;
}

bool TPS25750::read_mode(uint32_t &mode) { return read_uint32(USB_PD_MODE, mode); }

bool TPS25750::read_type(uint32_t &type) { return read_uint32(USB_PD_TYPE, type); }

bool TPS25750::read_device_capabilities(uint32_t &caps) {
  return read_uint32(USB_PD_DEVICE_CAPABILITIES, caps);
}

bool TPS25750::read_version(uint32_t &version) {
  return read_uint32(USB_PD_VERSION, version);
}

bool TPS25750::read_customer_use(CustomerUseRegister &data) {
  return read_bytes(USB_PD_CUSTUSE, data.bytes.data(), data.bytes.size());
}

bool TPS25750::read_build_description(char *buffer, size_t length) {
  constexpr size_t REGISTER_LENGTH = 49;
  return read_ascii_string(USB_PD_BUILD_DESCRIPTION, buffer, length,
                           REGISTER_LENGTH);
}

bool TPS25750::read_device_info(char *buffer, size_t length) {
  constexpr size_t REGISTER_LENGTH = 40;
  return read_ascii_string(USB_PD_DEVICE_INFO, buffer, length, REGISTER_LENGTH);
}

bool TPS25750::read_interrupt_event1(InterruptRegister &events) {
  return read_bytes(USB_PD_INT_EVENT1, events.bytes.data(), events.bytes.size());
}

bool TPS25750::read_interrupt_mask1(InterruptRegister &mask) {
  return read_bytes(USB_PD_INT_MASK1, mask.bytes.data(), mask.bytes.size());
}

bool TPS25750::write_interrupt_mask1(const InterruptRegister &mask) {
  return write_bytes(USB_PD_INT_MASK1, mask.bytes.data(), mask.bytes.size());
}

bool TPS25750::write_interrupt_clear1(const InterruptRegister &clear_bits) {
  return write_bytes(USB_PD_INT_CLEAR1, clear_bits.bytes.data(),
                     clear_bits.bytes.size());
}

bool TPS25750::read_status(StatusRegister &status) {
  return read_bytes(USB_PD_STATUS, status.bytes.data(), status.bytes.size());
}

bool TPS25750::read_power_path_status(PowerPathStatusRegister &status) {
  return read_bytes(USB_PD_POWER_PATH_STATUS, status.bytes.data(),
                    status.bytes.size());
}

bool TPS25750::read_power_control(uint32_t &control) {
  return read_uint32(USB_PD_POWER_CONTROL, control);
}

bool TPS25750::write_power_control(uint32_t control) {
  uint8_t raw[4] = {
      static_cast<uint8_t>(control & 0xFF),
      static_cast<uint8_t>((control >> 8) & 0xFF),
      static_cast<uint8_t>((control >> 16) & 0xFF),
      static_cast<uint8_t>((control >> 24) & 0xFF),
  };
  return write_bytes(USB_PD_POWER_CONTROL, raw, sizeof(raw));
}

bool TPS25750::read_power_status(uint16_t &status) {
  return read_uint16(USB_PD_POWER_STATUS, status);
}

bool TPS25750::read_pd_status(uint32_t &status) {
  return read_uint32(USB_PD_PD_STATUS, status);
}

bool TPS25750::read_typec_state(uint32_t &state) {
  return read_uint32(USB_PD_TYPEC_STATE, state);
}

bool TPS25750::read_gpio_status(GpioStatusRegister &status) {
  return read_bytes(USB_PD_GPIO_STATUS, status.bytes.data(), status.bytes.size());
}

bool TPS25750::read_rx_source_caps(RxCapsRegister &caps) {
  return read_bytes(USB_PD_RX_SOURCE_CAPS, caps.bytes.data(), caps.bytes.size());
}

bool TPS25750::read_rx_sink_caps(RxCapsRegister &caps) {
  return read_bytes(USB_PD_RX_SINK_CAPS, caps.bytes.data(), caps.bytes.size());
}

bool TPS25750::read_tx_source_caps(TxCapsRegister &caps) {
  return read_bytes(USB_PD_TX_SOURCE_CAPS, caps.bytes.data(), caps.bytes.size());
}

bool TPS25750::read_tx_sink_caps(RxCapsRegister &caps) {
  return read_bytes(USB_PD_TX_SINK_CAPS, caps.bytes.data(), caps.bytes.size());
}

bool TPS25750::read_active_contract_pdo(ActivePdoRegister &pdo) {
  return read_bytes(USB_PD_ACTIVE_CONTRACT_PDO, pdo.bytes.data(),
                    pdo.bytes.size());
}

bool TPS25750::read_active_contract_rdo(uint32_t &rdo) {
  return read_uint32(USB_PD_ACTIVE_CONTRACT_RDO, rdo);
}

bool TPS25750::read_port_status(PortStatusSnapshot &snapshot) {
  if (!read_mode(snapshot.mode)) {
    return false;
  }

  if (!read_status(snapshot.status)) {
    return false;
  }

  if (!read_power_path_status(snapshot.power_path)) {
    return false;
  }

  if (!read_power_status(snapshot.power_status)) {
    return false;
  }

  if (!read_pd_status(snapshot.pd_status)) {
    return false;
  }

  if (!read_typec_state(snapshot.typec_state)) {
    return false;
  }

  if (!read_gpio_status(snapshot.gpio_status)) {
    return false;
  }

  if (!read_interrupt_event1(snapshot.interrupt_event)) {
    return false;
  }

  return true;
}

bool TPS25750::read_register_block(uint8_t reg, uint8_t *buffer, size_t length) {
  return read_bytes(reg, buffer, length);
}

bool TPS25750::write_register_block(uint8_t reg, const uint8_t *buffer,
                                    size_t length) {
  return write_bytes(reg, buffer, length);
}

TPS25750::PowerStatusDecoded TPS25750::decode_power_status(uint16_t raw) const {
  PowerStatusDecoded decoded;
  decoded.sourcing_high_voltage = (raw & (1u << 5)) != 0u;
  decoded.sourcing_vbus = (raw & (1u << 4)) != 0u;
  decoded.vbus_present = (raw & (1u << 2)) != 0u;
  decoded.vbus_detect_enabled = (raw & (1u << 3)) != 0u;
  decoded.vconn_present = (raw & (1u << 1)) != 0u;
  decoded.sinking_vbus = (raw & (1u << 0)) != 0u;
  return decoded;
}

bool TPS25750::write_primary_data(const uint8_t *payload, size_t length) {
  if (payload == nullptr || length > PRIMARY_DATA_REGISTER_SIZE) {
    return false;
  }
  return write_bytes(USB_PD_DATA1, payload, length);
}

bool TPS25750::read_primary_data(uint8_t *buffer, size_t length) {
  if (buffer == nullptr || length > PRIMARY_DATA_REGISTER_SIZE) {
    return false;
  }
  return read_bytes(USB_PD_DATA1, buffer, length);
}

bool TPS25750::write_primary_command(uint32_t command) {
  uint8_t raw[4] = {
      static_cast<uint8_t>(command & 0xFF),
      static_cast<uint8_t>((command >> 8) & 0xFF),
      static_cast<uint8_t>((command >> 16) & 0xFF),
      static_cast<uint8_t>((command >> 24) & 0xFF),
  };
  return write_bytes(USB_PD_CMD1, raw, sizeof(raw));
}

bool TPS25750::read_bytes(uint8_t reg, uint8_t *buffer, size_t length) {
  if (buffer == nullptr || length == 0) {
    return false;
  }
  int ret = i2c.read_bytes(address, reg, buffer, static_cast<int>(length));
  return ret == static_cast<int>(length);
}

bool TPS25750::write_bytes(uint8_t reg, const uint8_t *buffer, size_t length) {
  if (buffer == nullptr || length == 0) {
    return false;
  }
  int ret = i2c.write_bytes(address, reg, buffer, static_cast<int>(length));
  return ret == static_cast<int>(length + 1);
}

bool TPS25750::read_uint16(uint8_t reg, uint16_t &value) {
  uint8_t raw[2] = {0, 0};
  if (!read_bytes(reg, raw, sizeof(raw))) {
    return false;
  }
  value = static_cast<uint16_t>(raw[0]) |
          (static_cast<uint16_t>(raw[1]) << 8);
  return true;
}

bool TPS25750::read_uint32(uint8_t reg, uint32_t &value) {
  uint8_t raw[4] = {0, 0, 0, 0};
  if (!read_bytes(reg, raw, sizeof(raw))) {
    return false;
  }
  value = compose_le32(raw);
  return true;
}

bool TPS25750::read_ascii_string(uint8_t reg, char *buffer,
                                 size_t buffer_length,
                                 size_t register_length) {
  if (buffer == nullptr || buffer_length < 2u) {
    return false;
  }

  uint8_t raw[64] = {0};
  if (register_length > sizeof(raw)) {
    return false;
  }

  if (!read_bytes(reg, raw, register_length)) {
    return false;
  }

  size_t copy_len = register_length;
  if (copy_len > buffer_length - 1) {
    copy_len = buffer_length - 1;
  }

  for (size_t i = 0; i < copy_len; ++i) {
    buffer[i] = static_cast<char>(raw[i]);
  }
  buffer[copy_len] = '\0';
  return true;
}
