/** @file ssd1306.cpp
 *
 * @brief this module defines the ssd1306 OLED display interface.
 *
 * @author Nathan Winslow
 *
 * @par
 * For more information, see the README file.
 *
 * @cite https://github.com/MR-Addict/Pi-Pico-SSD1306-C-Library/tree/main
 *
 * Enhancements:
 * - Basic Text Layout – convenience APIs now include printf(fmt, ...), set_cursor(x, y),
 *   and wrap_text(...) so strings can be positioned without manual pre-calculation.
 * - Bitmap Blitting Enhancements – transparency masks and bitwise operations
 *   (Copy/OR/XOR/AND) make sprite composition flexible.
 * - Drawing Primitives Expansion – adds Bézier curves, rounded rectangles, and
 *   triangle fills similar to Adafruit GFX.
 * - Brightness/Power Profiles – in addition to set_contrast, utility helpers expose
 *   set_power_save(on/off) and dim(bool) for charge-pump and precharge control.
 * - Animations & Transitions – helper functions support fade-in/fade-out and simple
 *   wipe transitions driven by partial updates.
 */

#include <pico_drivers/display/ssd1306/ssd1306.hpp>

#include <hardware/dma.h>
#include <hardware/i2c.h>
#include <hardware/regs/i2c.h>
#include <pico/assert.h>
#include <pico/platform.h>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <pico_drivers/display/ssd1306/font/dialog_bold_16.hpp>
#include <pico_drivers/display/ssd1306/font/ssd1306_font.hpp>

/* Constructors */
OLED::OLED(uint8_t height, uint8_t width, bool reversed)
    : i2c(I2C()), width(width), height(height), pages(height / 8),
      buff_size(static_cast<uint16_t>(width * pages)), reversed(reversed),
      display_context(make_display_context(width, height, buff_size)) {
    hard_assert(buff_size <= MAX_BUFFER_SIZE);
    enable_double_buffer(true);
    init();
    clear_buffer();
    show();
}

OLED::OLED(I2C i2c, uint8_t height, uint8_t width, bool reversed)
    : i2c(i2c), width(width), height(height), pages(height / 8),
      buff_size(static_cast<uint16_t>(width * pages)), reversed(reversed),
      display_context(make_display_context(width, height, buff_size)) {
    hard_assert(buff_size <= MAX_BUFFER_SIZE);
    enable_double_buffer(true);
    init();
    clear_buffer();
    show();
}

OLED::~OLED() = default;

/* Private Methods */
DisplayContext OLED::make_display_context(uint8_t ctx_width, uint8_t ctx_height,
                                         uint16_t buffer_size) {
    auto pixel_writer = [this](int16_t px, int16_t py) { this->set_pixel(px, py); };
    auto span_writer = [this](int16_t sx, int16_t sy, uint16_t len) {
        this->for_each_pixel_span(
            sx, sy, len, [](uint8_t &cell, uint8_t mask, uint16_t) { cell |= mask; });
    };
    auto text_writer = [this](int16_t px, int16_t py, bool on) {
        if (on) {
            this->set_pixel(px, py);
        }
    };

    FrameStreamer streamer(
        std::make_unique<I2CFrameTransport>(i2c, OLED_ADDRESS, buffer_size));
    DisplayContext context(ctx_width, ctx_height, std::move(streamer), std::move(pixel_writer),
                           std::move(span_writer), std::move(text_writer), &Dialog_bold_16);
    return context;
}

