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

#include <pico_drivers/display/ssd1306/ssd1306.hpp>
#include <hardware/dma.h>
#include <hardware/i2c.h>
#include <hardware/regs/i2c.h>
#include <pico/assert.h>
#include <pico/platform.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <pico_drivers/display/ssd1306/font/ssd1306_font.hpp>
#include <pico_drivers/display/ssd1306/font/dialog_bold_16.hpp>

/* Constructors */
OLED::OLED(uint8_t height, uint8_t width, bool reversed)
    : i2c(I2C()),
      height(height),
      width(width),
      reversed(reversed),
      pages(height / 8),
      buff_size(width * pages),
      my_font(&Dialog_bold_16) {
  hard_assert(buff_size <= MAX_BUFFER_SIZE);
  enable_double_buffer(true);
  init();
  clear_buffer();
  show();
}

OLED::OLED(I2C i2c, uint8_t height, uint8_t width, bool reversed)
    : i2c(i2c),
      height(height),
      width(width),
      reversed(reversed),
      pages(height / 8),
      buff_size(width * pages),
      my_font(&Dialog_bold_16) {
  hard_assert(buff_size <= MAX_BUFFER_SIZE);
  enable_double_buffer(true);
  init();
  clear_buffer();
  show();
}

OLED::~OLED() { deinit_dma(); }

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
    write_cmd(0x12);  // TODO: may want to change to switch case
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

