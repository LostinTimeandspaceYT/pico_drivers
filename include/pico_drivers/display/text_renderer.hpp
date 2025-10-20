#ifndef PICO_DRIVERS_DISPLAY_TEXT_RENDERER_HPP_
#define PICO_DRIVERS_DISPLAY_TEXT_RENDERER_HPP_

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <utility>

#include <pico_drivers/display/gfx_font.hpp>

class TextRenderer {
  public:
    using PixelWriter = std::function<void(int16_t, int16_t, bool)>;

    TextRenderer(uint16_t width, uint16_t height, PixelWriter pixel_writer,
                 const GFXfont *font = nullptr)
        : pixel_writer_(std::move(pixel_writer)), width_(width), height_(height), font_(font) {}

    void set_bounds(uint16_t width, uint16_t height) {
        width_ = width;
        height_ = height;
    }

    void set_font(const GFXfont *font) { font_ = font; }
    const GFXfont *font() const { return font_; }

    void set_cursor(uint16_t x, uint16_t y) {
        cursor_x_ = x;
        cursor_y_ = y;
    }

    uint16_t cursor_x() const { return cursor_x_; }
    uint16_t cursor_y() const { return cursor_y_; }

    void set_text_wrap(bool wrap) { text_wrap_ = wrap; }
    bool text_wrap() const { return text_wrap_; }

    void write_char(uint8_t ch) {
        if (font_ == nullptr) {
            return;
        }

        if (ch == '\r') {
            return;
        }

        if (ch == '\n') {
            cursor_x_ = 0;
            cursor_y_ = advance_line(cursor_y_);
            return;
        }

        const GFXglyph *glyph = glyph_for(ch);
        if (glyph == nullptr) {
            return;
        }

        if (text_wrap_ && cursor_x_ != 0 && cursor_x_ + glyph->width + glyph->x_offset > width_) {
            cursor_x_ = 0;
            cursor_y_ = advance_line(cursor_y_);
        }

        if (text_wrap_ && cursor_y_ + font_->y_advance > height_) {
            cursor_y_ = 0;
        }

        render_glyph(cursor_x_, cursor_y_, ch);
        cursor_x_ = static_cast<uint16_t>(cursor_x_ + glyph->x_advance);
    }

    void write(const char *str) {
        if (str == nullptr) {
            return;
        }
        while (*str != '\0') {
            write_char(static_cast<uint8_t>(*str++));
        }
    }

    void println(const char *str) {
        write(str);
        write_char('\n');
    }

    void println() { write_char('\n'); }

    void printf(const char *fmt, ...) {
        if (fmt == nullptr) {
            return;
        }

        char buffer[128];
        va_list args;
        va_start(args, fmt);
        int written = vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        if (written < 0) {
            return;
        }
        if (written >= static_cast<int>(sizeof(buffer))) {
            buffer[sizeof(buffer) - 1] = '\0';
        }
        write(buffer);
    }

    void wrap_text(uint16_t x, uint16_t y, const char *str, uint16_t max_width) {
        if (font_ == nullptr || str == nullptr || width_ == 0 || height_ == 0) {
            return;
        }
        if (x >= width_ || y >= height_) {
            return;
        }

        uint16_t available_width = max_width;
        if (available_width == 0 || available_width > width_ - x) {
            available_width = width_ - x;
        }
        if (available_width == 0) {
            return;
        }

        const uint16_t line_start_x = x;
        uint16_t cursor_x_pos = line_start_x;
        uint16_t cursor_y_pos = y;
        const uint16_t max_line_x = static_cast<uint16_t>(line_start_x + available_width);

        const char *cursor = str;
        while (*cursor != '\0' && cursor_y_pos < height_) {
            char ch = *cursor;
            if (ch == '\r') {
                ++cursor;
                continue;
            }
            if (ch == '\n') {
                cursor_x_pos = line_start_x;
                uint16_t next_line = advance_line(cursor_y_pos);
                if (next_line >= height_) {
                    break;
                }
                cursor_y_pos = next_line;
                ++cursor;
                continue;
            }

            if (ch == ' ') {
                uint8_t advance = glyph_advance(static_cast<uint8_t>(' '));
                if (cursor_x_pos != line_start_x && cursor_x_pos + advance > max_line_x) {
                    uint16_t next_line = advance_line(cursor_y_pos);
                    if (next_line >= height_) {
                        break;
                    }
                    cursor_y_pos = next_line;
                    cursor_x_pos = line_start_x;
                } else {
                    cursor_x_pos = static_cast<uint16_t>(cursor_x_pos + advance);
                }
                ++cursor;
                continue;
            }

            const char *word_end = cursor;
            uint16_t word_width = 0;
            while (*word_end != '\0' && *word_end != ' ' && *word_end != '\n' &&
                   *word_end != '\r') {
                word_width = static_cast<uint16_t>(word_width +
                                                   glyph_advance(static_cast<uint8_t>(*word_end)));
                ++word_end;
            }

            if (cursor_x_pos != line_start_x && cursor_x_pos + word_width > max_line_x) {
                uint16_t next_line = advance_line(cursor_y_pos);
                if (next_line >= height_) {
                    break;
                }
                cursor_y_pos = next_line;
                cursor_x_pos = line_start_x;
            }

            const char *word_cursor = cursor;
            while (word_cursor < word_end && cursor_y_pos < height_) {
                uint8_t ch_val = static_cast<uint8_t>(*word_cursor);
                uint8_t advance = glyph_advance(ch_val);
                if (advance > available_width) {
                    if (cursor_x_pos != line_start_x) {
                        uint16_t next_line = advance_line(cursor_y_pos);
                        if (next_line >= height_) {
                            return;
                        }
                        cursor_y_pos = next_line;
                        cursor_x_pos = line_start_x;
                    }
                } else if (cursor_x_pos != line_start_x && cursor_x_pos + advance > max_line_x) {
                    uint16_t next_line = advance_line(cursor_y_pos);
                    if (next_line >= height_) {
                        return;
                    }
                    cursor_y_pos = next_line;
                    cursor_x_pos = line_start_x;
                }

                if (render_glyph(cursor_x_pos, cursor_y_pos, ch_val)) {
                    cursor_x_pos = static_cast<uint16_t>(cursor_x_pos + advance);
                }
                ++word_cursor;
            }
            cursor = word_end;
        }
    }

