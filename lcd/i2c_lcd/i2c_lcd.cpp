#include "i2c_lcd.hpp"

#define SLEEP_TIME_MS (5)

/*#define LCD_PRINT_DEBUG*/

I2CLCD::I2CLCD(uint8_t num_rows, uint8_t num_cols) : LCD(num_rows, num_cols), i2c(I2C()), is_backlight(true) {
  i2c.write_blocking(LCD_I2C_ADDRESS, 0x00, 1, false);
  write_nibble(LCD_FUNCTION_RESET);
  write_nibble(LCD_FUNCTION);
  display_off();
  clear();

  write_command(LCD_ENTRY_MODE | LCD_ENTRY_INC);
  int wait_time = millis();
  delay_ms(SLEEP_TIME_MS);
  hide_cursor();
  display_on();

  uint8_t cmd = LCD_FUNCTION;
  num_rows > 1 ? cmd |= LCD_FUNCTION_2LINES : cmd = cmd;
  write_command(cmd);
}

void I2CLCD::write_nibble(uint8_t nibble) {
  uint8_t byte = ((nibble >> 4) & 0x0f) << SHIFT_DATA;
  uint8_t b_mask = byte | MASK_E;
#if defined (LCD_PRINT_DEBUG)
  printf("writing nibble: 0x%X to 0x%X\n", byte, b_mask);
#endif
  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &b_mask, 1, false);
  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &byte, 1, false);
  
  return;
}

void I2CLCD::write_command(uint8_t cmd) {
  uint8_t b = ((is_backlight << SHIFT_BACKLIGHT) | (((cmd >> 4) & 0x0f) << SHIFT_DATA));
  uint8_t b_mask = b | MASK_E;

#if defined (LCD_PRINT_DEBUG)
  printf("writing command: 0x%X to 0x%X\n", b, b_mask);
#endif

  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &b_mask, 1, false);
  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &b, 1, false);

  b = ((is_backlight << SHIFT_BACKLIGHT) | ((cmd & 0x0f) << SHIFT_DATA));
  b_mask = b | MASK_E;

#if defined (LCD_PRINT_DEBUG)
  printf("writing command: 0x%X to 0x%X\n", b, b_mask);
#endif

  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &b_mask, 1, false);
  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &b, 1, false);

  //The home and clear commands require a worst case delay of 4.1 milliseconds.
  if (cmd <= 3) {
    delay_ms(SLEEP_TIME_MS);
  }
  return;
}

void I2CLCD::write_data(uint8_t data) {
  uint8_t b = (MASK_RS | (is_backlight << SHIFT_BACKLIGHT) | (((data >> 4) & 0x0f) << SHIFT_DATA));
  uint8_t b_mask = b | MASK_E;

#if defined (LCD_PRINT_DEBUG)
  printf("writing data: 0x%X to 0x%X\n", b, b_mask);
#endif

  i2c.write_blocking(LCD_I2C_ADDRESS, &b_mask, 1, false);
  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &b, 1, false);
  delay_ms(SLEEP_TIME_MS);

  b = (MASK_RS | (is_backlight << SHIFT_BACKLIGHT) | ((data & 0x0f) << SHIFT_DATA));
  b_mask = b | MASK_E;

#if defined (LCD_PRINT_DEBUG)
  printf("writing data: 0x%X to 0x%X\n", b, b_mask);
#endif

  i2c.write_blocking(LCD_I2C_ADDRESS, &b_mask, 1, false);
  delay_ms(SLEEP_TIME_MS);
  i2c.write_blocking(LCD_I2C_ADDRESS, &b, 1, false);
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

