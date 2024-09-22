/** @file stusb4500.hpp
 *
 * @brief Module for operation the STUSB4500 USB-PD sink controller.
 *
 * @par
 * Datasheet: https://www.st.com/resource/en/datasheet/stusb4500.pdf
 */

#ifndef _STUSB4500_H
#define _STUSB4500_H

#include "../i2c/i2c.hpp"
#include "stusb4xxx_register_map.hpp"

static const uint STUSB4500_RESET_PIN = PIN_UNUSED;
static const uint STUSB4500_INTERRUPT_PIN = PIN_UNUSED;
static const uint8_t STUSB4500_ADDRESS = 0x28;
static const uint8_t NUM_OF_SECTORS = 5;
static const uint8_t SIZE_OF_SECTOR = 8;
static const uint8_t BYTES_PER_PDO = 4;

/* Op-Codes */
static const uint8_t READ = 0x00;
static const uint8_t WRITE_PL = 0x01;
static const uint8_t WRITE_SER = 0x02;
static const uint8_t ERASE_SECTOR = 0x05;
static const uint8_t PROG_SECTOR = 0x06;
static const uint8_t SOFT_PROG_SECTOR = 0x07;
static const uint8_t SECTOR_0 = 0x01;
static const uint8_t SECTOR_1 = 0x02;
static const uint8_t SECTOR_2 = 0x04;
static const uint8_t SECTOR_3 = 0x08;
static const uint8_t SECTOR_4 = 0x10;
static const uint8_t DEFAULT = 0xFF;
static const uint8_t FTP_CUST_PASSWORD_REG = 0x95;
static const uint8_t FTP_CUST_PASSWORD = 0x47;
static const uint8_t FTP_CTRL_0 = 0x96;
static const uint8_t FTP_CUST_PWR = 0x80;
static const uint8_t FTP_CUST_RST_N = 0x40;
static const uint8_t FTP_CUST_REQ = 0x10;
static const uint8_t FTP_CUST_SECT = 0x07;
static const uint8_t FTP_CTRL_1 = 0x97;
static const uint8_t FTP_CUST_SER = 0xF8;
static const uint8_t FTP_CUST_OPCODE = 0x07;
static const uint8_t RW_BUFFER = 0x53;
static const uint8_t SOFT_RESET = 0x0D;
static const uint8_t SEND_CMD = 0x26;

#define HARD_RESET_USB_SINK            \
  do {                                 \
    gpio_put(STUSB4500_RESET_PIN, 1);  \
    asm volatile("nop \n nop \n nop"); \
    asm volatile("nop \n nop \n nop"); \
    gpio_put(STUSB4500_RESET_PIN, 0);  \
  } while (0)

enum PDO_NUM {
  PDO_1 = 1,
  PDO_2,
  PDO_3,
};

class STUSB4500 {
 private:
  STUSB4500();
  ~STUSB4500();
  register32_t pdo_data;
  uint8_t sector[NUM_OF_SECTORS][SIZE_OF_SECTOR];
  I2C i2c;

  /* Private Methods */
  void init_pins();
  void write_byte_to_reg(uint8_t addr, uint8_t value);
  void write_to_reg(uint8_t addr, uint8_t *data);
  void read_from_reg(uint8_t addr, uint8_t num_of_bytes, uint8_t *rbuf);
  void enter_write_mode(uint8_t esector);
  void write_sector(uint8_t sector_num, uint8_t *data);
  void read();
  void reset_sector();
  void exit_test_mode(void);
  void wait_exec(void);

 public:
  static STUSB4500 *inst;
  /**
   * \brief Singleton object used to control
   * power delivery.
   *
   * \return pointer to the instance.
   */
  static STUSB4500 *get_instance() {
    if (inst == nullptr) inst = new STUSB4500();
    return inst;
  }

  STUSB4500(STUSB4500 const &) = delete;
  STUSB4500 &operator=(STUSB4500 const &) = delete;
  /**
   * \brief performs a soft reset to renegotiate a PD contract
   * with the source.
   */
  void soft_reset();
  void write(bool def);
  void write_pdo(PDO_NUM pdo_num, uint8_t *data);

  /**
   * \brief Read the contents of the PDO register and load it into pdo struct
   */
  void load_pdo(PDO_NUM pdo_num);

  /* Getters */
  float get_voltage(PDO_NUM pdo_num);
  float get_current(PDO_NUM pdo_num);
  float get_low_volt_limit(PDO_NUM pdo_num);
  float get_upper_volt_limit(PDO_NUM pdo_num);
  float get_flex_current();
  uint8_t get_pdo_num();

  /**
   * \brief Configuration Codes:
   *    00b: Configuration 1
   *    01b: N/A
   *    10b: Configuration 2 (Default)
   *    11b: Configuration 3
   *
   * Configuration 1: All PDOs on single VBUS power path
   *
   * Configuration 2: All PDOs on single VBUS power path +
   * dedicated high power charging path
   *
   * Configuration 3: ALL PDOs on single VBUS power path +
   * detection of Type-C current capability from source
   *
   * \note if the VBUS_EN_SINK is in a Hi-Z state, then
   * no source is detected.
   *
   * For more information, check page 7 of the datasheet
   *
   * \return configuration code
   */
  uint8_t get_POWER_OK_config();

  /**
   * \brief Control Codes:
   *    00b: Software controlled GPIO
   *    01b: Hardware fault detection
   *    10b: Debug accessory detection
   *    11b: Indicates USB-C current capability
   *    advertised by the source
   *
   * \return control code
   */
  uint8_t get_GPIO_ctrl();
  float get_requested_source_current();

  /**
   * \brief Sets the pdo to desired number.
   * To change the output voltage a call to soft_reset()
   * is required after setting the pdo number.
   */
  void set_pdo_num(PDO_NUM pdo_num);

  /**
   * \brief Sets the voltage of pdo number to
   * value. Note: pdo 1 cannot be changed from
   * 5V
   *
   * \return True if set, false otherwise
   */
  bool set_voltage(PDO_NUM pdo_num, float value);

  /**
   * \brief Set the current of pdo number to
   * value. as of 2023, USB-PD has a maximum
   * output current of 5.0A
   *
   * \return True if set, false otherwise
   */
  bool set_current(PDO_NUM pdo_num, float i);
  bool set_lower_volt_limit(PDO_NUM pdo_num, float value);
  bool set_upper_volt_limit(PDO_NUM pdo_num, float value);
  bool set_flex_current(float value);
  void set_ext_power(bool enable);
  void set_usb_comms_capable(bool enable);
  bool set_POWER_OK_config(uint8_t config_code);
  bool set_gpio_ctrl(uint8_t ctrl_code);
  void set_power_over_5v_only(bool enable);
  void set_request_source_current(bool enable);

  bool is_ext_power();
  bool is_usb_comm_capable();
  bool is_power_over_5v_only();
};
#endif /* END _STUSB4500_H */

/*End of File*/
