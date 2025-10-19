#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../tps25750/tps25750.hpp"
#include "picoshell.h"

namespace {
TPS25750 *detect_pd(uint8_t &address) {
    static uint8_t cached_address = TPS25750_SADDR_2;
    constexpr std::array<uint8_t, 4> addresses = {
        TPS25750_SADDR_1,
        TPS25750_SADDR_2,
        TPS25750_SADDR_3,
        TPS25750_SADDR_4,
    };

    TPS25750 *pd = TPS25750::get_instance(cached_address);
    if (pd != nullptr && pd->probe()) {
        address = cached_address;
        return pd;
    }

    for (uint8_t addr : addresses) {
        pd = TPS25750::get_instance(addr);
        if (pd != nullptr && pd->probe()) {
            cached_address = addr;
            address = addr;
            return pd;
        }
    }

    address = 0;
    return nullptr;
}

template <size_t N>
void print_register_bytes(struct ush_object *self, const char *label,
                          const TPS25750::RegisterArray<N> &reg) {
    ush_printf(self, "  %s:", label);
    for (size_t i = 0; i < reg.bytes.size(); ++i) {
        ush_printf(self, " %02x", reg.bytes[i]);
    }
    ush_printf(self, "\r\n");
}

void print_snapshot(struct ush_object *self, const TPS25750::PortStatusSnapshot &snapshot) {
    ush_printf(self, "  MODE: 0x%08lx\r\n", static_cast<unsigned long>(snapshot.mode));
    print_register_bytes(self, "STATUS", snapshot.status);
    print_register_bytes(self, "POWER_PATH", snapshot.power_path);
    ush_printf(self, "  POWER_STATUS: 0x%04x\r\n", snapshot.power_status);
    ush_printf(self, "  PD_STATUS: 0x%08lx\r\n", static_cast<unsigned long>(snapshot.pd_status));
    ush_printf(self, "  TYPEC_STATE: 0x%08lx\r\n",
               static_cast<unsigned long>(snapshot.typec_state));
    print_register_bytes(self, "GPIO_STATUS", snapshot.gpio_status);
    print_register_bytes(self, "INT_EVENT1", snapshot.interrupt_event);
}

void print_identification(struct ush_object *self, TPS25750 *pd) {
    uint32_t version = 0;
    if (pd->read_version(version)) {
        ush_printf(self, "  Version: 0x%08lx\r\n", static_cast<unsigned long>(version));
    } else {
        ush_print(self, (char *)"  Version: <unavailable>\r\n");
    }

    char description[50] = {0};
    if (pd->read_build_description(description, sizeof(description))) {
        ush_printf(self, "  Build: %s\r\n", description);
    } else {
        ush_print(self, (char *)"  Build: <unavailable>\r\n");
    }

    char info[41] = {0};
    if (pd->read_device_info(info, sizeof(info))) {
        ush_printf(self, "  Device: %s\r\n", info);
    } else {
        ush_print(self, (char *)"  Device: <unavailable>\r\n");
    }
}

} // namespace

static void tps_probe_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file,
                                    int argc, char *argv[]) {
    (void)file;
    (void)argc;
    (void)argv;

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected on known addresses.\r\n");
        return;
    }

    ush_printf(self, "TPS25750 detected at 0x%02X\r\n", address);
    print_identification(self, pd);
}

static void tps_status_exec_callback(struct ush_object *self,
                                     struct ush_file_descriptor const *file, int argc,
                                     char *argv[]) {
    (void)file;
    (void)argc;
    (void)argv;

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    TPS25750::PortStatusSnapshot snapshot{};
    if (!pd->read_port_status(snapshot)) {
        ush_print(self, (char *)"Failed to read port status snapshot.\r\n");
        return;
    }

    ush_printf(self, "TPS25750 status (0x%02X)\r\n", address);
    print_snapshot(self, snapshot);
}

static void tps_read_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file,
                                   int argc, char *argv[]);
static void tps_write_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file,
                                    int argc, char *argv[]);
static void tps_cmd_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file,
                                  int argc, char *argv[]);
static void tps_data_write_exec_callback(struct ush_object *self,
                                         struct ush_file_descriptor const *file, int argc,
                                         char *argv[]);
