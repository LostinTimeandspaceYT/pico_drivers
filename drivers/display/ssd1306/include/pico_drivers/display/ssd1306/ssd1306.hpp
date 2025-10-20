/** @file ssd1306.hpp
 *
 * @brief
 *
 * @par
 * This module is based on the official pico examples, and the work of
 * MR-Addict and jfoucher on GitHub. It has been modified to work with this
 * libraries I2C wrapper class.
 *
 * @author Nathan Winslow
 *
 * @cite https://github.com/MR-Addict/Pi-Pico-SSD1306-C-Library/tree/main
 */

#ifndef _SSD1306_H
#define _SSD1306_H

#include <algorithm>

#include <pico_drivers/display/display_context.hpp>
#include <pico_drivers/display/display_power_manager.hpp>
#include <pico_drivers/display/frame_streamer.hpp>
#include <pico_drivers/display/gfx_font.hpp>
#include <pico_drivers/display/ssd1306/bitmap.hpp>
#include <pico_drivers/i2c/i2c.hpp>

#include "pico/stdlib.h"

static constexpr uint8_t OLED_ADDRESS = 0x3C;

static constexpr uint8_t SET_CONTRAST = 0x81;
static constexpr uint8_t SET_ENTIRE_ON = 0xA4;
static constexpr uint8_t SET_NORM_INV = 0xA6;
static constexpr uint8_t SET_DISP = 0xAE;
static constexpr uint8_t SET_MEM_ADDR = 0x20;
static constexpr uint8_t SET_COL_ADDR = 0x21;
static constexpr uint8_t SET_PAGE_ADDR = 0x22;
static constexpr uint8_t SET_DISP_START_LINE = 0x40;
static constexpr uint8_t SET_SEG_REMAP = 0xA0;
static constexpr uint8_t SET_MUX_RATIO = 0xA8;
static constexpr uint8_t SET_DISP_OFFSET = 0xD3;
static constexpr uint8_t SET_COM_PIN_CFG = 0xDA;
static constexpr uint8_t SET_DISP_CLK_DIV = 0xD5;
static constexpr uint8_t SET_PRECHARGE = 0xD9;
static constexpr uint8_t SET_VCOM_DESEL = 0xDB;
static constexpr uint8_t SET_CHARGE_PUMP = 0x8D;
static constexpr uint8_t SET_SCROLL = 0x2E;
static constexpr uint8_t RIGHT_HORIZONTAL_SCROLL = 0x26;
static constexpr uint8_t LEFT_HORIZONTAL_SCROLL = 0x27;
static constexpr uint8_t VERTICAL_RIGHT_SCROLL = 0x29;
static constexpr uint8_t VERTICAL_LEFT_SCROLL = 0x2A;
static constexpr uint8_t SET_COM_OUT_DIR_REVERSE = 0xC0;
static constexpr uint8_t SET_COM_OUT_DIR_NORMAL = 0xC8;
static constexpr uint8_t SET_VERTICAL_SCROLL_AREA = 0xA3;

class OLED {
  public:
    enum class Rotation {
        Deg0,
        Deg90,
        Deg180,
        Deg270,
    };

    enum class ScrollDirection {
        Left,
        Right,
    };

    enum class BitmapBlitMode {
        Copy,
        Or,
        And,
        Xor,
    };

    OLED(uint8_t height, uint8_t width, bool reversed);
    OLED(I2C i2c, uint8_t height, uint8_t width, bool reversed);
    ~OLED();