void OLED::init() {
    write_cmd(SET_DISP | 0x00);

    /* Set horizontal address mode */
    write_cmd(SET_MEM_ADDR);
    write_cmd(0x00);

    /* Set the seg-map */
    if (reversed)
        write_cmd(SET_SEG_REMAP);
    else
        write_cmd(SET_SEG_REMAP | 0x01);

    /* set the display offset */
    write_cmd(SET_DISP_OFFSET);
    write_cmd(0x00);

    /* Set COM pins hardware configuration, 0x12 for 128x64 and 0x02 for 128x32 */
    write_cmd(SET_COM_PIN_CFG);

    if (height == 64)
        write_cmd(0x12); // TODO: may want to change to switch case
    else if (height == 32)
        write_cmd(0x02);

    write_cmd(SET_DISP_CLK_DIV);
    write_cmd(0x80);

    write_cmd(SET_PRECHARGE);
    write_cmd(0xf1);

    write_cmd(SET_VCOM_DESEL);
    write_cmd(0x30);

    write_cmd(SET_CONTRAST);
    write_cmd(0xff);

    /* Set OLED on following from RAM */
    write_cmd(SET_ENTIRE_ON);

    // NO inverse, '0' for pixel off, '1' for pixel on
    write_cmd(SET_NORM_INV);

    write_cmd(SET_CHARGE_PUMP);
    write_cmd(0x14);

    /* Set scroll to off */
    write_cmd(SET_SCROLL | 0x00);

    /* Turn the OLED on */
    write_cmd(SET_DISP | 0x01);
    display_context.shape_rasterizer.set_bounds(width, height);
    display_context.text_renderer.set_bounds(width, height);
    power_manager.reset(0xFF);
}

void OLED::write_cmd(uint8_t cmd) {
    /* 0x00 writes a command */
    uint8_t buff[] = {0x00, cmd};
    i2c.write_blocking(OLED_ADDRESS, buff, 2, false);
}

void OLED::write_data(uint8_t data) {
    // 0x40 writes data
    uint8_t buff[] = {0x40, data};
    i2c.write_blocking(OLED_ADDRESS, buff, 2, false);
}

void OLED::swap(uint8_t *x1, uint8_t *x2) {
    uint8_t tmp = *x1;
    *x1 = *x2;
    *x2 = tmp;
}

void OLED::is_display(bool display) { write_cmd(SET_DISP | display); }

void OLED::set_contrast(uint8_t contrast) {
    write_cmd(SET_CONTRAST);
    write_cmd(contrast);
    power_manager.on_contrast_written(contrast);
}

void OLED::set_rotation(Rotation desired_rotation) {
    // Temporarily disable the panel while scan direction is updated.
    write_cmd(SET_DISP | 0x00);

    rotation = desired_rotation;
    switch (rotation) {
    case Rotation::Deg0:
        write_cmd(SET_SEG_REMAP | 0x01);   // column 0 maps to SEG0
        write_cmd(SET_COM_OUT_DIR_NORMAL); // scan from COM[N-1] to COM0
        reversed = false;
        break;
    case Rotation::Deg180:
        write_cmd(SET_SEG_REMAP);           // column 127 maps to SEG0
        write_cmd(SET_COM_OUT_DIR_REVERSE); // scan from COM0 to COM[N-1]
        reversed = true;
        break;
    case Rotation::Deg90:
    case Rotation::Deg270:
        // 90° and 270° orientations need full buffer remapping which is not yet supported.
        write_cmd(SET_SEG_REMAP | 0x01);
        write_cmd(SET_COM_OUT_DIR_NORMAL);
        reversed = false;
        break;
    }

    write_cmd(SET_DISP | 0x01);
}

void OLED::enable_double_buffer(bool enable) {
    display_context.frame_buffer.enable_double_buffer(enable);
}

void OLED::swap_buffers() { display_context.frame_buffer.swap_buffers(); }

uint8_t *OLED::back_buffer() { return display_context.frame_buffer.back_buffer(); }

const uint8_t *OLED::front_buffer() const { return display_context.frame_buffer.front_buffer(); }

void OLED::set_cursor(uint8_t x, uint8_t y) { display_context.text_renderer.set_cursor(x, y); }

void OLED::set_text_wrap(bool wrap) { display_context.text_renderer.set_text_wrap(wrap); }

void OLED::write_char(uint8_t character) { display_context.text_renderer.write_char(character); }

void OLED::write(const char *str) { display_context.text_renderer.write(str); }

void OLED::println(const char *str) { display_context.text_renderer.println(str); }

void OLED::println() { display_context.text_renderer.println(); }

void OLED::printf(const char *fmt, ...) {
    if (fmt == nullptr) {
        return;
    }

    char formatted[128];

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(formatted, sizeof(formatted), fmt, args);
    va_end(args);

    if (written < 0) {
        return;
    }
    if (written >= static_cast<int>(sizeof(formatted))) {
        formatted[sizeof(formatted) - 1] = '\0';
    }

    display_context.text_renderer.write(formatted);
}