static void tps_data_read_exec_callback(struct ush_object *self,
                                        struct ush_file_descriptor const *file, int argc,
                                        char *argv[]);
static void tps_power_control_exec_callback(struct ush_object *self,
                                            struct ush_file_descriptor const *file, int argc,
                                            char *argv[]);
static void tps_power_status_exec_callback(struct ush_object *self,
                                           struct ush_file_descriptor const *file, int argc,
                                           char *argv[]);
static void tps_rx_caps_exec_callback(struct ush_object *self,
                                      struct ush_file_descriptor const *file, int argc,
                                      char *argv[]);

static void tps_read_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file,
                                   int argc, char *argv[]) {
    (void)file;

    if (argc < 2 || argc > 3) {
        ush_print(self, (char *)"Usage: read <reg> [len]\r\n");
        return;
    }

    char *end = nullptr;
    long reg_val = strtol(argv[1], &end, 0);
    if ((end == argv[1]) || (reg_val < 0) || (reg_val > 0xFF)) {
        ush_print(self, (char *)"Invalid register address.\r\n");
        return;
    }

    int length = 4;
    if (argc == 3) {
        length = static_cast<int>(strtol(argv[2], &end, 0));
        if ((end == argv[2]) || length <= 0) {
            ush_print(self, (char *)"Invalid length.\r\n");
            return;
        }
    }

    if (length > 64) {
        ush_print(self, (char *)"Length too large (max 64).\r\n");
        return;
    }

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    std::array<uint8_t, 64> buffer{};
    if (!pd->read_register_block(static_cast<uint8_t>(reg_val), buffer.data(),
                                 static_cast<size_t>(length))) {
        ush_print(self, (char *)"Register read failed.\r\n");
        return;
    }

    ush_printf(self, "Reg 0x%02lX:", reg_val);
    for (int i = 0; i < length; ++i) {
        ush_printf(self, " %02x", buffer[static_cast<size_t>(i)]);
    }
    ush_print(self, (char *)"\r\n");
}

static void tps_cmd_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file,
                                  int argc, char *argv[]) {
    (void)file;

    if (argc != 2) {
        ush_print(self, (char *)"Usage: cmd <value>\r\nExample: cmd 0x00000001\r\n");
        return;
    }

    char *end = nullptr;
    unsigned long cmd = strtoul(argv[1], &end, 0);
    if (end == argv[1]) {
        ush_print(self, (char *)"Invalid command value.\r\n");
        return;
    }

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    if (!pd->write_primary_command(static_cast<uint32_t>(cmd))) {
        ush_print(self, (char *)"Failed to write primary command.\r\n");
        return;
    }

    ush_printf(self, "CMD1 <= 0x%08lX\r\n", cmd);
}

static void tps_data_write_exec_callback(struct ush_object *self,
                                         struct ush_file_descriptor const *file, int argc,
                                         char *argv[]) {
    (void)file;

    if (argc < 2) {
        ush_print(self, (char *)"Usage: data_write <byte> [byte ...]\r\n");
        return;
    }

    size_t length = static_cast<size_t>(argc - 1);
    if (length > 64) {
        ush_print(self, (char *)"Too many bytes (max 64).\r\n");
        return;
    }

    std::array<uint8_t, 64> buffer{};
    for (int i = 1; i < argc; ++i) {
        char *end = nullptr;
        long value = strtol(argv[i], &end, 0);
        if ((end == argv[i]) || (value < 0) || (value > 0xFF)) {
            ush_printf(self, "Invalid byte '%s'.\r\n", argv[i]);
            return;
        }
        buffer[static_cast<size_t>(i - 1)] = static_cast<uint8_t>(value);
    }

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    if (!pd->write_primary_data(buffer.data(), length)) {
        ush_print(self, (char *)"Failed to write DATA1.\r\n");
        return;
    }

    ush_printf(self, "DATA1 <=");
    for (size_t i = 0; i < length; ++i) {
        ush_printf(self, " %02x", buffer[i]);
    }
    ush_print(self, (char *)"\r\n");
}