    void draw_char(uint16_t x, uint16_t y, uint8_t ch) { render_glyph(x, y, ch); }

    void draw_string(uint16_t x, uint16_t y, const char *str) {
        if (font_ == nullptr || str == nullptr) {
            return;
        }

        uint16_t cursor_x_pos = x;
        uint16_t cursor_y_pos = y;

        while (*str != '\0' && cursor_y_pos < height_) {
            uint8_t ch = static_cast<uint8_t>(*str++);
            if (ch == '\r') {
                continue;
            }
            if (ch == '\n') {
                cursor_x_pos = x;
                cursor_y_pos = advance_line(cursor_y_pos);
                continue;
            }

            const GFXglyph *glyph = glyph_for(ch);
            if (glyph == nullptr) {
                continue;
            }

            if (cursor_x_pos + glyph->width + glyph->x_offset > width_) {
                cursor_x_pos = x;
                cursor_y_pos = advance_line(cursor_y_pos);
            }

            render_glyph(cursor_x_pos, cursor_y_pos, ch);
            cursor_x_pos = static_cast<uint16_t>(cursor_x_pos + glyph->x_advance);
        }
    }

  private:
    PixelWriter pixel_writer_;
    uint16_t width_;
    uint16_t height_;
    uint16_t cursor_x_ = 0;
    uint16_t cursor_y_ = 0;
    bool text_wrap_ = true;
    const GFXfont *font_ = nullptr;

    uint16_t advance_line(uint16_t current_y) const {
        if (font_ == nullptr) {
            return current_y;
        }
        uint32_t next = static_cast<uint32_t>(current_y) + font_->y_advance;
        if (next >= height_) {
            return static_cast<uint16_t>(height_);
        }
        return static_cast<uint16_t>(next);
    }

    const GFXglyph *glyph_for(uint8_t ch) const {
        if (font_ == nullptr) {
            return nullptr;
        }
        if (ch < font_->first_char || ch > font_->last_char) {
            return nullptr;
        }
        return font_->glyph + (ch - font_->first_char);
    }

    uint8_t glyph_advance(uint8_t ch) const {
        const GFXglyph *glyph = glyph_for(ch);
        if (glyph != nullptr) {
            return glyph->x_advance;
        }
        if (font_ != nullptr && font_->y_advance != 0) {
            return static_cast<uint8_t>(font_->y_advance / 2);
        }
        return 0;
    }

    bool render_glyph(uint16_t x, uint16_t y, uint8_t ch) const {
        if (font_ == nullptr || !pixel_writer_) {
            return false;
        }

        const GFXglyph *glyph = glyph_for(ch);
        if (glyph == nullptr) {
            return false;
        }

        const uint8_t *bitmap = font_->bitmap;
        uint16_t offset = glyph->bitmap_offset;

        const int16_t base_x = static_cast<int16_t>(x) + glyph->x_offset;
        const int16_t base_y = static_cast<int16_t>(y) + font_->y_advance + glyph->y_offset;

        uint16_t bit_index = 0;
        uint8_t bits = 0;

        for (uint8_t row = 0; row < glyph->height; ++row) {
            for (uint8_t col = 0; col < glyph->width; ++col) {
                if ((bit_index & 7u) == 0) {
                    bits = bitmap[offset++];
                }
                if (bits & 0x80u) {
                    int16_t pixel_x = static_cast<int16_t>(base_x + col);
                    int16_t pixel_y = static_cast<int16_t>(base_y + row);
                    if (pixel_x >= 0 && pixel_y >= 0 && pixel_x < static_cast<int16_t>(width_) &&
                        pixel_y < static_cast<int16_t>(height_)) {
                        pixel_writer_(pixel_x, pixel_y, true);
                    }
                }
                bits <<= 1;
                ++bit_index;
            }
        }

        return true;
    }
};

#endif // PICO_DRIVERS_DISPLAY_TEXT_RENDERER_HPP_
