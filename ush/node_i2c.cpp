/** @file node_i2c.cpp
 *
 * @brief this module creates a CLI in picoshell for controlling the I2C peripherals on RP2 based boards.
 *
 * @par
 * For more information about I2C, check the Pico SDK documentation.
 *
 * @author Nathan Winslow, 2024
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "picoshell.h"
#include "../utils/common.hpp"
#include "ush.h"

static const uint i2c0_pins[] = {0,1,4,5,8,9,12,13,16,17,20,21};
static const uint i2c1_pins[] = {2,3,6,7,10,11,14,15,18,19,26,27};

/* Shell flags */
static bool i2c0_is_init = false;
static bool i2c1_is_init = false;

/* Helper functions */
bool reserved_addr(uint8_t addr) { return (addr & 0x78) == 0 || (addr & 0x78) == 0x78; }

i2c_inst_t* pin_to_inst(uint pin) { return ((pin >> 1) & 0b1) ? i2c1 : i2c0; }


/* Callbacks */
static void i2c_init_exec_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, int argc,
                              char *argv[]) {

  if (argc != 3) {
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }
  uint sda = atoi(argv[1]);
  if (sda >= 26 || sda < 0) {
    ush_print(self, "ERROR: Invalid SDA pin");
    return;
  }

  uint scl = atoi(argv[2]);
  if (scl >= 27 || scl < 1) {
    ush_print(self, "ERROR: Invalid SCL pin");
    return;
  }

  i2c_inst_t* i2c = pin_to_inst(sda);
  if (i2c == NULL) {
    ush_print(self, "ERROR: Could not find I2C instance.");
    return;
  }

  uint i2c_port = i2c_get_index(i2c);

  if (i2c_port == 0 && i2c0_is_init) {
    ush_print(self, "I2C0 port already initialized.\r\n");
  } 
  else if (i2c_port == 1 && i2c1_is_init) {
    ush_print(self, "I2C1 port already initialized.\r\n");
  }
  else {
    i2c_init(i2c, 400 * 1000); //defaults to 400KHz baudrate
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
    i2c_port == 1 ? i2c1_is_init = true : i2c0_is_init = true;
  }
  return;
}


static void i2c_deinit_exec_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, int argc,
                              char *argv[]) {
  if (argc != 2) {
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }

  int port_num = atoi(argv[1]);
  port_num >= 1 ? port_num = 1 : port_num = 0;

  if (!i2c0_is_init && port_num == 0) {
    ush_print(self, "ERROR: I2C port 0 must be initialized before deinitializing.\r\n");
    return;
  }

  if (!i2c1_is_init && port_num == 1) {
    ush_print(self, "ERROR: I2C port 1 must be initialized before deinitializing.\r\n");
    return;
  }

  i2c_inst_t* i2c = i2c_get_instance(port_num);
  if (i2c == NULL) {
    ush_print(self, "ERROR: Could not find I2C instance.");
    return;
  }

  i2c_deinit(i2c);
#if PICO_2040 //WARN: This code ONLY works on the RP2040
  if (port_num == 0) {
    for (int i = 0; i < sizeof(i2c0_pins); ++i) {
      if (gpio_get_function(i2c0_pins[i] == GPIO_FUNC_I2C)) {
        gpio_set_function(i2c0_pins[i], GPIO_FUNC_NULL);
        gpio_disable_pulls(i2c0_pins[i]);
      }
    }
    i2c0_is_init = false;
  } 
  else {
    for (int i = 0; i < sizeof(i2c1_pins); ++i) {
      if (gpio_get_function(i2c1_pins[i] == GPIO_FUNC_I2C)) {
        gpio_set_function(i2c1_pins[i], GPIO_FUNC_NULL);
        gpio_disable_pulls(i2c1_pins[i]);
      }
    }
    i2c1_is_init = false;
  }
#else
  port_num == 0 ? i2c0_is_init = false : i2c1_is_init = false;
#endif

  return;
}

static void i2c_scan_exec_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, int argc,
                              char *argv[]) {
  if (argc != 2) {
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }
  uint port = atoi(argv[1]);

  i2c_inst_t* i2c = i2c_get_instance(port);
  if (i2c == NULL) {
    ush_print(self, "ERROR: Could not find I2C instance.");
    return;
  }

  if ((port == 0 && !i2c0_is_init) || (port == 1 && !i2c1_is_init)) {
    ush_print(self, "ERROR: I2C port must be initialized before scanning.\r\n");
    return;
  }

  //NOTE: A bit of delay is added to prevent the serial port from hanging.
  int await = millis();
  while (millis() - await < 5);

  //NOTE: This is a refactor of `bus_scan.c` from the pico-examples
  ush_printf(self, "...I2C Bus Scan...\n");
  ush_printf(self, "   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F \n");
  for (int addr = 0; addr < (1 << 7); ++addr) {
    if (addr % 16 == 0) ush_printf(self, "%02x ", addr);
    int ret;
    uint8_t rxdata;

    //TODO: change read_blocking to a read_until and return error if timedout.
    if (reserved_addr(addr)) {
      ret = PICO_ERROR_GENERIC;
    } else {
      ret = i2c_read_blocking(i2c, addr, &rxdata, 1, false);
    }
    ush_printf(self, ret < 0 ? "." : "@");
    ush_printf(self, addr % 16 == 15 ? "\n" : "  ");
  }
  ush_printf(self, "...Done with i2c scan...\n");
  ush_flush(self);
  return;
}

size_t i2c_get_data_callback(struct ush_object *self,
                             struct ush_file_descriptor const *file, uint8_t **data) {
  if (!i2c0_is_init && !i2c1_is_init) {
    ush_print(self, "ERROR: I2C PORTS NOT INITIALIZED!\r\n");
    return 0;
  }
  static char i2c_buf[16];

  //TODO: Grab data from I2C port and store in buffer.
  snprintf(i2c_buf, sizeof(i2c_buf), "%s", i2c_buf);

  *data = (uint8_t*) i2c_buf;

  return strlen((char *)(*data));
}


// i2c directory handler
static struct ush_node_object i2c;

// i2c directory files descriptor
static const struct ush_file_descriptor i2c_files[] = {
  {
    .name = "init",
    .description = "Initialize I2C",
    .help = "Initializes I2C port on pins SDA & SCL\r\nUsage: init [SDA] [SCL]\r\n",
    .exec = i2c_init_exec_callback,
  },
  {
    .name = "deinit",
    .description = "Deinitialize I2C",
    .help = "Deinitializes I2C port 0 or 1\r\nUsage: deinit [0|1]\r\n\n",
    .exec = i2c_deinit_exec_callback,
  },
  {
    .name = "scan",
    .description = "Scan the desired I2C port",
    .help = "Scans the I2C bus for any available devices.\r\nUsage: scan [0|1]\r\n\n",
    .exec = i2c_scan_exec_callback,
  },
};

extern struct ush_object ush;

void picoshell_i2c_mount(void) {
  ush_node_mount(&ush, "/i2c", &i2c, i2c_files, sizeof(i2c_files) / sizeof(i2c_files[0]));
}
