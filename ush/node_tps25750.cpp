#include <array>
#include <cstdlib>

#include "picoshell.h"
#include "../tps25750/tps25750.hpp"

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

void print_snapshot(struct ush_object *self,
                    const TPS25750::PortStatusSnapshot &snapshot) {
  ush_printf(self, "  MODE: 0x%08lx\r\n",
             static_cast<unsigned long>(snapshot.mode));
  print_register_bytes(self, "STATUS", snapshot.status);
  print_register_bytes(self, "POWER_PATH", snapshot.power_path);
  ush_printf(self, "  POWER_STATUS: 0x%04x\r\n", snapshot.power_status);
  ush_printf(self, "  PD_STATUS: 0x%08lx\r\n",
             static_cast<unsigned long>(snapshot.pd_status));
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

}  // namespace

static void tps_probe_exec_callback(struct ush_object *self,
                                    struct ush_file_descriptor const *file,
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
                                     struct ush_file_descriptor const *file,
                                     int argc, char *argv[]) {
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

static void tps_read_exec_callback(struct ush_object *self,
                                   struct ush_file_descriptor const *file,
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
};

extern struct ush_object ush;

void picoshell_tps25750_mount(void) {
  ush_node_mount(&ush, "/tps25750", &tps_node, tps_files,
                 sizeof(tps_files) / sizeof(tps_files[0]));
}