void OLED::wrap_text(uint8_t x, uint8_t y, const char *str, uint8_t max_width) {
    display_context.text_renderer.wrap_text(x, y, str, max_width);
}

void OLED::is_inverse(bool inverse) { write_cmd(SET_NORM_INV | inverse); }

void OLED::clear_buffer() { display_context.frame_buffer.clear(); }

void OLED::show() {
    /* Set col, row, and page address for sending buffer */
    write_cmd(SET_COL_ADDR);
    write_cmd(0);
    write_cmd(width - 1);
    write_cmd(SET_PAGE_ADDR);
    write_cmd(0);
    write_cmd(pages - 1);

    bool use_double_buffer = display_context.frame_buffer.double_buffer_enabled();
    if (use_double_buffer) {
        swap_buffers();
    }

    const uint8_t *frame = front_buffer();
    if (frame == nullptr) {
        return;
    }
    display_context.frame_streamer.start_transfer(frame, buff_size);

    if (use_double_buffer) {
        display_context.frame_buffer.copy_front_to_back();
    }
}

void OLED::set_pixel(int16_t x, int16_t y) {
    if (x < 0 || y < 0 || x >= static_cast<int16_t>(width) || y >= static_cast<int16_t>(height)) {
        return;
    }
    uint8_t *buffer = display_context.frame_buffer.back_buffer();
    if (buffer == nullptr) {
        return;
    }
    uint16_t row = static_cast<uint16_t>(y) / 8;
    uint16_t offset = static_cast<uint16_t>(x) + static_cast<uint16_t>(width) * row;
    uint8_t mask = static_cast<uint8_t>(0x01 << (y % 8));
    buffer[offset] |= mask;
}

void OLED::apply_pixel(uint8_t x, uint8_t y, bool value, BitmapBlitMode mode) {
    if (x >= width || y >= height) {
        return;
    }

    uint16_t offset = x + width * (y / 8);
    uint8_t mask = static_cast<uint8_t>(0x01 << (y % 8));
    uint8_t *buffer = display_context.frame_buffer.back_buffer();
    if (buffer == nullptr) {
        return;
    }
    uint8_t &cell = buffer[offset];

    switch (mode) {
    case BitmapBlitMode::Copy:
        if (value) {
            cell |= mask;
        } else {
            cell &= static_cast<uint8_t>(~mask);
        }
        break;
    case BitmapBlitMode::Or:
        if (value) {
            cell |= mask;
        }
        break;
    case BitmapBlitMode::And:
        if (!value) {
            cell &= static_cast<uint8_t>(~mask);
        }
        break;
    case BitmapBlitMode::Xor:
        if (value) {
            cell ^= mask;
        }
        break;
    }
}

void OLED::animate_contrast(uint8_t start, uint8_t end, uint16_t duration_ms, uint8_t steps,
                            bool persist_tracking, bool update_restore) {
    if (steps == 0) {
        steps = 1;
    }

    power_manager.suspend_contrast_persist(true);
    set_contrast(start);

    int16_t delta = static_cast<int16_t>(end) - static_cast<int16_t>(start);
    uint16_t delay = duration_ms / steps;

    for (uint8_t i = 1; i <= steps; ++i) {
        int16_t interpolated = static_cast<int16_t>(start) + (delta * i) / steps;
        uint8_t value = static_cast<uint8_t>(std::clamp<int16_t>(interpolated, 0, 0xFF));
        set_contrast(value);
        if (delay > 0) {
            sleep_ms(delay);
        }
    }

    power_manager.suspend_contrast_persist(false);

    if (persist_tracking) {
        set_contrast(end);
    } else {
        power_manager.on_contrast_written(end);
        if (update_restore && !power_manager.dimmed()) {
            power_manager.set_fade_restore_contrast(end);
        }
    }

    if (update_restore && persist_tracking) {
        power_manager.set_fade_restore_contrast(end);
    }
}

