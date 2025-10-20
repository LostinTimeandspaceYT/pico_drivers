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

// TODO: Decide if I want to use a struct for passing coordinates

#ifndef _SSD1306_H
#define _SSD1306_H

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
  uint8_t cursor_x_pos() const { return cursor_x; }
  uint8_t cursor_y_pos() const { return cursor_y; }
  void set_text_wrap(bool wrap);
  void write_char(uint8_t character);
  void write(const char *str);
  void println(const char *str);
  void println();
  void printf(const char *fmt, ...);

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

  /**
   * @brief Like draw_circle but colors in the shape.
   *
   * @param xc x coordinate of circle's center
   * @param yc y coordinate of circle's center
   * @param radius length of radius in pixels
   */
  void draw_filled_circle(int16_t xc, int16_t yc, uint16_t radius);

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
  void start_horizontal_scroll(ScrollDirection direction,
                               uint8_t start_page,
                               uint8_t end_page,
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
  void start_diagonal_scroll(ScrollDirection direction,
                             uint8_t start_page,
                             uint8_t end_page,
                             uint8_t frame_interval,
                             uint8_t vertical_offset);

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
  void draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                   const uint8_t *img);

 private:
  I2C i2c;
  uint8_t width;
  uint8_t height;
  uint8_t pages;
  uint16_t buff_size;
  bool reversed;
  Rotation rotation = Rotation::Deg0;
  static constexpr uint16_t MAX_BUFFER_SIZE = 1024;
  uint8_t buffers[2][MAX_BUFFER_SIZE] = {};
  int draw_buffer_index = 0;
  int display_buffer_index = 0;
  bool double_buffer_enabled = false;
  uint8_t cursor_x = 0;
  uint8_t cursor_y = 0;
  bool text_wrap = true;
  uint16_t dma_frame_buffer[MAX_BUFFER_SIZE + 1] = {0};
  uint16_t dma_control_word = 0;
  int dma_control_channel = -1;
  int dma_data_channel = -1;
  bool dma_initialized = false;
  const GFXfont *my_font;

  void init(void);
  void write_cmd(uint8_t cmd);
  void write_data(uint8_t data);
  void swap(uint8_t *x1, uint8_t *x2);
  bool bit_read(uint8_t character, uint8_t index);
  void draw_pixel(uint8_t x, uint8_t y);
  void init_dma();
  void deinit_dma();
  void start_dma_transfer(const uint8_t *frame, uint16_t length);
};

#endif  // end _SSD1306_H
/* END OF FILE */
