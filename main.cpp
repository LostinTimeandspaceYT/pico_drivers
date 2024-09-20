#include "i2c/i2c.hpp"
#include "pico/stdlib.h"
#include "ssd1306/bitmap.hpp"
#include <cstdint>
#include <stdio.h>

#include "ush/microshell.h"

/* Debug Symbols */
#define DEBUG_AP33772 (false)
#define DEBUG_STUSB4500 (false)
#define DEBUG_SSD1306 (true)

#if DEBUG_AP33772
#include "ap33772/ap33772.hpp"
#endif

#if DEBUG_STUSB4500
#include "stusb4500/stusb4500.hpp"
#endif

#if DEBUG_SSD1306
#include "ssd1306/ssd1306.hpp"
#endif

/*Function Prototypes */
void setup(void);
void forever_blink(void);
void scan_i2c_bus();


// working buffers allocations (size could be customized)
#define BUF_IN_SIZE    32
#define BUF_OUT_SIZE   32
#define PATH_MAX_SIZE  32

static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];


// root directory handler
static struct ush_node_object root;

// microshell instance handler
static struct ush_object ush;


// non-blocking read interface
static int ush_read(struct ush_object *self, char *ch)
{
    // should be implemented as a FIFO
    if (stdio_get_until(ch, 1, PICO_ERROR_TIMEOUT)> 0) {
        *ch = getchar();
        return 1;
    }
    return 0;
}

// non-blocking write interface
static int ush_write(struct ush_object *self, char ch)
{
    // should be implemented as a FIFO
    return (printf("%c", ch) == 1);
}

// I/O interface descriptor
static const struct ush_io_interface ush_iface = {
    .read = ush_read,
    .write = ush_write,
};

// microshell descriptor
static const struct ush_descriptor ush_desc = {
    .io = &ush_iface,                           // I/O interface pointer
    .input_buffer = ush_in_buf,                 // working input buffer
    .input_buffer_size = sizeof(ush_in_buf),    // working input buffer size
    .output_buffer = ush_out_buf,               // working output buffer
    .output_buffer_size = sizeof(ush_out_buf),  // working output buffer size
    .path_max_length = PATH_MAX_SIZE,           // path maximum length (stack)
    .hostname = "Pico2",                      // hostname (in prompt)
};

int main() {

  setup();
  scan_i2c_bus();
  OLED oled = OLED(64, 128, true);
  oled.draw_bitmap(0, 0, 40, 32, sun_with_clouds_40x32);
  oled.print(0, 36, (uint8_t *)"Hello World!");
  oled.show();

  while (1) {
    // ush_service(&ush);
    forever_blink();
  }

  return 0;
}

void setup(void) {
  setup_default_uart();
  stdio_init_all();
  sleep_ms(2000); // Internet suggests sleeping after calling stdio_init_all();

  /* Make sure the pico is alive... */
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
  gpio_put(PICO_DEFAULT_LED_PIN, 1);

  ush_init(&ush, &ush_desc);

  // ush_node_mount(&ush, "/", &root, NULL, 0);
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

bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scan_i2c_bus() {
  I2C i2c = I2C();
  printf("\n I2C Bus Scan\n");
  printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F \n");
  for (int addr = 0; addr < (1 << 7); ++addr) {
    if (addr % 16 == 0)
      printf("%02x ", addr);
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
