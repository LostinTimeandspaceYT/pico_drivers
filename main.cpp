#include <stdio.h>
#include <string.h>

#include <cstdint>

#include "i2c/i2c.hpp"
#include "pico/stdlib.h"
#include "ush/microshell.h"
#include "ush/ush.h"
#include "ush/ush_types.h"

#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C))) //for reboot

/* Debug Symbols */
#define DEBUG_AP33772 (false)
#define DEBUG_STUSB4500 (false)
#define DEBUG_SSD1306 (false)

#if DEBUG_AP33772
#include "ap33772/ap33772.hpp"
#endif

#if DEBUG_STUSB4500
#include "stusb4500/stusb4500.hpp"
#endif

#if DEBUG_SSD1306
#include "ssd1306/bitmap.hpp"
#include "ssd1306/ssd1306.hpp"

OLED oled = OLED(64, 128, true);
#endif

/*Function Prototypes */
void setup(void);
void forever_blink(void);
void scan_i2c_bus();
void scan_i2c_bus(uint sda, uint scl);

// working buffers allocations (size could be customized)
#define BUF_IN_SIZE 512
#define BUF_OUT_SIZE 512
#define PATH_MAX_SIZE 256

static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];

// Picoshell instance handler
static struct ush_object ush;

// non-blocking read interface
static int ush_read(struct ush_object *self, char *ch) {
  // should be implemented as a FIFO
  if (stdio_get_until(ch, BUF_IN_SIZE, PICO_ERROR_TIMEOUT) > 0) {
    return 1;
  }
  return 0;
}

// non-blocking write interface
static int ush_write(struct ush_object *self, char ch) {
  // should be implemented as a FIFO
  return (printf("%c", ch) == 1);
}

// I/O interface descriptor
static const struct ush_io_interface ush_iface = {
    .read = ush_read,
    .write = ush_write,
};

// Picoshell descriptor
static const struct ush_descriptor ush_desc = {
    .io = &ush_iface,                           // I/O interface pointer
    .input_buffer = ush_in_buf,                 // working input buffer
    .input_buffer_size = sizeof(ush_in_buf),    // working input buffer size
    .output_buffer = ush_out_buf,               // working output buffer
    .output_buffer_size = sizeof(ush_out_buf),  // working output buffer size
    .path_max_length = PATH_MAX_SIZE,           // path maximum length (stack)
    .hostname = "Pico2",                        // hostname (in prompt)
};

// toggle file execute callback
static void toggle_exec_callback(struct ush_object *self,
                                 struct ush_file_descriptor const *file, int argc,
                                 char *argv[]) {
  // simple toggle led, without any arguments validation
  gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
}

// reboot cmd file execute callback
static void reboot_exec_callback(struct ush_object *self,
                                 struct ush_file_descriptor const *file, int argc,
                                 char *argv[]) {
  // src: https://forums.raspberrypi.com/viewtopic.php?t=318747
  AIRCR_Register = 0x5FA0004;
  ush_print(self, "ERROR: Reboot not supported...");
}

// set file execute callback
static void set_exec_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, int argc,
                              char *argv[]) {
  // arguments count validation
  if (argc != 2) {
    // return predefined error message
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }

  // arguments validation
  if (strcmp(argv[1], "1") == 0) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
  } else if (strcmp(argv[1], "0") == 0) {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
  } else {
    // return predefined error message
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }
}

bool reserved_addr(uint8_t addr) { return (addr & 0x78) == 0 || (addr & 0x78) == 0x78; }

static void iscan_exec_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, int argc,
                              char *argv[]) {
  if (argc != 3) {
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }
  uint sda = atoi(argv[1]);
  if (sda >= 32 || sda < 0) {
    ush_print(self, "error: Invalid SDA pin");
    return;
  }

  uint scl = atoi(argv[2]);
  if (scl >= 32 || scl < 1) {
    ush_print(self, "error: Invalid SCL pin");
    return;
  }
  I2C i2c = I2C(sda, scl);

  //NOTE: This is a refactor of `bus_scan.c` from the pico-examples
  ush_printf(self, "...I2C Bus Scan...\n");
  ush_printf(self, "   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F \n");
  for (int addr = 0; addr < (1 << 7); ++addr) {
    if (addr % 16 == 0) ush_printf(self, "%02x ", addr);
    int ret;
    uint8_t rxdata;
    if (reserved_addr(addr))
      ret = PICO_ERROR_GENERIC;
    else
      ret = i2c.read_blocking(addr, &rxdata, 1, false);
    ush_printf(self, ret < 0 ? "." : "@");
    ush_printf(self, addr % 16 == 15 ? "\n" : "  ");
  }
  ush_printf(self, "Done with i2c scan...\n");
  ush_flush(self);
  return;
}

// info file get data callback
size_t info_get_data_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, uint8_t **data) {
  static const char *info = "Use Picoshell and make fun!\r\n";

  // return pointer to data
  *data = (uint8_t *)info;
  // return data size
  return strlen(info);
}

