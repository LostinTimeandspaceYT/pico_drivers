/** @file ap33772.hpp
 *
 * @brief this module is a driver for the AP33772, a high performance USB-PD
 * Sink Controller.
 *
 * @par
 * For more information, please see the datasheet.
 *
 *
 * Datasheet: https://www.diodes.com/part/view/AP33772/
 */

#ifndef _AP3372_H
#define _AP3372_H

#include "../i2c/i2c.hpp"

enum AP33772_CMDS {
  CMD_SRCPDO = 0x00,
  CMD_PDONUM = 0x1C,
  CMD_STATUS = 0x1D,
  CMD_MASK = 0x1E,
  CMD_VOLTAGE = 0x20,
  CMD_CURRENT = 0x21,
  CMD_TEMP = 0x22,
  CMD_OCPTHRESH = 0x23,
  CMD_OTPTHRESH = 0x24,
  CMD_DRTHRESH = 0x25,
  CMD_TR25 = 0x28,
  CMD_TR50 = 0x2A,
  CMD_TR75 = 0x2C,
  CMD_TR100 = 0x2E,
  CMD_RDO = 0x30,  // 0x30 - 0x33
  CMD_VID = 0x34
};

enum AP33772_MASKS {
  EN_READY = 1 << 0,
  EN_SUCCESS = 1 << 1,
  EN_NEWPDO = 1 << 2,
  EN_OVP = 1 << 4,
  EN_OCP = 1 << 5,
  EN_OTP = 1 << 6,
  EN_DR = 1 << 7
};

struct AP33772_STATUS {
  union {
    struct {
      uint8_t is_ready : 1;
      uint8_t is_successful : 1;
      uint8_t is_new_pdo : 1;
      uint8_t reserved : 1;
      uint8_t is_ovp : 1;
      uint8_t is_ocp : 1;
      uint8_t is_otp : 1;
      uint8_t is_dr : 1;
    };
    uint8_t read_status;
  };

  uint8_t read_voltage;  // LSB: 80mV
  uint8_t read_current;  // LSB: 24mA
  uint8_t read_temp;     // unit: 1C
};

struct AP33772_EVENT_FLAG {  // used when reading the status of the IC
  union {
    struct {
      uint8_t new_neg_success : 1;
      uint8_t new_neg_fail : 1;
      uint8_t neg_success : 1;
      uint8_t neg_failed : 1;
      uint8_t reserved_1 : 4;
    };
    uint8_t negotiation_event;
  };

  union {
    struct {
      uint8_t ovp : 1;
      uint8_t ocp : 1;
      uint8_t otp : 1;
      uint8_t dr : 1;
      uint8_t reserved_2 : 4;
    };
    uint8_t protect_event;
  };
};

struct PDO_DATA {
  union {
    struct {
      uint32_t max_current : 10;  // unit: 10mA
      uint32_t voltage : 10;      // unit: 50mV
      uint32_t reserved_1 : 10;   // shall be set to zero
      uint32_t type : 2;          // 00b - Fixed supply
    } fixed;

    struct {
      uint32_t max_current : 7;  // unit: 50mA
      uint32_t reserved_2 : 1;   // shall be set to zero
      uint32_t min_voltage : 8;  // unit: 100mV
      uint32_t reserved_1 : 1;   // shall be set to zero
      uint32_t max_voltage : 8;  // unit: 100mV
      uint32_t reserved : 3;     // shall be set to zero
      uint32_t type : 2;         // 00b - Programmable Power Supply (01b .. 11b reserved)
      uint32_t apdo : 2;         // 11b - Augmented Power Data Object (APDO)
    } pps;

    struct {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
    };
    uint32_t data;
  };
};

struct RDO_DATA {
  union {
    struct {
      uint32_t max_current : 10;  // max current in 10mA units
      uint32_t op_current : 10;   // operating current in 10mA units
      uint32_t reserved_1 : 8;    // shall be set to zero
      uint32_t obj_pos : 3;       // object position (000b is reserved)
      uint32_t reserved_2 : 1;    // shall be set to zero
    } fixed;

