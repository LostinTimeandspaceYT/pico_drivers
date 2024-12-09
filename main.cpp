/** @file main.cpp
 *
 * @brief Entry point into the program
 *
 * @author Nathan Winslow
 */


#include "pico/stdlib.h"
#include "ush/picoshell.h"


/*Function Prototypes */
void setup(void);

int main() {
  setup();

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