    void show();
    void clear_buffer();
    void is_display(bool inverse);
    void is_inverse(bool inverse);
    void set_contrast(uint8_t contrast);
    void set_font(const GFXfont *font);
    void set_rotation(Rotation rotation);
    void enable_double_buffer(bool enable);
    void swap_buffers();
    uint8_t *back_buffer();
    const uint8_t *front_buffer() const;
    uint8_t width_px() const { return width; }
    uint8_t height_px() const { return height; }
    void set_cursor(uint8_t x, uint8_t y);
    uint8_t cursor_x_pos() const {
        return static_cast<uint8_t>(display_context.text_renderer.cursor_x());
    }
    uint8_t cursor_y_pos() const {
        return static_cast<uint8_t>(display_context.text_renderer.cursor_y());
    }
    void set_text_wrap(bool wrap);
    void write_char(uint8_t character);
    void write(const char *str);
    void println(const char *str);
    void println();
    void printf(const char *fmt, ...);
    void wrap_text(uint8_t x, uint8_t y, const char *str, uint8_t max_width);

    /* Methods for drawing to display */

    /**
     * @brief draws a horizontal line onto the display.
     *
     * @param x starting x position
     * @param y starting y position
     * @param width length of the line in pixels
     */
    void draw_fast_hline(uint8_t x, uint8_t y, uint8_t width);

    /**
     * @brief draws a vertical line onto the display.
     *
     * @param x starting x position
     * @param y starting y position
     * @param height length of the line in pixels
     */
    void draw_fast_vline(uint8_t x, uint8_t y, uint8_t height);

    /**
     * @brief draws a line from two end-points.
     *
     * @param x1 x1 position
     * @param y1 y1 position
     * @param x2 x2 position
     * @param y2 y2 position
     */
    void draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
    void draw_triangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
    void draw_filled_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                              int16_t y2);

    /**
     * @brief Draws an outline of a rectangle starting from x,y with dimensions
     * width * height
     *
     * @param x starting x position
     * @param y starting y position
     * @param width pixels in the x direction
     * @param height pixels in the y direction
     */
    void draw_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

    /**
     * @brief Like draw_rectangle but colors in the shape.
     *
     * @param x starting x position
     * @param y starting y position
     * @param width pixels in the x direction
     * @param height pixels in the y direction
     */
    void draw_filled_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

    /**
     * @brief Draws a circle from xc,yc as its centerpoint.
     *
     * @param xc x coordinate of circle's center
     * @param yc y coordinate of circle's center
     * @param radius length of radius in pixels
     */
    void draw_circle(int16_t xc, int16_t yc, uint16_t radius);
    void draw_rounded_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                                uint8_t radius);

    /**
     * @brief Like draw_circle but colors in the shape.
     *
     * @param xc x coordinate of circle's center
     * @param yc y coordinate of circle's center
     * @param radius length of radius in pixels
     */
    void draw_filled_circle(int16_t xc, int16_t yc, uint16_t radius);
    void draw_filled_rounded_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                                       uint8_t radius);
    void draw_quadratic_bezier(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                               int16_t y2);

    /**
     * @brief Push only the specified region of the back buffer to the display.
     */
    void update_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

    /**
     * @brief Configure the vertical scroll area.
     *
     * @param top_fixed_rows Number of rows at the top that remain static.
     * @param scroll_rows Number of rows that participate in vertical scrolling.
     */
    void set_vertical_scroll_area(uint8_t top_fixed_rows, uint8_t scroll_rows);

    /**
     * @brief Start a horizontal scroll between two page addresses.
     *
     * @param direction ScrollDirection::Right or ::Left.
     * @param start_page First page (0-7) included in the scroll region.
     * @param end_page Last page (0-7) included in the scroll region.
     * @param frame_interval Frame interval value (0-7) defined by the datasheet.
     */
    void start_horizontal_scroll(ScrollDirection direction, uint8_t start_page, uint8_t end_page,
                                 uint8_t frame_interval);

    /**
     * @brief Start a combined vertical and horizontal scroll.
     *
     * @param direction ScrollDirection::Right or ::Left.
     * @param start_page First page (0-7) included in the scroll region.
     * @param end_page Last page (0-7) included in the scroll region.
     * @param frame_interval Frame interval value (0-7) defined by the datasheet.
     * @param vertical_offset Number of rows to shift each frame (0-63).
     */
    void start_diagonal_scroll(ScrollDirection direction, uint8_t start_page, uint8_t end_page,
                               uint8_t frame_interval, uint8_t vertical_offset);

    /**
     * @brief Stop any active scrolling effect.
     */
    void stop_scroll();

    /**
     * @brief Displays a single character.
     *
     * @param x starting x position
     * @param y starting y position
     * @param ch hex value of character to print (0 - 255)
     */
    void print_char(uint8_t x, uint8_t y, uint8_t ch);

    /**
     * @brief Displays a string of characters.
     *
     * @param x starting x position
     * @param y starting y position
     * @param str pointer to character string.
     */
    void print(uint8_t x, uint8_t y, uint8_t *str);
    void printf(uint8_t x, uint8_t y, const char *fmt, ...);

    /**
     * @brief Draws a bitmap image.
     *
     * @param x starting x position
     * @param y starting y position
     * @param width width of img
     * @param height height of img
     * @param img reference to the bitmap
     */
    void draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *img,
                     BitmapBlitMode mode = BitmapBlitMode::Or, const uint8_t *mask = nullptr);

    void set_power_save(bool enable);
    void dim(bool enable);
    void fade_in(uint16_t duration_ms, uint8_t steps = 32);
    void fade_out(uint16_t duration_ms, uint8_t steps = 32);
    void horizontal_wipe(bool left_to_right, uint16_t duration_ms, uint8_t chunk_width = 8);

  private:
    I2C i2c;
    uint8_t width;
    uint8_t height;
    uint8_t pages;
    uint16_t buff_size;
    bool reversed;
    Rotation rotation = Rotation::Deg0;
    static constexpr uint16_t MAX_BUFFER_SIZE = 1024;
    DisplayContext display_context;
    DisplayPowerManager power_manager;

    void init(void);
    void write_cmd(uint8_t cmd);
    void write_data(uint8_t data);
    void swap(uint8_t *x1, uint8_t *x2);
    void set_pixel(int16_t x, int16_t y);
    void apply_pixel(uint8_t x, uint8_t y, bool value, BitmapBlitMode mode);
    void blit_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap,
                     uint8_t stride, BitmapBlitMode mode, const uint8_t *mask);
    void animate_contrast(uint8_t start, uint8_t end, uint16_t duration_ms, uint8_t steps,
                          bool persist_tracking, bool update_restore);
    DisplayContext make_display_context(uint8_t width, uint8_t height, uint16_t buffer_size);

    template <typename Fn> void for_each_pixel_span(int16_t x, int16_t y, uint16_t length, Fn &&fn);
};

