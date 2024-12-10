#include "i2c_lcd.hpp"


I2CLCD::I2CLCD(I2C* i2c, uint8_t addr, uint8_t num_rows, uint8_t num_cols) :
  i2c(i2c),
  addr(addr),
  num_rows(num_rows),
  num_cols(num_cols),
  cursor_x(0),
  cursor_y(0),
  nl_reached(false) {
  i2c->write_blocking(addr, 0x00, 1, false);

  uint32_t wait = millis();
  while (millis() - wait < 20);

  write_nibble(LCD_FUNCTION_RESET);

  wait = millis();
  while (millis() - wait < 5);

  //NOTE: Not sure why this is necessary, but it is.
  write_nibble(LCD_FUNCTION_RESET);
  write_nibble(LCD_FUNCTION_RESET);
  write_nibble(LCD_FUNCTION);

  uint8_t cmd = LCD_FUNCTION;
  num_rows > 1 ? cmd |= LCD_FUNCTION_2LINES : cmd = cmd;
  write_command(cmd);
}

void I2CLCD::write_nibble(uint8_t nibble) {
  uint8_t byte = ((nibble >> 4) & 0x0f) << SHIFT_DATA;
  uint8_t b_mask = byte | MASK_E;
  i2c->write_blocking(addr, &b_mask, 1, false);
  i2c->write_blocking(addr, &byte, 1, false);
  return;
}

void I2CLCD::write_command(uint8_t cmd) {
  uint8_t b = ((is_backlight << SHIFT_BACKLIGHT) | (((cmd >> 4) & 0x0f) << SHIFT_DATA));
  uint8_t b_mask = b | MASK_E;
  i2c->write_blocking(addr, &b_mask, 1, false);
  i2c->write_blocking(addr, &b, 1, false);

  b = ((is_backlight << SHIFT_BACKLIGHT) | ((cmd & 0x0f) << SHIFT_DATA));
  b_mask = b | MASK_E;
  i2c->write_blocking(addr, &b_mask, 1, false);
  i2c->write_blocking(addr, &b, 1, false);

  //The home and clear commands require a worst case delay of 4.1 milliseconds.
  if (cmd <= 3) {
    uint32_t wait = millis();
    while (millis() - wait < 5);
  }
}

void I2CLCD::write_data(uint8_t data) {
  uint8_t b = (MASK_RS | (is_backlight << SHIFT_BACKLIGHT) | (((data >> 4) & 0x0f) << SHIFT_DATA));
  uint8_t b_mask = b | MASK_E;
  i2c->write_blocking(addr, &b_mask, 1, false);
  i2c->write_blocking(addr, &b, 1, false);

  b = (MASK_RS | (is_backlight << SHIFT_BACKLIGHT) | ((data & 0x0f) << SHIFT_DATA));
  b_mask = b | MASK_E;
  i2c->write_blocking(addr, &b_mask, 1, false);
  i2c->write_blocking(addr, &b, 1, false);
  return;
}

void I2CLCD::backlight(bool on) {
  if (on) {
    uint8_t b = 1 << SHIFT_BACKLIGHT;
    i2c->write_blocking(addr, &b, 1, false);
    is_backlight = true;
    return;
  }
  i2c->write_blocking(addr, 0x00, 1, false);
  is_backlight = false;
  return;
}

