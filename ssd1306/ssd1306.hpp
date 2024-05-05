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

#include "../i2c/i2c.hpp"
#include "bitmap.hpp"
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
static constexpr uint8_t SET_COM_OUT_DIR = 0xC8;
static constexpr uint8_t SET_DISP_OFFSET = 0xD3;
static constexpr uint8_t SET_COM_PIN_CFG = 0xDA;
static constexpr uint8_t SET_DISP_CLK_DIV = 0xD5;
static constexpr uint8_t SET_PRECHARGE = 0xD9;
static constexpr uint8_t SET_VCOM_DESEL = 0xDB;
static constexpr uint8_t SET_CHARGE_PUMP = 0x8D;
static constexpr uint8_t SET_SCROLL = 0x2E;
static constexpr uint8_t SET_HOR_SCROLL = 0x26;
static constexpr uint8_t SET_COM_OUT_DIR_REVERSE = 0xC0;

struct GFXglyph {
  uint16_t bitmap_offset; // Ptr into GFXfont->bitmap
  uint8_t width;          // Bitmap dimensions in pixels
  uint8_t height;
  uint8_t x_advance; // Distance to advance cursour (x-axis)
  int8_t x_offset;   // Distance from cursor position to Upper Left corner
  int8_t y_offset;
};

struct GFXfont {
  uint8_t *bitmap;    // Glyph bitmaps, concatenated
  GFXglyph *glyph;    // array of glyphs
  uint8_t first_char; // ASCII extents
  uint8_t last_char;  // ASCII extents
  uint8_t y_advance;  // distance to Newline
};

class OLED {

public:
  OLED(uint8_t height, uint8_t width, bool reversed);
  OLED(I2C i2c, uint8_t height, uint8_t width, bool reversed);
  ~OLED();

  void show();
  void clear_buffer();
  void is_display(bool inverse);
  void is_inverse(bool inverse);
  void set_contrast(uint8_t contrast);
  void set_font(const GFXfont *font);

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
  void draw_filled_rectangle(uint8_t x, uint8_t y, uint8_t width,
                             uint8_t height);

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
   * @brief Sets the direction graphics scroll from
   *
   * @param direction true for down, false for up
   */
  void set_scroll_direction(bool direction);

  void is_scroll(bool enabled);

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
  uint8_t buffer[1024] = {0}; // Ensure the buffer is clear.
  const GFXfont *my_font;

  void init(void);
  void write_cmd(uint8_t cmd);
  void write_data(uint8_t data);
  void swap(uint8_t *x1, uint8_t *x2);
  bool bit_read(uint8_t character, uint8_t index);
  void draw_pixel(uint8_t x, uint8_t y);
};

#endif // end _SSD1306_H
/* END OF FILE */