void OLED::update_region(uint8_t x, uint8_t y, uint8_t region_width, uint8_t region_height) {
    if (region_width == 0 || region_height == 0) {
        return;
    }

    if (x >= width || y >= height) {
        return;
    }

    if (x + region_width > width) {
        region_width = width - x;
    }
    if (y + region_height > height) {
        region_height = height - y;
    }

    uint8_t start_page = y / 8;
    uint8_t end_page = (y + region_height - 1) / 8;

    uint8_t *front = display_context.frame_buffer.front_buffer();
    const uint8_t *back = display_context.frame_buffer.back_buffer();

    if (front == nullptr || back == nullptr) {
        return;
    }

    if (display_context.frame_buffer.double_buffer_enabled()) {
        for (uint8_t page = start_page; page <= end_page; ++page) {
            uint16_t offset = (page * width) + x;
            std::memcpy(front + offset, back + offset, region_width);
        }
    }

    const uint8_t *frame = front;

    for (uint8_t page = start_page; page <= end_page; ++page) {
        write_cmd(SET_COL_ADDR);
        write_cmd(x);
        write_cmd(x + region_width - 1);
        write_cmd(SET_PAGE_ADDR);
        write_cmd(page);
        write_cmd(page);

        uint16_t offset = (page * width) + x;
        display_context.frame_streamer.start_transfer(frame + offset, region_width);
    }
}

void OLED::draw_fast_hline(uint8_t x, uint8_t y, uint8_t width) {
    display_context.shape_rasterizer.draw_fast_hline(x, y, width);
}

void OLED::draw_fast_vline(uint8_t x, uint8_t y, uint8_t height) {
    display_context.shape_rasterizer.draw_fast_vline(x, y, height);
}

void OLED::draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    display_context.shape_rasterizer.draw_line(x1, y1, x2, y2);
}

void OLED::draw_triangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    display_context.shape_rasterizer.draw_triangle(x0, y0, x1, y1, x2, y2);
}

void OLED::draw_filled_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                                int16_t y2) {
    display_context.shape_rasterizer.draw_filled_triangle(x0, y0, x1, y1, x2, y2);
}

void OLED::draw_circle(int16_t xc, int16_t yc, uint16_t radius) {
    display_context.shape_rasterizer.draw_circle(xc, yc, radius);
}

void OLED::draw_filled_circle(int16_t xc, int16_t yc, uint16_t radius) {
    display_context.shape_rasterizer.draw_filled_circle(xc, yc, radius);
}

void OLED::draw_quadratic_bezier(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                                 int16_t y2) {
    display_context.shape_rasterizer.draw_quadratic_bezier(x0, y0, x1, y1, x2, y2);
}

void OLED::draw_rectangle(uint8_t x, uint8_t y, uint8_t rect_width, uint8_t rect_height) {
    display_context.shape_rasterizer.draw_rectangle(x, y, rect_width, rect_height);
}

void OLED::draw_rounded_rectangle(uint8_t x, uint8_t y, uint8_t rect_width, uint8_t rect_height,
                                  uint8_t radius) {
    display_context.shape_rasterizer.draw_rounded_rectangle(x, y, rect_width, rect_height, radius);
}

void OLED::draw_filled_rectangle(uint8_t x, uint8_t y, uint8_t rect_width, uint8_t rect_height) {
    display_context.shape_rasterizer.draw_filled_rectangle(x, y, rect_width, rect_height);
}

void OLED::draw_filled_rounded_rectangle(uint8_t x, uint8_t y, uint8_t rect_width,
                                         uint8_t rect_height, uint8_t radius) {
    display_context.shape_rasterizer.draw_filled_rounded_rectangle(x, y, rect_width, rect_height,
                                                                  radius);
}

void OLED::set_vertical_scroll_area(uint8_t top_fixed_rows, uint8_t scroll_rows) {
    if (top_fixed_rows + scroll_rows > height) {
        scroll_rows = (height > top_fixed_rows) ? (height - top_fixed_rows) : 0;
    }

    write_cmd(SET_VERTICAL_SCROLL_AREA);
    write_cmd(top_fixed_rows);
    write_cmd(scroll_rows);
}