static void tps_data_read_exec_callback(struct ush_object *self,
                                        struct ush_file_descriptor const *file, int argc,
                                        char *argv[]) {
    (void)file;

    size_t length = 64;
    if (argc == 2) {
        char *end = nullptr;
        long len_long = strtol(argv[1], &end, 0);
        if ((end == argv[1]) || (len_long <= 0) || (len_long > 64)) {
            ush_print(self, (char *)"Invalid length (1-64).\r\n");
            return;
        }
        length = static_cast<size_t>(len_long);
    } else if (argc > 2) {
        ush_print(self, (char *)"Usage: data_read [len]\r\n");
        return;
    }

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    std::array<uint8_t, 64> buffer{};
    if (!pd->read_primary_data(buffer.data(), length)) {
        ush_print(self, (char *)"Failed to read DATA1.\r\n");
        return;
    }

    ush_printf(self, "DATA1 =>");
    for (size_t i = 0; i < length; ++i) {
        ush_printf(self, " %02x", buffer[i]);
    }
    ush_print(self, (char *)"\r\n");
}

static void tps_power_control_exec_callback(struct ush_object *self,
                                            struct ush_file_descriptor const *file, int argc,
                                            char *argv[]) {
    (void)file;

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    if (argc == 1) {
        uint32_t control = 0;
        if (!pd->read_power_control(control)) {
            ush_print(self, (char *)"Failed to read POWER_CONTROL.\r\n");
            return;
        }
        ush_printf(self, "POWER_CONTROL => 0x%08lx\r\n", static_cast<unsigned long>(control));
        return;
    }

    if (argc == 2) {
        char *end = nullptr;
        unsigned long value = strtoul(argv[1], &end, 0);
        if (end == argv[1]) {
            ush_print(self, (char *)"Invalid value.\r\n");
            return;
        }
        if (!pd->write_power_control(static_cast<uint32_t>(value))) {
            ush_print(self, (char *)"Failed to write POWER_CONTROL.\r\n");
            return;
        }
        ush_printf(self, "POWER_CONTROL <= 0x%08lx\r\n", value);
        return;
    }

    ush_print(self, (char *)"Usage: power_control [value]\r\n");
}

static void tps_power_status_exec_callback(struct ush_object *self,
                                           struct ush_file_descriptor const *file, int argc,
                                           char *argv[]) {
    (void)file;
    (void)argc;
    (void)argv;

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    uint16_t raw = 0;
    if (!pd->read_power_status(raw)) {
        ush_print(self, (char *)"Failed to read POWER_STATUS.\r\n");
        return;
    }

    auto decoded = pd->decode_power_status(raw);

    ush_printf(self, "POWER_STATUS => 0x%04x\r\n", raw);
    ush_printf(self, "  sourcing_high_voltage: %s\r\n",
               decoded.sourcing_high_voltage ? "yes" : "no");
    ush_printf(self, "  sourcing_vbus: %s\r\n", decoded.sourcing_vbus ? "yes" : "no");
    ush_printf(self, "  sinking_vbus: %s\r\n", decoded.sinking_vbus ? "yes" : "no");
    ush_printf(self, "  vbus_present: %s\r\n", decoded.vbus_present ? "yes" : "no");
    ush_printf(self, "  vbus_detect_enabled: %s\r\n", decoded.vbus_detect_enabled ? "yes" : "no");
    ush_printf(self, "  vconn_present: %s\r\n", decoded.vconn_present ? "yes" : "no");
}

