/** @file main.cpp
 *
 * @brief Entry point into the program
 *
 * @author Nathan Winslow
 */


#include "pico/stdlib.h"
#include "ush/picoshell.h"
#include "lcd/i2c_lcd/i2c_lcd.hpp"
#include "i2c/i2c.hpp"

char str[] = "hello world!\n";

/*Function Prototypes */
void setup(void);

int main() {
  setup();
  sleep_ms(500);
  I2CLCD lcd = I2CLCD(20, 4);
  sleep_ms(500);


  while (1) {
    picoshell_service();
  }

  return 0;
}

void setup(void) {
  /*setup_default_uart();*/
  stdio_init_all();
  sleep_ms(2000);

  /* Make sure the pico is alive... */
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
  gpio_put(PICO_DEFAULT_LED_PIN, 1);

  picoshell_init();
}