void OLED::start_horizontal_scroll(ScrollDirection direction, uint8_t start_page, uint8_t end_page,
                                   uint8_t frame_interval) {
    if (start_page > end_page) {
        swap(&start_page, &end_page);
    }

    stop_scroll();

    uint8_t command =
        (direction == ScrollDirection::Right) ? RIGHT_HORIZONTAL_SCROLL : LEFT_HORIZONTAL_SCROLL;
    write_cmd(command);
    write_cmd(0x00);                  // dummy
    write_cmd(start_page & 0x07);     // start page address
    write_cmd(frame_interval & 0x07); // time interval
    write_cmd(end_page & 0x07);       // end page address
    write_cmd(0x00);                  // dummy
    write_cmd(0xFF);                  // dummy
    write_cmd(SET_SCROLL | 0x01);     // activate scroll
}

void OLED::start_diagonal_scroll(ScrollDirection direction, uint8_t start_page, uint8_t end_page,
                                 uint8_t frame_interval, uint8_t vertical_offset) {
    if (start_page > end_page) {
        swap(&start_page, &end_page);
    }

    stop_scroll();

    uint8_t command =
        (direction == ScrollDirection::Right) ? VERTICAL_RIGHT_SCROLL : VERTICAL_LEFT_SCROLL;
    write_cmd(command);
    write_cmd(0x00);                   // dummy
    write_cmd(start_page & 0x07);      // start page address
    write_cmd(frame_interval & 0x07);  // time interval
    write_cmd(end_page & 0x07);        // end page address
    write_cmd(vertical_offset & 0x3F); // vertical scroll offset (0-63)
    write_cmd(SET_SCROLL | 0x01);      // activate scroll
}

void OLED::stop_scroll() { write_cmd(SET_SCROLL | 0x00); }

void OLED::set_font(const GFXfont *font) { display_context.text_renderer.set_font(font); }

void OLED::print_char(uint8_t x, uint8_t y, uint8_t character) {
    display_context.text_renderer.draw_char(x, y, character);
}

void OLED::print(uint8_t x, uint8_t y, uint8_t *str) {
    if (str == nullptr) {
        return;
    }
    display_context.text_renderer.draw_string(x, y, reinterpret_cast<const char *>(str));
}

void OLED::printf(uint8_t x, uint8_t y, const char *fmt, ...) {
    if (fmt == nullptr)
        return;

    char formatted[128];

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(formatted, sizeof(formatted), fmt, args);
    va_end(args);

    if (written < 0)
        return;
    if (written >= static_cast<int>(sizeof(formatted))) {
        formatted[sizeof(formatted) - 1] = '\0';
    }

    print(x, y, reinterpret_cast<uint8_t *>(formatted));
}

void OLED::blit_bitmap(uint8_t x, uint8_t y, uint8_t bitmap_width, uint8_t bitmap_height,
                       const uint8_t *bitmap, uint8_t stride, BitmapBlitMode mode,
                       const uint8_t *mask) {
    if (bitmap == nullptr || stride == 0 || bitmap_width == 0 || bitmap_height == 0) {
        return;
    }

    for (uint8_t row = 0; row < bitmap_height; ++row) {
        int16_t dest_y = static_cast<int16_t>(y) + static_cast<int16_t>(row);
        const uint8_t *row_bits = bitmap + static_cast<uint16_t>(row) * stride;
        const uint8_t *row_mask =
            (mask != nullptr) ? (mask + static_cast<uint16_t>(row) * stride) : nullptr;

        for_each_pixel_span(
            static_cast<int16_t>(x), dest_y, bitmap_width,
            [&](uint8_t &cell, uint8_t bit_mask, uint16_t column_index) {
                if (row_mask != nullptr) {
                    bool allow =
                        ((row_mask[column_index / 8] >> (7 - (column_index % 8))) & 0x01) != 0;
                    if (!allow) {
                        return;
                    }
                }

                bool pixel_on =
                    ((row_bits[column_index / 8] >> (7 - (column_index % 8))) & 0x01) != 0;
                switch (mode) {
                case BitmapBlitMode::Copy:
                    if (pixel_on) {
                        cell |= bit_mask;
                    } else {
                        cell &= static_cast<uint8_t>(~bit_mask);
                    }
                    break;
                case BitmapBlitMode::Or:
                    if (pixel_on) {
                        cell |= bit_mask;
                    }
                    break;
                case BitmapBlitMode::And:
                    if (!pixel_on) {
                        cell &= static_cast<uint8_t>(~bit_mask);
                    }
                    break;
                case BitmapBlitMode::Xor:
                    if (pixel_on) {
                        cell ^= bit_mask;
                    }
                    break;
                }
            });
    }
}

