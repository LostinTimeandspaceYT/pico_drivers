#include <pico/time.h>

#include <cstdio>
#include <pico_drivers/display/ssd1306/ssd1306.hpp>

#include "pico/stdlib.h"
#include "ush/picoshell.h"

static constexpr int STATUS_UPDATE_PERIOD_MS = 500;

static OLED oled_display(64, 128, false);
static repeating_timer status_timer;
static uint32_t counter = 0;

static bool status_timer_callback(repeating_timer *rt) {
  (void)rt;

  oled_display.clear_buffer();
  oled_display.set_cursor(0, 0);
  oled_display.set_text_wrap(false);
  oled_display.printf("Counter: %lu", counter++);
  gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
  oled_display.show();

  return true;  // keep timer running
}

int main() {
  stdio_init_all();
  sleep_ms(1000);
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
  gpio_put(PICO_DEFAULT_LED_PIN, 0);

  oled_display.set_rotation(OLED::Rotation::Deg0);
  oled_display.enable_double_buffer(true);
  oled_display.clear_buffer();
  oled_display.set_text_wrap(false);
  oled_display.set_cursor(0, 0);
  oled_display.printf("Counter: 0");
  oled_display.show();

  add_repeating_timer_ms(STATUS_UPDATE_PERIOD_MS, status_timer_callback, nullptr,
                         &status_timer);

  picoshell_init();

  while (true) {
    picoshell_service();
    tight_loop_contents();
  }
}
