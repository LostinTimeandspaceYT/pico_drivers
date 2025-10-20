#ifndef PICO_DRIVERS_DISPLAY_SHAPE_RASTERIZER_HPP_
#define PICO_DRIVERS_DISPLAY_SHAPE_RASTERIZER_HPP_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <utility>

class ShapeRasterizer {
  public:
    using PixelWriter = std::function<void(int16_t, int16_t)>;
    using SpanWriter = std::function<void(int16_t, int16_t, uint16_t)>;

    ShapeRasterizer(uint16_t width, uint16_t height, PixelWriter writer,
                    SpanWriter span_writer = {})
        : width_(width), height_(height), pixel_writer_(std::move(writer)),
          span_writer_(std::move(span_writer)) {}

    void set_bounds(uint16_t width, uint16_t height) {
        width_ = width;
        height_ = height;
    }

    void set_pixel_writer(PixelWriter writer) { pixel_writer_ = std::move(writer); }
    void set_span_writer(SpanWriter writer) { span_writer_ = std::move(writer); }

    void draw_fast_hline(int16_t x, int16_t y, uint16_t width) const {
        if (width == 0) {
            return;
        }
        if (span_writer_) {
            span_writer_(x, y, width);
            return;
        }
        for (int16_t i = 0; i < static_cast<int16_t>(width); ++i) {
            set_pixel(x + i, y);
        }
    }

    void draw_fast_vline(int16_t x, int16_t y, uint16_t height) const {
        for (int16_t i = 0; i < static_cast<int16_t>(height); ++i) {
            set_pixel(x, y + i);
        }
    }

