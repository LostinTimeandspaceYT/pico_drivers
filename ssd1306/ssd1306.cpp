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
 */

#include "ssd1306.hpp"
#include "font/dialog_bold_16.hpp"
#include "font/ssd1306_font.hpp"

/* Constructors */
OLED::OLED(uint8_t height, uint8_t width, bool reversed)
    : i2c(I2C()), height(height), width(width), reversed(reversed),
      pages(height / 8), buff_size(width * pages), my_font(&Dialog_bold_16) {
  init();
  clear_buffer();
  show();
}

OLED::OLED(I2C i2c, uint8_t height, uint8_t width, bool reversed)
    : i2c(i2c), height(height), width(width), reversed(reversed),
      pages(height / 8), buff_size(width * pages), my_font(&Dialog_bold_16) {
  init();
  clear_buffer();
  show();
}

OLED::~OLED() {}

/* Private Methods */
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

bool OLED::bit_read(uint8_t character, uint8_t index) {
  return bool((character >> index) & 0x01);
}

void OLED::is_display(bool display) { write_cmd(SET_DISP | display); }

void OLED::set_contrast(uint8_t contrast) {
  write_cmd(SET_CONTRAST);
  write_cmd(contrast);
}

void OLED::is_inverse(bool inverse) { write_cmd(SET_NORM_INV | inverse); }

void OLED::clear_buffer() {
  for (uint16_t i = 0; i < buff_size; ++i) {
    buffer[i] = 0x0000;
  }
}

void OLED::show() {
  /* Set col, row, and page address for sending buffer */
  write_cmd(SET_COL_ADDR);
  write_cmd(0);
  write_cmd(width - 1);
  write_cmd(SET_PAGE_ADDR);
  write_cmd(0);
  write_cmd(pages - 1);

  for (uint16_t i = 0; i < buff_size; ++i) {
    write_data(buffer[i]);
  }
}

void OLED::draw_pixel(uint8_t x, uint8_t y) {
  if (x < width && y < height) {
    buffer[x + width * (y / 8)] |= 0x01 << (y % 8);
  }
}

void OLED::draw_fast_hline(uint8_t x, uint8_t y, uint8_t width) {
  for (uint8_t i = 0; i < width; ++i) {
    draw_pixel(x + i, y);
  }
}

void OLED::draw_fast_vline(uint8_t x, uint8_t y, uint8_t height) {
  for (uint8_t i = 0; i < height; ++i) {
    draw_pixel(x, y + i);
  }
}

void OLED::draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  if (x1 > x2) {
    swap(&x1, &x2);
    swap(&y1, &y2);
  }
  float slope = (float)(y2 - y1) / (float)(x2 - x1);
  for (uint8_t x = x1; x <= x2; ++x) {
    float y = slope * (float)(x - x1) + (float)y1;
    draw_pixel(x, y);
  }
}

void OLED::draw_circle(int16_t xc, int16_t yc, uint16_t radius) {
  int16_t x = -radius;
  int16_t y = 0;
  int16_t e = 2 - (2 * radius);
  do {
    draw_pixel(xc + x, yc - y);
    draw_pixel(xc - x, yc + y);
    draw_pixel(xc + y, yc + x);
    draw_pixel(xc - y, yc - x);
    int16_t tmp = e;
    if (tmp <= y) {
      e += (++y * 2) + 1;
    }
    if ((tmp > x) || (e > y)) {
      e += (++y * 2) + 1;
    }
  } while (x < 0);
}

void OLED::draw_filled_circle(int16_t xc, int16_t yc, uint16_t radius) {
  int16_t x = radius;
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

void OLED::draw_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  draw_fast_hline(x, y, width);
  draw_fast_hline(x, y + height - 1, width);
  draw_fast_vline(x, y, height);
  draw_fast_vline(x + width - 1, y, height);
}

void OLED::draw_filled_rectangle(uint8_t x, uint8_t y, uint8_t width,
                                 uint8_t height) {
  for (uint8_t i = 0; i < height; ++i) {
    draw_fast_hline(x, y + i, width);
  }
}

void OLED::set_scroll_direction(bool direction) {
  write_cmd(SET_HOR_SCROLL | direction);
  write_cmd(0x00);
  write_cmd(0);
  write_cmd(0x06);
  write_cmd(pages - 1);
  write_cmd(0x00);
  write_cmd(0xff);
}

void OLED::is_scroll(bool is_enable) { write_cmd(SET_SCROLL | is_enable); }

void OLED::set_font(const GFXfont *font) { my_font = font; }

void OLED::print_char(uint8_t x, uint8_t y, uint8_t character) {
  if (character < my_font->first_char || character > my_font->last_char)
    return;

  character -= my_font->first_char;
  GFXglyph *glyph = my_font->glyph + character;
  uint8_t *bitmap = my_font->bitmap;

  uint16_t bitmap_offset = glyph->bitmap_offset;
  uint8_t width = glyph->width;
  uint8_t height = glyph->height;
  int8_t x_offset = glyph->x_offset;
  uint8_t y_offset = my_font->y_advance + glyph->y_offset;
  uint8_t bits = 0;
  uint8_t a_bit = 0;

  for (uint8_t i = 0; i < height; ++i) {
    for (uint8_t j = 0; j < width; ++j) {
      if (!(a_bit++ & 7)) {
        bits = bitmap[bitmap_offset++];
      }
      if (bits & 0x80) {
        draw_pixel(x + x_offset + j, y + y_offset + i);
      }
      bits <<= 1;
    }
  }
}

void OLED::print(uint8_t x, uint8_t y, uint8_t *str) {
  if (str == nullptr)
    return;

  uint8_t i = 0;
  while (str[i] != '\0') {
    uint8_t character = str[i];
    GFXglyph *glyph = my_font->glyph + character - my_font->first_char;
    if (x + glyph->width + glyph->x_offset > width) {
      x = 0;
      y += my_font->y_advance;
    }
    print_char(x, y, character);
    x += glyph->x_advance;
    ++i;
  }
}

void OLED::draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                       const uint8_t *img) {
  for (uint8_t i = 0; i < height; ++i) {
    for (uint8_t j = 0; j < width; ++j) {
      bool value = bit_read(img[i * ((width - 1) / 8 + 1) + j / 8], 7 - j % 8);
      if (value)
        draw_pixel(x + j, y + i);
    }
  }
}
