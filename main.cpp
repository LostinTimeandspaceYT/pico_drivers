#include "pico/stdlib.h"
#include <stdio.h>

/* Debug Symbols */
#define DEBUG_AP33772 (false)
#define DEBUG_STUSB4500 (true)

#if DEBUG_AP33772
#include "ap33772/ap33772.hpp"
#endif

#if DEBUG_STUSB4500
#include "stusb4500/stusb4500.hpp"
#endif

/*Function Prototypes */
void setup(void);
void forever_blink(void);


int main() {
    
    setup();

    printf("Hello World\n");
    #ifdef DEBUG_STUSB4500
    STUSB4500* sink = STUSB4500::get_instance();
    #endif

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
    for(;;) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(500);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(500);
        tight_loop_contents();
    }
}