namespace {

enum class PdoType : uint8_t { Fixed = 0, Battery = 1, Variable = 2, Augmented = 3 };

const char *pdo_type_to_string(PdoType type) {
    switch (type) {
    case PdoType::Fixed:
        return "Fixed";
    case PdoType::Battery:
        return "Battery";
    case PdoType::Variable:
        return "Variable";
    case PdoType::Augmented:
        return "Augmented";
    default:
        return "Unknown";
    }
}

void decode_and_print_fixed_pdo(struct ush_object *self, uint32_t pdo_word) {
    float voltage = static_cast<float>((pdo_word >> 10) & 0x3FF) * 50.0f / 1000.0f;
    float current = static_cast<float>(pdo_word & 0x3FF) * 10.0f / 1000.0f;
    ush_printf(self, "    Voltage: %.2f V, Current: %.2f A\r\n", voltage, current);
}

void decode_and_print_variable_pdo(struct ush_object *self, uint32_t pdo_word) {
    float min_voltage = static_cast<float>((pdo_word >> 10) & 0x3FF) * 50.0f / 1000.0f;
    float max_voltage = static_cast<float>((pdo_word >> 20) & 0x3FF) * 50.0f / 1000.0f;
    float current = static_cast<float>(pdo_word & 0x3FF) * 10.0f / 1000.0f;
    ush_printf(self, "    Voltage: %.2f-%.2f V, Current: %.2f A\r\n", min_voltage, max_voltage,
               current);
}

void decode_and_print_battery_pdo(struct ush_object *self, uint32_t pdo_word) {
    float min_voltage = static_cast<float>((pdo_word >> 10) & 0x3FF) * 50.0f / 1000.0f;
    float max_voltage = static_cast<float>((pdo_word >> 20) & 0x3FF) * 50.0f / 1000.0f;
    float power = static_cast<float>(pdo_word & 0x3FF) * 250.0f / 1000.0f;
    ush_printf(self, "    Voltage: %.2f-%.2f V, Power: %.2f W\r\n", min_voltage, max_voltage,
               power);
}

void decode_and_print_apdo(struct ush_object *self, uint32_t pdo_word) {
    float min_voltage = static_cast<float>((pdo_word >> 8) & 0xFF) * 100.0f / 1000.0f;
    float max_voltage = static_cast<float>((pdo_word >> 17) & 0x1FF) * 100.0f / 1000.0f;
    float current = static_cast<float>(pdo_word & 0x7F) * 50.0f / 1000.0f; // 50mA units
    ush_printf(self, "    Voltage: %.2f-%.2f V, Current: %.2f A (APDO)\r\n", min_voltage,
               max_voltage, current);
}

void print_pdo(struct ush_object *self, uint32_t pdo_word, size_t index) {
    auto type = static_cast<PdoType>((pdo_word >> 30) & 0x3);
    ush_printf(self, "  PDO %zu (%s): 0x%08lx\r\n", index + 1, pdo_type_to_string(type),
               static_cast<unsigned long>(pdo_word));

    switch (type) {
    case PdoType::Fixed:
        decode_and_print_fixed_pdo(self, pdo_word);
        break;
    case PdoType::Variable:
        decode_and_print_variable_pdo(self, pdo_word);
        break;
    case PdoType::Battery:
        decode_and_print_battery_pdo(self, pdo_word);
        break;
    case PdoType::Augmented:
        decode_and_print_apdo(self, pdo_word);
        break;
    }
}

} // namespace

static void print_pdo_list(struct ush_object *self, const uint8_t *data, size_t bytes) {
    if (bytes <= 2) {
        ush_print(self, (char *)"  (no PDOs present)\r\n");
        return;
    }

    size_t pdo_index = 0;
    for (size_t offset = 2; offset + 3 < bytes; offset += 4) {
        uint32_t word = static_cast<uint32_t>(data[offset]) |
                        (static_cast<uint32_t>(data[offset + 1]) << 8) |
                        (static_cast<uint32_t>(data[offset + 2]) << 16) |
                        (static_cast<uint32_t>(data[offset + 3]) << 24);
        if (word == 0) {
            if (pdo_index == 0) {
                ush_print(self, (char *)"  (no PDOs present)\r\n");
            }
            break;
        }
        print_pdo(self, word, pdo_index++);
    }
}

static void tps_rx_caps_exec_callback(struct ush_object *self,
                                      struct ush_file_descriptor const *file, int argc,
                                      char *argv[]) {
    (void)file;

    bool source = true;
    if (argc == 2) {
        if (strcmp(argv[1], "sink") == 0) {
            source = false;
        } else if (strcmp(argv[1], "source") != 0) {
            ush_print(self, (char *)"Usage: pdo [source|sink]\r\n");
            return;
        }
    } else if (argc > 2) {
        ush_print(self, (char *)"Usage: pdo [source|sink]\r\n");
        return;
    }

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    TPS25750::RxCapsRegister caps{};
    if (source) {
        if (!pd->read_rx_source_caps(caps)) {
            ush_print(self, (char *)"Failed to read RX_SOURCE_CAPS.\r\n");
            return;
        }
        ush_print(self, (char *)"RX Source Capabilities:\r\n");
    } else {
        if (!pd->read_rx_sink_caps(caps)) {
            ush_print(self, (char *)"Failed to read RX_SINK_CAPS.\r\n");
            return;
        }
        ush_print(self, (char *)"RX Sink Capabilities:\r\n");
    }

    print_pdo_list(self, caps.bytes.data(), caps.bytes.size());
}