void OLED::set_rotation(Rotation desired_rotation) {
  // Temporarily disable the panel while scan direction is updated.
  write_cmd(SET_DISP | 0x00);

  rotation = desired_rotation;
  switch (rotation) {
  case Rotation::Deg0:
    write_cmd(SET_SEG_REMAP | 0x01);    // column 0 maps to SEG0
    write_cmd(SET_COM_OUT_DIR_NORMAL);  // scan from COM[N-1] to COM0
    reversed = false;
    break;
  case Rotation::Deg180:
    write_cmd(SET_SEG_REMAP);            // column 127 maps to SEG0
    write_cmd(SET_COM_OUT_DIR_REVERSE);  // scan from COM0 to COM[N-1]
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
  if (enable == double_buffer_enabled) {
    return;
  }

  if (enable) {
    draw_buffer_index = 1;
    display_buffer_index = 0;
    std::memcpy(buffers[draw_buffer_index], buffers[display_buffer_index], buff_size);
    double_buffer_enabled = true;
  } else {
    double_buffer_enabled = false;
    draw_buffer_index = 0;
    display_buffer_index = 0;
  }
}

void OLED::swap_buffers() {
  if (!double_buffer_enabled) {
    return;
  }

  int tmp = draw_buffer_index;
  draw_buffer_index = display_buffer_index;
  display_buffer_index = tmp;
}

uint8_t *OLED::back_buffer() { return buffers[draw_buffer_index]; }

const uint8_t *OLED::front_buffer() const { return buffers[display_buffer_index]; }

void OLED::set_cursor(uint8_t x, uint8_t y) {
  cursor_x = x;
  cursor_y = y;
}

void OLED::set_text_wrap(bool wrap) { text_wrap = wrap; }

void OLED::write_char(uint8_t character) {
  if (character == '\n') {
    cursor_x = 0;
    cursor_y += my_font->y_advance;
    if (text_wrap && cursor_y + my_font->y_advance > height) {
      cursor_y = 0;
    }
    return;
  }

  if (character == '\r') {
    return;
  }

  if (character < my_font->first_char || character > my_font->last_char) {
    return;
  }

  GFXglyph *glyph = my_font->glyph + character - my_font->first_char;
  if (text_wrap && (cursor_x + glyph->width + glyph->x_offset > width)) {
    cursor_x = 0;
    cursor_y += my_font->y_advance;
    if (cursor_y + my_font->y_advance > height) {
      cursor_y = 0;
    }
  }

  if (text_wrap && cursor_y + my_font->y_advance > height) {
    cursor_y = 0;
  }

  print_char(cursor_x, cursor_y, character);
  cursor_x += glyph->x_advance;
}

void OLED::write(const char *str) {
  if (str == nullptr) return;
  while (*str) {
    write_char(static_cast<uint8_t>(*str++));
  }
}

void OLED::println(const char *str) {
  write(str);
  write_char('\n');
}

void OLED::println() { write_char('\n'); }

void OLED::printf(const char *fmt, ...) {
  if (fmt == nullptr) return;

  char formatted[128];

  va_list args;
  va_start(args, fmt);
  int written = vsnprintf(formatted, sizeof(formatted), fmt, args);
  va_end(args);

  if (written < 0) return;
  if (written >= static_cast<int>(sizeof(formatted))) {
    formatted[sizeof(formatted) - 1] = '\0';
  }

  write(formatted);
}

void OLED::is_inverse(bool inverse) { write_cmd(SET_NORM_INV | inverse); }

void OLED::clear_buffer() {
  std::memset(buffers[draw_buffer_index], 0, buff_size);
  if (!double_buffer_enabled) {
    std::memset(buffers[display_buffer_index], 0, buff_size);
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

  if (double_buffer_enabled) {
    swap_buffers();
  }

  const uint8_t *frame = front_buffer();
  start_dma_transfer(frame, buff_size);

  if (double_buffer_enabled) {
    std::memcpy(buffers[draw_buffer_index], buffers[display_buffer_index], buff_size);
  }
}

void OLED::draw_pixel(uint8_t x, uint8_t y) {
  if (x < width && y < height) {
    buffers[draw_buffer_index][x + width * (y / 8)] |= 0x01 << (y % 8);
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

  if (double_buffer_enabled) {
    for (uint8_t page = start_page; page <= end_page; ++page) {
      uint16_t offset = (page * width) + x;
      std::memcpy(buffers[display_buffer_index] + offset,
                  buffers[draw_buffer_index] + offset,
                  region_width);
    }
  }

  const uint8_t *frame = front_buffer();

  for (uint8_t page = start_page; page <= end_page; ++page) {
    write_cmd(SET_COL_ADDR);
    write_cmd(x);
    write_cmd(x + region_width - 1);
    write_cmd(SET_PAGE_ADDR);
    write_cmd(page);
    write_cmd(page);

    uint16_t offset = (page * width) + x;
    start_dma_transfer(frame + offset, region_width);
  }
}

void OLED::draw_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  draw_fast_hline(x, y, width);
  draw_fast_hline(x, y + height - 1, width);
  draw_fast_vline(x, y, height);
  draw_fast_vline(x + width - 1, y, height);
}

void OLED::draw_filled_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  for (uint8_t i = 0; i < height; ++i) {
    draw_fast_hline(x, y + i, width);
  }
}

void OLED::set_vertical_scroll_area(uint8_t top_fixed_rows, uint8_t scroll_rows) {
  if (top_fixed_rows + scroll_rows > height) {
    scroll_rows = (height > top_fixed_rows) ? (height - top_fixed_rows) : 0;
  }

  write_cmd(SET_VERTICAL_SCROLL_AREA);
  write_cmd(top_fixed_rows);
  write_cmd(scroll_rows);
}

void OLED::start_horizontal_scroll(ScrollDirection direction, uint8_t start_page,
                                   uint8_t end_page, uint8_t frame_interval) {
  if (start_page > end_page) {
    swap(&start_page, &end_page);
  }

  stop_scroll();

  uint8_t command = (direction == ScrollDirection::Right) ? RIGHT_HORIZONTAL_SCROLL
                                                          : LEFT_HORIZONTAL_SCROLL;
  write_cmd(command);
  write_cmd(0x00);                   // dummy
  write_cmd(start_page & 0x07);      // start page address
  write_cmd(frame_interval & 0x07);  // time interval
  write_cmd(end_page & 0x07);        // end page address
  write_cmd(0x00);                   // dummy
  write_cmd(0xFF);                   // dummy
  write_cmd(SET_SCROLL | 0x01);      // activate scroll
}

void OLED::start_diagonal_scroll(ScrollDirection direction, uint8_t start_page,
                                 uint8_t end_page, uint8_t frame_interval,
                                 uint8_t vertical_offset) {
  if (start_page > end_page) {
    swap(&start_page, &end_page);
  }

  stop_scroll();

  uint8_t command = (direction == ScrollDirection::Right) ? VERTICAL_RIGHT_SCROLL
                                                          : VERTICAL_LEFT_SCROLL;
  write_cmd(command);
  write_cmd(0x00);                    // dummy
  write_cmd(start_page & 0x07);       // start page address
  write_cmd(frame_interval & 0x07);   // time interval
  write_cmd(end_page & 0x07);         // end page address
  write_cmd(vertical_offset & 0x3F);  // vertical scroll offset (0-63)
  write_cmd(SET_SCROLL | 0x01);       // activate scroll
}

void OLED::stop_scroll() { write_cmd(SET_SCROLL | 0x00); }

void OLED::set_font(const GFXfont *font) { my_font = font; }

void OLED::print_char(uint8_t x, uint8_t y, uint8_t character) {
  if (character < my_font->first_char || character > my_font->last_char) return;

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
  if (str == nullptr) return;

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

void OLED::printf(uint8_t x, uint8_t y, const char *fmt, ...) {
  if (fmt == nullptr) return;

  char formatted[128];

  va_list args;
  va_start(args, fmt);
  int written = vsnprintf(formatted, sizeof(formatted), fmt, args);
  va_end(args);

  if (written < 0) return;
  if (written >= static_cast<int>(sizeof(formatted))) {
    formatted[sizeof(formatted) - 1] = '\0';
  }

  print(x, y, reinterpret_cast<uint8_t *>(formatted));
}

void OLED::init_dma() {
  if (dma_initialized) {
    return;
  }

  dma_control_channel = dma_claim_unused_channel(true);
  dma_data_channel = dma_claim_unused_channel(true);
  dma_initialized = true;
}

void OLED::deinit_dma() {
  if (!dma_initialized) {
    return;
  }

  if (dma_control_channel >= 0) {
    dma_channel_unclaim(dma_control_channel);
  }
  if (dma_data_channel >= 0) {
    dma_channel_unclaim(dma_data_channel);
  }
  dma_control_channel = -1;
  dma_data_channel = -1;
  dma_initialized = false;
}

void OLED::start_dma_transfer(const uint8_t *frame, uint16_t length) {
  if (frame == nullptr || length == 0) {
    return;
  }

  if (!dma_initialized) {
    init_dma();
  }

  hard_assert(length <= MAX_BUFFER_SIZE);

  i2c_inst_t *inst = i2c.handle();
  i2c_hw_t *hw = i2c_get_hw(inst);

  // Ensure previous activity has completed.
  while (hw->status & I2C_IC_STATUS_MST_ACTIVITY_BITS) {
    tight_loop_contents();
  }

  // Clear abort flags and target the display address.
  (void)hw->clr_tx_abrt;
  hw->tar = OLED_ADDRESS;

  dma_control_word = I2C_IC_DATA_CMD_RESTART_BITS | 0x40;

  for (uint16_t i = 0; i < length; ++i) {
    uint16_t word = frame[i];
    if (i == length - 1) {
      word |= I2C_IC_DATA_CMD_STOP_BITS;
    }
    dma_frame_buffer[i] = word;
  }

  dma_channel_config data_cfg = dma_channel_get_default_config(dma_data_channel);
  channel_config_set_transfer_data_size(&data_cfg, DMA_SIZE_16);
  channel_config_set_read_increment(&data_cfg, true);
  channel_config_set_write_increment(&data_cfg, false);
  channel_config_set_dreq(&data_cfg, i2c_get_dreq(inst, true));
  dma_channel_configure(dma_data_channel, &data_cfg, &hw->data_cmd, dma_frame_buffer,
                        length, false);

  dma_channel_config ctrl_cfg = dma_channel_get_default_config(dma_control_channel);
  channel_config_set_transfer_data_size(&ctrl_cfg, DMA_SIZE_16);
  channel_config_set_read_increment(&ctrl_cfg, false);
  channel_config_set_write_increment(&ctrl_cfg, false);
  channel_config_set_dreq(&ctrl_cfg, i2c_get_dreq(inst, true));
  channel_config_set_chain_to(&ctrl_cfg, dma_data_channel);
  dma_channel_configure(dma_control_channel, &ctrl_cfg, &hw->data_cmd, &dma_control_word,
                        1, false);

  dma_start_channel_mask(1u << dma_control_channel);

  dma_channel_wait_for_finish_blocking(dma_data_channel);
  dma_channel_wait_for_finish_blocking(dma_control_channel);

  while (hw->status & I2C_IC_STATUS_MST_ACTIVITY_BITS) {
    tight_loop_contents();
  }
  (void)hw->clr_stop_det;
}

void OLED::draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                       const uint8_t *img) {
  for (uint8_t i = 0; i < height; ++i) {
    for (uint8_t j = 0; j < width; ++j) {
      bool value = bit_read(img[i * ((width - 1) / 8 + 1) + j / 8], 7 - j % 8);
      if (value) draw_pixel(x + j, y + i);
    }
  }
}