template <typename Fn>
void OLED::for_each_pixel_span(int16_t x, int16_t y, uint16_t length, Fn &&fn) {
    if (length == 0) {
        return;
    }
    if (y < 0 || y >= static_cast<int16_t>(height)) {
        return;
    }
    uint16_t start_index = 0;
    if (x < 0) {
        uint16_t skip = static_cast<uint16_t>(std::min<int16_t>(-x, static_cast<int16_t>(length)));
        if (skip >= length) {
            return;
        }
        x += static_cast<int16_t>(skip);
        length -= skip;
        start_index = skip;
    }
    if (x >= static_cast<int16_t>(width)) {
        return;
    }
    uint16_t max_length = static_cast<uint16_t>(width) - static_cast<uint16_t>(x);
    if (length > max_length) {
        length = max_length;
    }

    uint8_t *buffer = display_context.frame_buffer.back_buffer();
    if (buffer == nullptr) {
        return;
    }

    uint16_t row = static_cast<uint16_t>(y) / 8;
    uint8_t mask = static_cast<uint8_t>(0x01 << (y & 0x07));
    uint16_t offset = row * static_cast<uint16_t>(width) + static_cast<uint16_t>(x);

    for (uint16_t i = 0; i < length; ++i) {
        fn(buffer[offset + i], mask, static_cast<uint16_t>(start_index + i));
    }
}

#endif // end _SSD1306_H
/* END OF FILE */