// led file get data callback
size_t led_get_data_callback(struct ush_object *self,
                             struct ush_file_descriptor const *file, uint8_t **data) {
  bool state = gpio_get(PICO_DEFAULT_LED_PIN);
  // return pointer to data
  *data = (uint8_t *)((state) ? "1\r\n" : "0\r\n");
  // return data size
  return strlen((char *)(*data));
}

// led file set data callback
void led_set_data_callback(struct ush_object *self,
                           struct ush_file_descriptor const *file, uint8_t *data,
                           size_t size) {
  // data size validation
  if (size < 1) return;

  // arguments validation
  if (data[0] == '1') {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
  } else if (data[0] == '0') {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
  }
}

// time file get data callback
size_t time_get_data_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, uint8_t **data) {
  static char time_buf[16];
  // read current time
  long current_time = millis();
  // convert
  snprintf(time_buf, sizeof(time_buf), "%ld\r\n", current_time);
  time_buf[sizeof(time_buf) - 1] = 0;
  // return pointer to data
  *data = (uint8_t *)time_buf;
  // return data size
  return strlen((char *)(*data));
}

// root directory files descriptor
static const struct ush_file_descriptor root_files[] = {
  {
    .name = "info.txt",  // info.txt file name
    .description = NULL,
    .help = NULL,
    .exec = NULL,
    .get_data = info_get_data_callback,
  }
};

// bin directory files descriptor
static const struct ush_file_descriptor bin_files[] = {
  {
    .name = "toggle",              // toggle file name
    .description = "toggle led",   // optional file description
    .help = "usage: toggle\r\n",   // optional help manual
    .exec = toggle_exec_callback,  // optional execute callback
  },
  {
    .name = "set",  // set file name
    .description = "set led",
    .help = "usage: set {0,1}\r\n",
    .exec = set_exec_callback
  },
  {
    .name = "iscan",  // set file name
    .description = "scans i2c bus.",
    .help = "usage: iscan [sda] [scl]\r\n",
    .exec = iscan_exec_callback
  },
};

// dev directory files descriptor
static const struct ush_file_descriptor dev_files[] = {
  {
    .name = "led",
    .description = NULL,
    .help = NULL,
    .exec = NULL,
    .get_data = led_get_data_callback,  // optional data getter callback
    .set_data = led_set_data_callback,  // optional data setter callback
  },
  {
    .name = "time",
    .description = NULL,
    .help = NULL,
    .exec = NULL,
    .get_data = time_get_data_callback,
  },
};

// cmd files descriptor
static const struct ush_file_descriptor cmd_files[] = {
  {
    .name = "reboot",
    .description = "reboot device",
    .help = NULL,
    .exec = reboot_exec_callback,
  },
};

// root directory handler
static struct ush_node_object root;

// dev directory handler
static struct ush_node_object dev;

// bin directory handler
static struct ush_node_object bin;

// cmd commands handler
static struct ush_node_object cmd;

int main() {
  setup();
#if DEBUG_SSD1306
  oled.draw_bitmap(0, 0, 40, 32, sun_with_clouds_40x32);
  oled.print(0, 36, (uint8_t *)"Hello World!");
  oled.show();
#endif

  while (1) {
    ush_service(&ush);
  }

  return 0;
}

void setup(void) {
  /*setup_default_uart();*/
  stdio_init_all();
  sleep_ms(2000);  // Internet suggests sleeping after calling stdio_init_all();

  /* Make sure the pico is alive... */
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
  gpio_put(PICO_DEFAULT_LED_PIN, 1);

  ush_init(&ush, &ush_desc);

  ush_commands_add(&ush, &cmd, cmd_files, sizeof(cmd_files) / sizeof(cmd_files[0]));

  // mount directories (root must be first)
  ush_node_mount(&ush, "/", &root, root_files,sizeof(root_files) / sizeof(root_files[0]));
  ush_node_mount(&ush, "/dev", &dev, dev_files, sizeof(dev_files) / sizeof(dev_files[0]));
  ush_node_mount(&ush, "/bin", &bin, bin_files, sizeof(bin_files) / sizeof(bin_files[0]));
}

void forever_blink(void) {
  for (;;) {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(500);

    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(500);

    tight_loop_contents();
  }
}


void scan_i2c_bus() {
  I2C i2c = I2C();
  printf("\n I2C Bus Scan\n");
  printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F \n");
  for (int addr = 0; addr < (1 << 7); ++addr) {
    if (addr % 16 == 0) printf("%02x ", addr);
    int ret;
    uint8_t rxdata;
    if (reserved_addr(addr))
      ret = PICO_ERROR_GENERIC;
    else
      ret = i2c.read_blocking(addr, &rxdata, 1, false);
    printf(ret < 0 ? "." : "@");
    printf(addr % 16 == 15 ? "\n" : "  ");
  }
  printf("Done with i2c scan...\n");
}

