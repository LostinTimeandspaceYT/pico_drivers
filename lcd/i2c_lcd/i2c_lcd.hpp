#include "../../i2c/i2c.hpp"
#include "../lcd_api.hpp"

// PCF8574 pin definitions
#define MASK_RS  (0x01)  // P0
#define MASK_RW  (0x02)  // P1
#define MASK_E   (0x04)  // P2

#define SHIFT_BACKLIGHT  (3)  // P3
#define SHIFT_DATA       (4)  // P4-P7

#define LCD_I2C_ADDRESS (0x27)

class I2CLCD : public LCD {

public:
  I2CLCD(uint8_t num_rows, uint8_t num_cols);
  ~I2CLCD() {}

  /* Derived methods */
  void write_command(uint8_t cmd);
  void write_data(uint8_t data);
  void write_nibble(uint8_t nibble);
  void backlight(bool on);

private:
  I2C i2c;
  bool is_backlight;

public:
  /*uint8_t num_rows;*/
  /*uint8_t num_cols;*/
  /*uint8_t cursor_x;*/
  /*uint8_t cursor_y;*/
  /*bool nl_reached; //used to determine wraparound point.*/
};