static void tps_write_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file,
                                    int argc, char *argv[]) {
    (void)file;

    if (argc < 3) {
        ush_print(self, (char *)"Usage: write <reg> <byte> [byte ...]\r\n");
        ush_print(self, (char *)"Example: write 0x08 0x10 0x00 0x00 0x00\r\n");
        return;
    }

    char *end = nullptr;
    long reg_val = strtol(argv[1], &end, 0);
    if ((end == argv[1]) || (reg_val < 0) || (reg_val > 0xFF)) {
        ush_print(self, (char *)"Invalid register address.\r\n");
        return;
    }

    size_t length = static_cast<size_t>(argc - 2);
    if (length > 64) {
        ush_print(self, (char *)"Too many data bytes (max 64).\r\n");
        return;
    }

    std::array<uint8_t, 64> buffer{};
    for (int i = 2; i < argc; ++i) {
        long value = strtol(argv[i], &end, 0);
        if ((end == argv[i]) || (value < 0) || (value > 0xFF)) {
            ush_printf(self, "Invalid byte '%s'.\r\n", argv[i]);
            return;
        }
        buffer[static_cast<size_t>(i - 2)] = static_cast<uint8_t>(value);
    }

    uint8_t address = 0;
    TPS25750 *pd = detect_pd(address);
    if (pd == nullptr) {
        ush_print(self, (char *)"TPS25750 not detected.\r\n");
        return;
    }

    if (!pd->write_register_block(static_cast<uint8_t>(reg_val), buffer.data(), length)) {
        ush_print(self, (char *)"Register write failed.\r\n");
        return;
    }

    ush_printf(self, "Wrote %zu bytes to 0x%02lX\r\n", length, reg_val);
}

static struct ush_node_object tps_node;

static const struct ush_file_descriptor tps_files[] = {
    {
        .name = "probe",
        .description = "Probe for TPS25750 and print identification",
        .help = "Usage: probe\r\n",
        .exec = tps_probe_exec_callback,
    },
    {
        .name = "status",
        .description = "Show consolidated port status snapshot",
        .help = "Usage: status\r\n",
        .exec = tps_status_exec_callback,
    },
    {
        .name = "read",
        .description = "Read raw register bytes",
        .help = "Usage: read <reg> [len]\r\nExample: read 0x3f 2\r\n",
        .exec = tps_read_exec_callback,
    },
    {
        .name = "write",
        .description = "Write raw register bytes",
        .help = "Usage: write <reg> <byte> [byte ...]\r\nExample: write 0x09 0x12 0x34\r\n",
        .exec = tps_write_exec_callback,
    },
    {
        .name = "cmd",
        .description = "Write a 32-bit primary command",
        .help = "Usage: cmd <value>\r\n",
        .exec = tps_cmd_exec_callback,
    },
    {
        .name = "data_read",
        .description = "Read from DATA1 register",
        .help = "Usage: data_read [len]\r\n",
        .exec = tps_data_read_exec_callback,
    },
    {
        .name = "data_write",
        .description = "Write bytes to DATA1 register",
        .help = "Usage: data_write <byte> [byte ...]\r\n",
        .exec = tps_data_write_exec_callback,
    },
    {
        .name = "power_control",
        .description = "Read or write POWER_CONTROL register",
        .help = "Usage: power_control [value]\r\n",
        .exec = tps_power_control_exec_callback,
    },
    {
        .name = "power_status",
        .description = "Decode POWER_STATUS register",
        .help = "Usage: power_status\r\n",
        .exec = tps_power_status_exec_callback,
    },
    {
        .name = "pdo",
        .description = "Print received PD capabilities",
        .help = "Usage: pdo [source|sink]\r\n",
        .exec = tps_rx_caps_exec_callback,
    },
};

extern struct ush_object ush;

void picoshell_tps25750_mount(void) {
    ush_node_mount(&ush, "/tps25750", &tps_node, tps_files,
                   sizeof(tps_files) / sizeof(tps_files[0]));
}
