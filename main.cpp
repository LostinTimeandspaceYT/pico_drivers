
#include <cstdint>
#include <cstdio>
#include <pico_drivers/display/ssd1306/font/ssd1306_font.hpp>
#include <pico_drivers/display/ssd1306/ssd1306.hpp>
#include <pico_drivers/low_pass_filter/low_pass_filter.hpp>
#include <pico/time.h>


#include "pico/stdlib.h"
#include "ush/picoshell.h"


// GLOBALS
static OLED oled_display(64, 128, false);

// GLOBAL ACCESSORS
OLED &get_oled_display() { return oled_display; }

void setup(void) {

    LowPassFilter lpf = LowPassFilter();
    lpf.reconfigure_filter(20.0f, 200.0f);

    stdio_init_all();
    sleep_ms(1000);
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    oled_display.set_rotation(OLED::Rotation::Deg0);
    oled_display.enable_double_buffer(true);
    oled_display.set_font(&SSD1306_Font);
    oled_display.clear_buffer();
    oled_display.set_text_wrap(true);
    oled_display.show();

    picoshell_init();

}

int main() {

    setup();

    while (true) {
        picoshell_service();
        tight_loop_contents();
    }
}
