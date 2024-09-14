/** @file i2c.hpp
 *
 * @brief A wrapper for i2c operations.
 *
 * @par
 * This module is based on the works of Pimironi
 *
 * Source:
 * https://github.com/pimoroni/pimoroni-pico/blob/main/common/pimoroni_i2c.hpp
 */

#ifndef _I2C_H
#define _I2C_H

#include "../utils/common.hpp"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <climits>
#include <stdio.h>

class I2C {
private:
  i2c_inst_t *i2c = PICO_DEFAULT_I2C;
  uint sda = I2C_DEFAULT_SDA;
  uint scl = I2C_DEFAULT_SCL;
  uint interrupt = PIN_UNUSED;
  uint32_t baudrate = I2C_DEFAULT_BAUDRATE;

public:
  I2C(uint sda, uint scl, uint32_t baudrate = I2C_DEFAULT_BAUDRATE)
      : sda(sda), scl(scl), baudrate(baudrate) {
    init();
  }

  I2C() : I2C(I2C_DEFAULT_SDA, I2C_DEFAULT_SCL) {}

  ~I2C() {
    i2c_deinit(i2c);
    gpio_disable_pulls(sda);
    gpio_set_function(sda, GPIO_FUNC_NULL);
    gpio_disable_pulls(scl);
    gpio_set_function(scl, GPIO_FUNC_NULL);
  }

  i2c_inst_t *pin_to_inst(uint pin);
  int write_blocking(uint8_t addr, const uint8_t *src, size_t len, bool nostop);
  int read_blocking(uint8_t addr, uint8_t *dst, size_t len, bool nostop);

  void reg_write_uint8(uint8_t address, uint8_t reg, uint8_t value);
  uint8_t reg_read_uint8(uint8_t address, uint8_t reg);
  uint16_t reg_read_uint16(uint8_t address, uint8_t reg);
  int16_t reg_read_int16(uint8_t address, uint8_t reg);
  uint32_t reg_read_uint32(uint8_t address, uint8_t reg);

  int write_bytes(uint8_t address, uint8_t reg, const uint8_t *buf, int len);
  int read_bytes(uint8_t address, uint8_t reg, uint8_t *buf, int len);

  uint8_t get_bits(uint8_t address, uint8_t reg, uint8_t shift,
                   uint8_t mask = 0b1);
  void set_bits(uint8_t address, uint8_t reg, uint8_t shift,
                uint8_t mask = 0b1);
  void clear_bits(uint8_t address, uint8_t reg, uint8_t shift,
                  uint8_t mask = 0b1);

private:
  void init();
};

#endif // END _I2C_H

/* END OF FILE */
