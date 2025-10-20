#ifndef PICO_DRIVERS_DISPLAY_GFX_FONT_HPP_
#define PICO_DRIVERS_DISPLAY_GFX_FONT_HPP_

#include <cstdint>

struct GFXglyph {
    uint16_t bitmap_offset; // Offset into bitmap array
    uint8_t width;
    uint8_t height;
    uint8_t x_advance;
    int8_t x_offset;
    int8_t y_offset;
};

struct GFXfont {
    uint8_t *bitmap;
    GFXglyph *glyph;
    uint8_t first_char;
    uint8_t last_char;
    uint8_t y_advance;
};

#endif // PICO_DRIVERS_DISPLAY_GFX_FONT_HPP_
