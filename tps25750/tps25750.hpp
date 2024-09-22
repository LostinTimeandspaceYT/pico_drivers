/** @file tps25750.hpp
 *
 * @brief this module defines an interface to the TPS25750 USB-C PD controller.
 *
 * @par
 * The TPS25750 is a highly integrated stand-alone USB Type-C and Power Delivery
 * (PD) controller optimized for applications supporting USB-C PD Power. The
 * TPS25750 integrates fully managed power paths with robust protection for a
 * complete USB-C PD solution. The TPS25750 also integrates control for external
 * battery charger ICs for added ease of use and reduced time to market.
 *
 * Link to datasheet:
 * https://www.ti.com/lit/ds/symlink/tps25750.pdf?ts=1714726260022&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTPS25750%253FkeyMatch%253DTPS25750%2526tisearch%253Dsearch-everything%2526usecase%253DGPN-ALT
 *
 * Link to Host Interface Technical Reference Manual:
 * https://www.ti.com/lit/ug/slvuc05a/slvuc05a.pdf?ts=1714750459366&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTPS25750
 */

/*
 * TODO:
 * 1. clearly define the I2C slave address setting.
 * 2. write out the register map.
 *
 *
 * */

#ifndef _TSP25750_H
#define _TSP25750_H

#include <cstdint>

#include "../i2c/i2c.hpp"

static constexpr uint8_t TPS25750_EXTERNAL_EEPROM_ADDRESS = 0x50;

enum TPS25750_SLAVE_ADDRESSES {
  // NOTE: bit 0 is noted as R/W on page 44 of the datasheet.
  TPS25750_SADDR_1 = 0b001000000,
  TPS25750_SADDR_2 = 0b001000010,
  TPS25750_SADDR_3 = 0b001000100,
  TPS25750_SADDR_4 = 0b001000110
};

class TPS25750 {
 public:
  static TPS25750 *inst;

  static TPS25750 *get_instance() {
    if (nullptr == inst) {
      inst = new TPS25750();
    }
    return inst;
  }

 private:
  TPS25750();
  ~TPS25750();
  TPS25750(TPS25750 const &) = delete;
  TPS25750 &operator=(TPS25750 const &) = delete;

  /* private members */
  I2C i2c;
};

#endif /* END _TSP25750_H */

/* END OF FILE */
