#include "i2c/i2c.hpp"
#include "pico/stdlib.h"
#include "ssd1306/bitmap.hpp"
#include <cstdint>
#include <stdio.h>

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

int main() {

  setup();
  scan_i2c_bus();
  OLED oled = OLED(64, 128, true);
  oled.draw_bitmap(0, 0, 40, 32, sun_with_clouds_40x32);
  oled.print(0, 36, (uint8_t *)"Hello World!");
  oled.show();

  forever_blink();

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