void OLED::draw_bitmap(uint8_t x, uint8_t y, uint8_t bitmap_width, uint8_t bitmap_height,
                       const uint8_t *img, BitmapBlitMode mode, const uint8_t *mask) {
    if (img == nullptr || bitmap_width == 0 || bitmap_height == 0) {
        return;
    }
    uint8_t stride = static_cast<uint8_t>((bitmap_width + 7u) / 8u);
    if (stride == 0) {
        return;
    }
    blit_bitmap(x, y, bitmap_width, bitmap_height, img, stride, mode, mask);
}

void OLED::set_power_save(bool enable) {
    if (power_manager.power_save_enabled() == enable) {
        return;
    }

    if (enable) {
        write_cmd(SET_SCROLL | 0x00);
        write_cmd(SET_DISP | 0x00);
        write_cmd(SET_CHARGE_PUMP);
        write_cmd(0x10); // disable charge pump
        write_cmd(SET_PRECHARGE);
        write_cmd(0x00); // minimum precharge
    } else {
        write_cmd(SET_CHARGE_PUMP);
        write_cmd(0x14); // enable charge pump
        write_cmd(SET_PRECHARGE);
        write_cmd(0xF1); // default datasheet value
        write_cmd(SET_DISP | 0x01);
        display_context.shape_rasterizer.set_bounds(width, height);
        display_context.text_renderer.set_bounds(width, height);
        power_manager.suspend_contrast_persist(true);
        set_contrast(power_manager.current_contrast());
        power_manager.suspend_contrast_persist(false);
    }

    power_manager.set_power_save_enabled(enable);
}

void OLED::dim(bool enable) {
    constexpr uint8_t DIM_CONTRAST = 0x20;

    if (enable) {
        if (power_manager.dimmed()) {
            return;
        }

        uint8_t stored_contrast = power_manager.current_contrast();
        power_manager.set_dimmed(true);
        power_manager.set_previous_contrast(stored_contrast);
        animate_contrast(stored_contrast, DIM_CONTRAST, 0, 1, true, false);
    } else {
        if (!power_manager.dimmed()) {
            return;
        }

        power_manager.set_dimmed(false);
        animate_contrast(power_manager.current_contrast(), power_manager.previous_contrast(), 0, 1,
                         true, true);
    }
}

void OLED::fade_out(uint16_t duration_ms, uint8_t steps) {
    if (steps == 0) {
        steps = 1;
    }

    uint8_t start = power_manager.current_contrast();
    power_manager.set_fade_restore_contrast(start);
    animate_contrast(start, 0, duration_ms, steps, false, false);
}

void OLED::fade_in(uint16_t duration_ms, uint8_t steps) {
    if (steps == 0) {
        steps = 1;
    }

    uint8_t target = power_manager.fade_restore_contrast();
    if (target == 0) {
        target = power_manager.previous_contrast();
    }

    animate_contrast(power_manager.current_contrast(), target, duration_ms, steps, true, true);
}

void OLED::horizontal_wipe(bool left_to_right, uint16_t duration_ms, uint8_t chunk_width) {
    if (width == 0 || height == 0) {
        return;
    }

    if (chunk_width == 0) {
        chunk_width = 1;
    }

    uint8_t steps = static_cast<uint8_t>((width + chunk_width - 1) / chunk_width);
    if (steps == 0) {
        steps = 1;
    }
    uint16_t delay = duration_ms / steps;

    if (left_to_right) {
        for (uint8_t start = 0; start < width;) {
            uint8_t segment = std::min<uint8_t>(chunk_width, static_cast<uint8_t>(width - start));
            update_region(start, 0, segment, height);
            start = static_cast<uint8_t>(start + segment);
            if (delay > 0) {
                sleep_ms(delay);
            }
        }
    } else {
        uint8_t remaining = width;
        while (remaining > 0) {
            uint8_t segment = std::min<uint8_t>(chunk_width, remaining);
            uint8_t start = static_cast<uint8_t>(remaining - segment);
            update_region(start, 0, segment, height);
            remaining = static_cast<uint8_t>(remaining - segment);
            if (delay > 0) {
                sleep_ms(delay);
            }
        }
    }
}
