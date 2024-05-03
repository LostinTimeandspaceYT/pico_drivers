/** @file common.h
 *
 * @brief module that defines params used during testing.
 *
 * @par
 * Credit:
 * https://github.com/pimoroni/pimoroni-pico/blob/main/common/pimoroni_common.hpp
 */

#ifndef _COMMON_H
#define _COMMON_H

#include "pico/stdlib.h"
#include <climits>
#include <cstdint>
#include <stdint.h>

/* Defines */
#define I2C_DEFAULT_INSTANCE (i2c0)
#define SPI_DEFAULT_INSTANCE (spi0)

static constexpr uint8_t PIN_UNUSED = 255;

/* Default I2C Params */
static constexpr uint32_t I2C_DEFAULT_BAUDRATE = 400000;
static constexpr uint32_t I2C_ALT_BAUDRATE = 100000;
static constexpr uint I2C_DEFAULT_SDA = PICO_DEFAULT_I2C_SDA_PIN;
static constexpr uint I2C_DEFAULT_SCL = PICO_DEFAULT_I2C_SCL_PIN;

/* Default SPI Params */
static constexpr uint32_t SPI_DEFAULT_BAUDRATE = 1000000; // 1MHz
static constexpr uint SPI_DEFAULT_MOSI = PICO_DEFAULT_SPI_RX_PIN;
static constexpr uint SPI_DEFAULT_MISO = PICO_DEFAULT_SPI_TX_PIN;
static constexpr uint SPI_DEFAULT_SCLK = PICO_DEFAULT_SPI_SCK_PIN;
static constexpr uint SPI_DEFAULT_CS = PICO_DEFAULT_SPI_CSN_PIN;

/* Based on the Arduino function */
inline uint32_t millis() { return to_ms_since_boot(get_absolute_time()); }

struct pin_pair {
  union {
    uint8_t first;
    uint8_t a;        // encoder
    uint8_t positive; // motor
    uint8_t forward;  // limit
    uint8_t phase;
  };
  union {
    uint8_t second;
    uint8_t b;        // encoder
    uint8_t negative; // motor
    uint8_t reverse;  // limit
    uint8_t enable;
  };

  constexpr pin_pair() : first(0), second(0) {}
  constexpr pin_pair(uint8_t first, uint8_t second)
      : first(first), second(second) {}
};

struct bool_pair {
  union {
    bool first;
    bool a;
  };
  union {
    bool second;
    bool b;
  };

  bool_pair() : first(false), second(false) {}
  bool_pair(bool first, bool second) : first(first), second(second) {}
};

struct register32_t {
  union {
    uint32_t value;
    uint8_t arr[4];
  };
};

struct register16_t {
  union {
    uint16_t value;
    uint8_t arr[2];
  };
};

#endif // end _COMMON_H
