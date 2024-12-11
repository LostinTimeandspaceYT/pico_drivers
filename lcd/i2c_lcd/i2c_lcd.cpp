#include "i2c_lcd.hpp"
#include <cstdio>

#define SLEEP_TIME_MS (5)


I2CLCD::I2CLCD(uint8_t num_rows, uint8_t num_cols) : LCD(num_rows, num_cols)
{
  I2C i2c = I2C();
  is_backlight = true;

  i2c.write_blocking(LCD_I2C_ADDRESS, 0x00, 1, false);

  //NOTE: Not sure why this is necessary, but it is.
  write_nibble(LCD_FUNCTION_RESET);
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
  printf("writing nibble: 0x%X to 0x%X\n", byte, b_mask);
  sleep_ms(SLEEP_TIME_MS);
  i2c.reg_write_uint8(LCD_I2C_ADDRESS, b_mask, byte);
  return;
}

void I2CLCD::write_command(uint8_t cmd) {
  uint8_t b = ((is_backlight << SHIFT_BACKLIGHT) | (((cmd >> 4) & 0x0f) << SHIFT_DATA));
  uint8_t b_mask = b | MASK_E;
  printf("writing command: 0x%X to 0x%X\n", b, b_mask);
  sleep_ms(SLEEP_TIME_MS);
  i2c.reg_write_uint8(LCD_I2C_ADDRESS, b_mask, b);

  b = ((is_backlight << SHIFT_BACKLIGHT) | ((cmd & 0x0f) << SHIFT_DATA));
  b_mask = b | MASK_E;
  printf("writing command: 0x%X to 0x%X\n", b, b_mask);
  sleep_ms(SLEEP_TIME_MS);
  i2c.reg_write_uint8(LCD_I2C_ADDRESS, b_mask, b);

  //The home and clear commands require a worst case delay of 4.1 milliseconds.
  if (cmd <= 3) {
    sleep_ms(SLEEP_TIME_MS);
    /*uint32_t wait = millis();*/
    /*while (millis() - wait < 5);*/
  }
}

void I2CLCD::write_data(uint8_t data) {
  uint8_t b = (MASK_RS | (is_backlight << SHIFT_BACKLIGHT) | (((data >> 4) & 0x0f) << SHIFT_DATA));
  uint8_t b_mask = b | MASK_E;
  printf("writing data: 0x%X to 0x%X\n", b, b_mask);
  sleep_ms(SLEEP_TIME_MS);
  i2c.reg_write_uint8(LCD_I2C_ADDRESS, b_mask, b);

  b = (MASK_RS | (is_backlight << SHIFT_BACKLIGHT) | ((data & 0x0f) << SHIFT_DATA));
  b_mask = b | MASK_E;
  printf("writing data: 0x%X to 0x%X\n", b, b_mask);
  sleep_ms(SLEEP_TIME_MS);
  i2c.reg_write_uint8(LCD_I2C_ADDRESS, b_mask, b);
  return;
}

void I2CLCD::backlight(bool on) {
  if (on) {
    uint8_t b = 1 << SHIFT_BACKLIGHT;
    i2c.write_blocking(LCD_I2C_ADDRESS, &b, 1, false);
    is_backlight = true;
    return;
  }
  i2c.write_blocking(LCD_I2C_ADDRESS, 0x00, 1, false);
  is_backlight = false;
  return;
}

