
#include "pico/stdlib.h"
#include "ssd1306/ssd1306.hpp"
#include "ush/picoshell.h"

static OLED oled_display(64, 128, false);

void setup();

int main() {
  setup();

  while (true) {
    picoshell_service();
  }
}

void setup() {
  stdio_init_all();
  sleep_ms(2000);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
  gpio_put(PICO_DEFAULT_LED_PIN, 1);

  picoshell_init();

  oled_display.clear_buffer();
  oled_display.print(0, 0,
                     reinterpret_cast<uint8_t *>(const_cast<char *>("TPS25750 Demo")));
  oled_display.show();
}