    void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1) const {
        int16_t dx = std::abs(x1 - x0);
        int16_t sx = (x0 < x1) ? 1 : -1;
        int16_t dy = -std::abs(y1 - y0);
        int16_t sy = (y0 < y1) ? 1 : -1;
        int16_t err = dx + dy;

        while (true) {
            set_pixel(x0, y0);
            if (x0 == x1 && y0 == y1) {
                break;
            }
            int16_t e2 = err * 2;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    void draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                       int16_t y2) const {
        draw_line(x0, y0, x1, y1);
        draw_line(x1, y1, x2, y2);
        draw_line(x2, y2, x0, y0);
    }

    void draw_filled_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                              int16_t y2) const {
        int16_t min_x = std::min(std::min(x0, x1), x2);
        int16_t max_x = std::max(std::max(x0, x1), x2);
        int16_t min_y = std::min(std::min(y0, y1), y2);
        int16_t max_y = std::max(std::max(y0, y1), y2);

        if (max_x < 0 || max_y < 0 || min_x >= static_cast<int16_t>(width_) ||
            min_y >= static_cast<int16_t>(height_)) {
            return;
        }

        min_x = std::max<int16_t>(0, min_x);
        min_y = std::max<int16_t>(0, min_y);
        max_x = std::min<int16_t>(static_cast<int16_t>(width_) - 1, max_x);
        max_y = std::min<int16_t>(static_cast<int16_t>(height_) - 1, max_y);

        int32_t area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
        if (area == 0) {
            draw_triangle(x0, y0, x1, y1, x2, y2);
            return;
        }

        for (int16_t y = min_y; y <= max_y; ++y) {
            for (int16_t x = min_x; x <= max_x; ++x) {
                int32_t w0 = (x1 - x0) * (y - y0) - (y1 - y0) * (x - x0);
                int32_t w1 = (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1);
                int32_t w2 = (x0 - x2) * (y - y2) - (y0 - y2) * (x - x2);
                bool has_neg = (w0 < 0) || (w1 < 0) || (w2 < 0);
                bool has_pos = (w0 > 0) || (w1 > 0) || (w2 > 0);
                if (!(has_neg && has_pos)) {
                    set_pixel(x, y);
                }
            }
        }
    }

    void draw_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height) const {
        draw_fast_hline(x, y, width);
        draw_fast_hline(x, y + static_cast<int16_t>(height) - 1, width);
        draw_fast_vline(x, y, height);
        draw_fast_vline(x + static_cast<int16_t>(width) - 1, y, height);
    }

    void draw_filled_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height) const {
        for (uint16_t row = 0; row < height; ++row) {
            draw_fast_hline(x, y + static_cast<int16_t>(row), width);
        }
    }

    void draw_circle(int16_t xc, int16_t yc, uint16_t radius) const {
        int16_t x = -static_cast<int16_t>(radius);
        int16_t y = 0;
        int16_t e = 2 - static_cast<int16_t>(radius * 2);
        do {
            set_pixel(xc + x, yc - y);
            set_pixel(xc - x, yc + y);
            set_pixel(xc + y, yc + x);
            set_pixel(xc - y, yc - x);
            int16_t tmp = e;
            if (tmp <= y) {
                e += (++y * 2) + 1;
            }
            if ((tmp > x) || (e > y)) {
                e += (++y * 2) + 1;
            }
        } while (x++ < 0);
    }

    void draw_filled_circle(int16_t xc, int16_t yc, uint16_t radius) const {
        int16_t x = static_cast<int16_t>(radius);
        int16_t y = 0;
        int16_t e = 1 - x;
        while (x >= y) {
            draw_line(xc + x, yc + y, xc - x, yc + y);
            draw_line(xc + y, yc + x, xc - y, yc + x);
            draw_line(xc - x, yc - y, xc + x, yc - y);
            draw_line(xc - y, yc - x, xc + y, yc - x);
            ++y;
            if (e >= 0) {
                --x;
                e += 2 * ((y - x) + 1);
            } else {
                e += (2 * y) + 1;
            }
        }
    }

    void draw_rounded_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height,
                                uint16_t radius) const {
        if (width == 0 || height == 0) {
            return;
        }

        uint16_t max_radius = std::min<uint16_t>(width / 2, height / 2);
        if (radius > max_radius) {
            radius = max_radius;
        }

        if (radius == 0) {
            draw_rectangle(x, y, width, height);
            return;
        }

        draw_fast_hline(x + static_cast<int16_t>(radius), y, width - 2 * radius);
        draw_fast_hline(x + static_cast<int16_t>(radius), y + static_cast<int16_t>(height) - 1,
                        width - 2 * radius);
        draw_fast_vline(x, y + static_cast<int16_t>(radius), height - 2 * radius);
        draw_fast_vline(x + static_cast<int16_t>(width) - 1, y + static_cast<int16_t>(radius),
                        height - 2 * radius);

        draw_circle_helper(x + static_cast<int16_t>(radius), y + static_cast<int16_t>(radius),
                           static_cast<int16_t>(radius), 0x1);
        draw_circle_helper(x + static_cast<int16_t>(width) - static_cast<int16_t>(radius) - 1,
                           y + static_cast<int16_t>(radius), static_cast<int16_t>(radius), 0x2);
        draw_circle_helper(x + static_cast<int16_t>(width) - static_cast<int16_t>(radius) - 1,
                           y + static_cast<int16_t>(height) - static_cast<int16_t>(radius) - 1,
                           static_cast<int16_t>(radius), 0x4);
        draw_circle_helper(x + static_cast<int16_t>(radius),
                           y + static_cast<int16_t>(height) - static_cast<int16_t>(radius) - 1,
                           static_cast<int16_t>(radius), 0x8);
    }

    void draw_filled_rounded_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height,
                                       uint16_t radius) const {
        if (width == 0 || height == 0) {
            return;
        }

        uint16_t max_radius = std::min<uint16_t>(width / 2, height / 2);
        if (radius > max_radius) {
            radius = max_radius;
        }

        if (radius == 0) {
            draw_filled_rectangle(x, y, width, height);
            return;
        }

        draw_filled_rectangle(x + static_cast<int16_t>(radius), y, width - 2 * radius, height);

        int16_t delta = static_cast<int16_t>(height) - 2 * static_cast<int16_t>(radius) - 1;
        fill_circle_helper(x + static_cast<int16_t>(width) - static_cast<int16_t>(radius) - 1,
                           y + static_cast<int16_t>(radius), static_cast<int16_t>(radius), 0x1,
                           delta);
        fill_circle_helper(x + static_cast<int16_t>(radius), y + static_cast<int16_t>(radius),
                           static_cast<int16_t>(radius), 0x2, delta);
    }

    void draw_quadratic_bezier(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                               int16_t y2) const {
        int32_t curvature_x = x0 - (2 * x1) + x2;
        if (curvature_x < 0) {
            curvature_x = -curvature_x;
        }
        int32_t curvature_y = y0 - (2 * y1) + y2;
        if (curvature_y < 0) {
            curvature_y = -curvature_y;
        }

        uint16_t steps = static_cast<uint16_t>(curvature_x + curvature_y);
        if (steps < 16) {
            steps = 16;
        }

        float step = 1.0f / static_cast<float>(steps);
        int16_t prev_x = x0;
        int16_t prev_y = y0;

        for (uint16_t i = 1; i <= steps; ++i) {
            float t = step * static_cast<float>(i);
            float inv = 1.0f - t;
            float xf = inv * inv * static_cast<float>(x0) +
                       2.0f * inv * t * static_cast<float>(x1) + t * t * static_cast<float>(x2);
            float yf = inv * inv * static_cast<float>(y0) +
                       2.0f * inv * t * static_cast<float>(y1) + t * t * static_cast<float>(y2);

            int16_t xi = static_cast<int16_t>(std::round(xf));
            int16_t yi = static_cast<int16_t>(std::round(yf));
            draw_line(prev_x, prev_y, xi, yi);
            prev_x = xi;
            prev_y = yi;
        }
    }

  private:
    uint16_t width_;
    uint16_t height_;
    PixelWriter pixel_writer_;
    SpanWriter span_writer_;

    void set_pixel(int16_t x, int16_t y) const {
        if (!pixel_writer_) {
            return;
        }
        if (x < 0 || y < 0 || x >= static_cast<int16_t>(width_) ||
            y >= static_cast<int16_t>(height_)) {
            return;
        }
        pixel_writer_(x, y);
    }

    void draw_vertical_span(int16_t x, int16_t y_start, int16_t length) const {
        if (length <= 0) {
            return;
        }
        if (x < 0 || x >= static_cast<int16_t>(width_)) {
            return;
        }

        int16_t y_end = y_start + length - 1;
        if (y_start > y_end) {
            std::swap(y_start, y_end);
        }
        if (y_end < 0 || y_start >= static_cast<int16_t>(height_)) {
            return;
        }

        if (y_start < 0) {
            y_start = 0;
        }
        if (y_end >= static_cast<int16_t>(height_)) {
            y_end = static_cast<int16_t>(height_) - 1;
        }

        for (int16_t y = y_start; y <= y_end; ++y) {
            set_pixel(x, y);
        }
    }

    void draw_circle_helper(int16_t x0, int16_t y0, int16_t r, uint8_t corner_mask) const {
        if (r == 0) {
            set_pixel(x0, y0);
            return;
        }

        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;

        while (x <= y) {
            if (corner_mask & 0x1) {
                set_pixel(x0 - y, y0 - x);
                set_pixel(x0 - x, y0 - y);
            }
            if (corner_mask & 0x2) {
                set_pixel(x0 + x, y0 - y);
                set_pixel(x0 + y, y0 - x);
            }
            if (corner_mask & 0x4) {
                set_pixel(x0 + x, y0 + y);
                set_pixel(x0 + y, y0 + x);
            }
            if (corner_mask & 0x8) {
                set_pixel(x0 - x, y0 + y);
                set_pixel(x0 - y, y0 + x);
            }
            if (f >= 0) {
                --y;
                ddF_y += 2;
                f += ddF_y;
            }
            ++x;
            ddF_x += 2;
            f += ddF_x;
        }
    }

    void fill_circle_helper(int16_t x0, int16_t y0, int16_t r, uint8_t corner_mask,
                            int16_t delta) const {
        if (r == 0) {
            draw_vertical_span(x0, y0, 1 + delta);
            return;
        }

        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;

        while (x <= y) {
            if (corner_mask & 0x1) {
                draw_vertical_span(x0 + x, y0 - y, 2 * y + 1 + delta);
                draw_vertical_span(x0 + y, y0 - x, 2 * x + 1 + delta);
            }
            if (corner_mask & 0x2) {
                draw_vertical_span(x0 - x, y0 - y, 2 * y + 1 + delta);
                draw_vertical_span(x0 - y, y0 - x, 2 * x + 1 + delta);
            }
            if (f >= 0) {
                --y;
                ddF_y += 2;
                f += ddF_y;
            }
            ++x;
            ddF_x += 2;
            f += ddF_x;
        }
    }
};

#endif // PICO_DRIVERS_DISPLAY_SHAPE_RASTERIZER_HPP_
