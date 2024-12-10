/**
 * @file
 * @brief 
 */

#include "HD44780.h"

class LCD {

public:
  virtual ~LCD() {}

  /**
   * @brief Writes a command to the LCD. Child classes must implement this
   * method. Commands are single bytes.
   *
   * @param cmd 
   */
  virtual void write_command(uint8_t cmd) = 0;
  /**
   * @brief Write data, such as character strings, to the LCD. Child classes
   * must implement this method.
   *
   * @param data
   */
  virtual void write_data(uint8_t data) = 0;

  virtual void clear() { 
    write_command(LCD_CLR);
    write_command(LCD_HOME);
    cursor_x = 0;
    cursor_y = 0;
    return;
  }

  virtual void show_cursor() {
    write_command(LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR);
    return;
  }

  virtual void hide_cursor() { 
    write_command(LCD_ON_CTRL | LCD_ON_DISPLAY);
    return;
  }

  virtual void blink_cursor_on() {
    write_command(LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR | LCD_ON_BLINK);
    return;
  }

  virtual void blink_cursor_off() {
    write_command(LCD_ON_CTRL | LCD_ON_DISPLAY | LCD_ON_CURSOR);
    return;
  }

  virtual void display_on() {
    write_command(LCD_ON_CTRL | LCD_ON_DISPLAY);
    return;
  }

  virtual void display_off() {
    write_command(LCD_ON_CTRL);
    return;
  }

  /**
   * @brief Some LCD modules have backlight controls. Child classes may
   * implement this functionality if desired.
   *
   * @param on true for on, false for off.
   */
  virtual void backlight(bool on) { return; }

  virtual void move_cursor_to(uint8_t x_pos, uint8_t y_pos) {
    cursor_x = x_pos;
    cursor_y = y_pos;
    uint8_t position = cursor_x & 0x3f;

    if (cursor_y & 1) {
      position += 0x40;
    }
    if (cursor_y & 2) {
      position += num_cols;
    }
    write_command(LCD_DDRAM | position);
    return;
  }

  virtual void put_char(uint8_t c) {

    if (c == '\n') {
      nl_reached ? nl_reached = false : cursor_x = num_cols;
    } else {
      write_data(c);
      cursor_x = (cursor_x + 1) % num_cols;
      if (cursor_x == 0) {
        nl_reached = true;
        cursor_y = (cursor_y + 1) % num_rows;
      }
      move_cursor_to(cursor_x, cursor_y);
    }
  }

  /**
   * @brief Writes str to the LCD at the current cursor position, advancing as needed.
   * WARN: This method does NOT perform buffer length checks.
   *
   * @param str 
   */
  virtual void put_str(uint8_t* str) {
    while (str != nullptr) {
      put_char(*str);
      str++;
    }
  }

  virtual void put_custom_char(uint8_t location, uint8_t* char_map) {
    location &= 0x07;
    write_command(LCD_CGRAM | (location << 3));
    asm volatile ("nop \n nop \n nop \n"); //small delay before writing data

    if (sizeof(char_map) < 8) {
      unsigned char err[] = "Error displaying custom char\n";
      put_str(err);
      return;
    }
    
    for (int i = 0; i < 8; ++i) {
      write_data(char_map[i]);
      asm volatile ("nop \n nop \n nop \n"); //small delay before writing data
    }
    move_cursor_to(cursor_x, cursor_y);
  }

  uint8_t num_rows;
  uint8_t num_cols;
  uint8_t cursor_x;
  uint8_t cursor_y;
  bool nl_reached; //used to determine wraparound point.

};
