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

I2C i2c = I2C(4, 5);
I2CLCD lcd = I2CLCD(&i2c, 0x27, 20, 4);
char str[] = "hello world!\n";

/*Function Prototypes */
void setup(void);

int main() {
  setup();

  while (1) {
    /*picoshell_service();*/
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

  lcd.put_str((uint8_t *)str);

  /*picoshell_init();*/
}

