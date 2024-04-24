#include "pico/stdlib.h"
#include <stdio.h>

#include "ap33772/ap33772.hpp"


void setup() {
    setup_default_uart();
    stdio_init_all();
    sleep_ms(2000); // Internet suggests sleeping after calling stdio_init_all();

    /* Make sure the pico is alive... */
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void blink_led(uint16_t delay_ms) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(delay_ms);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(delay_ms);
}

void loop() {
    for(;;) {
        blink_led(500);
        tight_loop_contents();
    }
}

void tear_down() {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    gpio_deinit(PICO_DEFAULT_LED_PIN);
}

int main() {

    setup();
    printf("Hello World\n");
    AP33772 *driver = AP33772::get_instance();
    loop();
    tear_down();
    return 0;
}