    struct {
      uint32_t op_current : 7;  // operating current in 50mA units
      uint32_t reserved_1 : 2;  // shall be set to zero
      uint32_t voltage : 11;    // output voltage in 20mV units
      uint32_t reserved_2 : 8;  // shall be set to zero
      uint32_t obj_pos : 3;     // object position (000b is reserved)
      uint32_t reserved_3 : 1;  // shall be set to zero
    } pps;

    struct {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
    };
    uint32_t data;
  };
};

static const uint8_t AP33772_ADDRESS = 0x51;
static const uint8_t READ_BUFF_LENGTH = 30;
static const uint8_t WRITE_BUFF_LENGTH = 6;
static const uint8_t SRCPDO_LENGTH = 28;

/**
 * @class AP33772
 * @brief Singleton class
 *
 */
class AP33772 {
 public:
  static AP33772 *inst;

  static AP33772 *get_instance() {
    if (inst == nullptr) inst = new AP33772();
    return inst;
  }

  /* TODO: decide if we need the INT pin, look at STUSB4500 for syntax */
  void init_pins(void);

  /**
   * @brief Checks if the power supply is compatible and fetches the PDO
   * profile.
   */
  void begin(void);

  /**
   * @brief Set VBUS voltage
   * @param target_voltage desired voltage in mV
   */
  void set_voltage(uint16_t target_voltage);

  /**
   * @brief Set maximum current before tripping at the wall plug
   * @param target_max_current desired current in mA
   */
  void set_max_current(uint16_t target_max_current);

  /**
   * @brief Set resistance value of 10K NTC at 25C, 50C, 75C, and 100C.
   * Default values are 10000, 4161, 1928, 974 Ohm
   *
   * @param TR25, TR50, TR75, TR100 in Ohms
   * @attention Blocking function due to long I2C write;
   * minimum blocking time is 15ms
   */
  void set_NTC(uint16_t TR25, uint16_t TR50, uint16_t TR75,
               uint16_t TR100);  // see EVB user guide page 13 for more information

  /**
   * @brief Set target temperature (C) when output power through USB-C is
   * reduced. Default is 120 C.
   * @param temp temperature in Celcius
   */
  void set_derating_temp(uint8_t temp);

  /**
   * @brief set the mask to flag certain events
   * @param mask condition to flag for.
   */
  void set_mask(AP33772_MASKS mask);

  /**
   * @brief clear the flag associated with the mask.
   * @param mask condition to clear.
   */
  void clear_mask(AP33772_MASKS mask);

  /**
   * @brief Write the desired power profile back to the source
   */
  void write_rdo(void);

  /**
   * @brief Read the VBUS voltage
   * @return voltage in mV
   */
  uint16_t read_voltage(void);

  /**
   * @brief Read VBUS current
   * @return current in mA
   */
  uint16_t read_current(void);

  /**
   * @brief Read NTC temperature
   * @return temperature in C
   */
  uint8_t read_temp(void);

  /**
   * @brief Read maximum VBUS current
   * @return current in mA
   */
  uint16_t get_max_current(void) const;

  /**
   * @brief Hard reset the power supply. Will cause temporary power outage
   */
  void reset();

  /**
   * @brief Debug code to check PPS profile PDOs
   */
  void print_pdo(void);

 private:
  AP33772();
  ~AP33772();
  AP33772(AP33772 const &) = delete;
  AP33772 &operator=(AP33772 const &) = delete;

  void read_from_reg(AP33772_CMDS cmd, uint8_t num_bytes);
  void write_to_reg(AP33772_CMDS cmd, uint8_t num_bytes);

  I2C i2c;
  uint8_t read_buff[READ_BUFF_LENGTH]{0};
  uint8_t write_buff[WRITE_BUFF_LENGTH]{0};
  uint8_t num_pdo;
  uint8_t index_pdo;
  uint32_t req_pps_volt;
  uint8_t exist_pps;
  int8_t pps_index;

  AP33772_STATUS status{0};
  AP33772_EVENT_FLAG event_flag{0};
  RDO_DATA rdo_data{0};
  PDO_DATA pdo_data[7]{0};
};

#endif  // End _AP33772_H
