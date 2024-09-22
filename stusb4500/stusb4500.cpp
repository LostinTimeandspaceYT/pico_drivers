#include "stusb4500.hpp"

STUSB4500::STUSB4500() : i2c(I2C()) { init_pins(); }

STUSB4500 *STUSB4500::inst = nullptr;

STUSB4500::~STUSB4500() {
  gpio_deinit(STUSB4500_INTERRUPT_PIN);
  gpio_deinit(STUSB4500_RESET_PIN);
  delete inst;
}

void STUSB4500::init_pins() {
  gpio_init(STUSB4500_RESET_PIN);
  gpio_set_dir(STUSB4500_RESET_PIN, GPIO_OUT);
  gpio_put(STUSB4500_RESET_PIN, 0);

  gpio_init(STUSB4500_INTERRUPT_PIN);
  gpio_set_dir(STUSB4500_INTERRUPT_PIN, GPIO_OUT);
  gpio_put(STUSB4500_INTERRUPT_PIN, 1);
  gpio_set_irq_enabled(STUSB4500_INTERRUPT_PIN, GPIO_IRQ_EDGE_FALL, true);
}

void STUSB4500::soft_reset() {
  write_byte_to_reg(TX_HEADER_LOW, SOFT_RESET);
  write_byte_to_reg(PD_COMMAND_CTRL, SEND_CMD);
}

void STUSB4500::write_byte_to_reg(uint8_t addr, uint8_t value) {
  uint8_t buffer[2] = {addr, value};
  i2c.write_blocking(STUSB4500_ADDRESS, &buffer[0], sizeof(buffer), false);
}

void STUSB4500::write_to_reg(uint8_t addr, uint8_t *data) {
  // TODO: likely a better way of doing this
  uint8_t buff[sizeof(data) + 1];
  buff[0] = addr;
  for (int i = 0; i < sizeof(data); ++i) {
    buff[i + 1] = data[i];
  }
  i2c.write_blocking(STUSB4500_ADDRESS, buff, sizeof(buff), false);
}

void STUSB4500::read_from_reg(uint8_t addr, uint8_t num_of_bytes, uint8_t *rbuf) {
  if (num_of_bytes > 8) num_of_bytes = 8;
  i2c.write_blocking(STUSB4500_ADDRESS, &addr, 1, false);
  int bytes_read = i2c.read_blocking(STUSB4500_ADDRESS, rbuf, num_of_bytes, false);
  if (bytes_read != num_of_bytes) puts("Error Reading from i2c");
}

void STUSB4500::exit_test_mode() {
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_RST_N);
  write_byte_to_reg(FTP_CUST_PASSWORD_REG, 0);
}

void STUSB4500::wait_exec() {
  bool done = false;
  while (!done) {
    uint8_t *data = new uint8_t[1]();
    read_from_reg(FTP_CUST_REQ, 1, data);
    if (data[0] & FTP_CUST_REQ)
      continue;
    else
      done = true;
  }
}

void STUSB4500::enter_write_mode(uint8_t esector) {
  write_byte_to_reg(FTP_CUST_PASSWORD_REG, FTP_CUST_PASSWORD);
  write_byte_to_reg(RW_BUFFER, 0);
  write_byte_to_reg(FTP_CTRL_0, 0);
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N);
  uint8_t val = ((esector << 3) & FTP_CUST_SER) | WRITE_SER & FTP_CUST_OPCODE;
  write_byte_to_reg(FTP_CTRL_1, val);
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ);
  wait_exec();

  write_byte_to_reg(FTP_CTRL_1, SOFT_PROG_SECTOR & FTP_CUST_OPCODE);
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ);
  wait_exec();

  write_byte_to_reg(FTP_CTRL_1, ERASE_SECTOR & FTP_CUST_OPCODE);
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ);
  wait_exec();
}

void STUSB4500::write_sector(uint8_t sector_num, uint8_t *data) {
  write_to_reg(RW_BUFFER, data);
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N);
  write_byte_to_reg(FTP_CTRL_1, WRITE_PL & FTP_CUST_OPCODE);
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ);
  wait_exec();

  write_byte_to_reg(FTP_CTRL_1, PROG_SECTOR & FTP_CUST_OPCODE);
  uint8_t value =
      ((sector_num & FTP_CUST_SECT) | FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ);
  write_byte_to_reg(FTP_CTRL_0, value);
  wait_exec();
}

void STUSB4500::set_pdo_num(PDO_NUM pdo_num) { write_byte_to_reg(DPM_PDO_NUMB, pdo_num); }

void STUSB4500::write_pdo(PDO_NUM pdo_num, uint8_t *data) {
  if (pdo_num < PDO_1 || pdo_num > PDO_3) return;
  uint8_t addr_mask = 0x85;
  write_to_reg(addr_mask + ((pdo_num - 1) * 4), data);
}

void STUSB4500::load_pdo(PDO_NUM pdo_num) {
  uint8_t addr_mask = 0x85;
  uint8_t address = addr_mask + ((pdo_num - 1) * BYTES_PER_PDO);
  uint8_t *buff = new uint8_t[BYTES_PER_PDO]();
  read_from_reg(address, BYTES_PER_PDO, buff);
  for (int i = 0; i < BYTES_PER_PDO; ++i) {
    pdo_data.arr[i] = buff[i];
  }
}

bool STUSB4500::set_voltage(PDO_NUM pdo_num, float volts) {
  if (5.0 < volts > 20.0) return false;
  if (pdo_num == PDO_1) volts = 5;
  volts *= 20;
  uint16_t val = (uint16_t)volts;
  load_pdo(pdo_num);
  pdo_data.value &= ~0xffc00;
  pdo_data.value |= val << 10;
  write_pdo(pdo_num, pdo_data.arr);
  return true;
}

bool STUSB4500::set_current(PDO_NUM pdo_num, float i) {
  if (0.01 < i > 5.0) return false;
  uint16_t mask = 0x03ff;
  i *= 100;
  uint16_t value = (uint16_t)i & mask;
  load_pdo(pdo_num);
  pdo_data.value &= ~mask;
  pdo_data.value |= value;
  write_pdo(pdo_num, pdo_data.arr);
  return true;
}

bool STUSB4500::set_lower_volt_limit(PDO_NUM pdo_num, float value) {
  if (5.0 < value > 20.0) return false;
  value -= 5.0;
  /* save the bits, not the type */
  unsigned char *c = reinterpret_cast<unsigned char *>(&value);
  if (pdo_num == PDO_2) {
    sector[3][4] &= 0x0f;
    sector[3][4] |= *c << 4;
  } else if (pdo_num == PDO_3) {
    sector[3][6] &= 0xf0;
    sector[3][6] |= *c << 4;
  }
  return true;
}

bool STUSB4500::set_upper_volt_limit(PDO_NUM pdo_num, float value) {
  if (5.0 < value > 21.0) return false;

  value -= 5.0;
  /* save the bits, not the type */
  unsigned char *c = reinterpret_cast<unsigned char *>(&value);
  if (pdo_num == PDO_1) {
    sector[3][3] &= 0x0f;
    sector[3][3] |= *c << 4;
  } else if (pdo_num == PDO_2) {
    sector[3][5] &= 0xf0;
    sector[3][5] |= *c << 4;
  } else {
    sector[3][6] &= 0x0f;
    sector[3][6] |= *c << 4;
  }
  return true;
}

bool STUSB4500::set_flex_current(float value) {
  if (0.0 < value > 5.0) return false;
  value *= 100;
  unsigned char *c = reinterpret_cast<unsigned char *>(&value);
  sector[4][3] &= 0x03;
  sector[4][3] |= (*c & 0x3f) << 2;
  sector[4][4] &= 0xf0;
  sector[4][4] |= (*c & 0x03c0) >> 6;
  return true;
}

void STUSB4500::set_ext_power(bool enable) {
  sector[3][2] &= 0xf7;
  sector[3][2] |= enable << 3;
}

void STUSB4500::set_usb_comms_capable(bool enable) {
  sector[3][2] &= 0xfe;
  sector[3][2] |= enable;
}

bool STUSB4500::set_POWER_OK_config(uint8_t config_code) {
  if (2 < config_code > 3) return false;
  sector[4][4] &= 0x9f;
  sector[4][4] |= config_code << 5;
  return true;
}

bool STUSB4500::set_gpio_ctrl(uint8_t ctrl_code) {
  if (0 < ctrl_code > 3) return false;
  sector[1][0] &= 0xcf;
  sector[1][0] |= ctrl_code << 4;
  return true;
}

void STUSB4500::set_power_over_5v_only(bool enable) {
  sector[4][6] &= 0xf7;
  sector[4][6] |= enable << 4;
}

void STUSB4500::set_request_source_current(bool enable) {
  sector[4][6] &= 0xef;
  sector[4][6] |= enable >> 4;
}

float STUSB4500::get_voltage(PDO_NUM pdo_num) {
  uint16_t mask = 0x3ff;
  load_pdo(pdo_num);
  float voltage = ((pdo_data.value >> 10) & mask) / 20.0;
  return voltage;
}

float STUSB4500::get_current(PDO_NUM pdo_num) {
  uint16_t mask = 0x3ff;
  load_pdo(pdo_num);
  pdo_data.value &= mask;
  return (float)pdo_data.value * 0.01;
}

float STUSB4500::get_low_volt_limit(PDO_NUM pdo_num) {
  float result;
  if (pdo_num == PDO_2)
    result = (sector[3][4] >> 4) + 5.0;
  else if (pdo_num == PDO_3)
    result = (sector[3][6] & 0x0f) + 5.0;
  else
    result = 0.0;
  return result;
}

float STUSB4500::get_upper_volt_limit(PDO_NUM pdo_num) {
  float result;
  if (pdo_num == PDO_1)
    result = (sector[3][3] >> 4) + 5.0;
  else if (pdo_num == PDO_2)
    result = (sector[3][5] & 0x0f) + 5.0;
  else
    result = (sector[3][6] >> 4) + 5.0;
  return result;
}

float STUSB4500::get_flex_current() {
  return (((sector[4][4] & 0x0f) << 6) + (sector[4][3] & 0xfc) >> 2) / 100.0;
}

uint8_t STUSB4500::get_pdo_num() {
  uint8_t *reg = new uint8_t[1]();
  read_from_reg(DPM_PDO_NUMB, 1, reg);
  return reg[0] & 0x07;
}

uint8_t STUSB4500::get_POWER_OK_config() { return (sector[4][4] & 0x60) >> 5; }

uint8_t STUSB4500::get_GPIO_ctrl() { return (sector[1][0] & 0x30) >> 4; }

float STUSB4500::get_requested_source_current() {  // FIXME!
  return (sector[4][6] & 0x10) >> 4;
}

bool STUSB4500::is_ext_power() { return (sector[3][2] & 0x08) >> 3; }

bool STUSB4500::is_usb_comm_capable() { return sector[3][2] & 0x01; }

bool STUSB4500::is_power_over_5v_only() { return (sector[4][6] & 0x80) >> 3; }

void STUSB4500::reset_sector() {
  for (int i; i < NUM_OF_SECTORS; ++i) {
    for (int j; j < SIZE_OF_SECTOR; ++j) {
      sector[i][j] = 0x00;
    }
  }
}

void STUSB4500::read() {
  write_byte_to_reg(FTP_CUST_PASSWORD_REG, FTP_CUST_PASSWORD);
  write_byte_to_reg(FTP_CTRL_0, 0x00);  // NVM reset
  write_byte_to_reg(FTP_CTRL_0, FTP_CUST_PWR | FTP_CUST_RST_N);
  reset_sector();
  for (int i = 0; i < NUM_OF_SECTORS; ++i) {
    write_byte_to_reg(FTP_CTRL_0,
                      FTP_CUST_PWR | FTP_CUST_RST_N);  // may not be needed
    write_byte_to_reg(FTP_CTRL_1, READ & FTP_CUST_OPCODE);
    write_byte_to_reg(FTP_CTRL_0,
                      FTP_CUST_SECT | FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ);
    uint8_t *rbuf = new uint8_t[1]();
    while (*rbuf & FTP_CUST_REQ != 0) {
      read_from_reg(FTP_CTRL_0, 1, rbuf);
      tight_loop_contents();
    }
    delete rbuf;
    uint8_t *rwbuf = new uint8_t[SIZE_OF_SECTOR]();
    read_from_reg(RW_BUFFER, SIZE_OF_SECTOR, rwbuf);

    // TODO: Check this logic, we may want to change the first element
    for (int i = 0; i < NUM_OF_SECTORS; ++i) {
      if ((sector[i] == 0) && (i > 0)) {
        for (int j = 0; j < 8; ++j) {
          sector[i][j] = rwbuf[j];
        }
        break;
      }
    }
  }
  exit_test_mode();
}

void STUSB4500::write(bool default_mode) {
  if (default_mode) {
    uint8_t def_sector_1[SIZE_OF_SECTOR] = {0x00, 0x00, 0xb0, 0xaa,
                                            0x00, 0x45, 0x00, 0x00};
    uint8_t def_sector_2[SIZE_OF_SECTOR] = {0x10, 0x40, 0x9C, 0x1C,
                                            0xFF, 0x01, 0x3C, 0xDF};
    uint8_t def_sector_3[SIZE_OF_SECTOR] = {0x02, 0x40, 0x0F, 0x00,
                                            0x32, 0x00, 0xFC, 0xF1};
    uint8_t def_sector_4[SIZE_OF_SECTOR] = {0x00, 0x19, 0x56, 0xAF,
                                            0xF5, 0x35, 0x5F, 0x00};
    uint8_t def_sector_5[SIZE_OF_SECTOR] = {0x00, 0x4B, 0x90, 0x21,
                                            0x43, 0x00, 0x40, 0xFB};
    uint8_t *def_sectors[NUM_OF_SECTORS] = {def_sector_1, def_sector_2, def_sector_3,
                                            def_sector_4, def_sector_5};
    enter_write_mode(SECTOR_0 | SECTOR_1 | SECTOR_2 | SECTOR_3 | SECTOR_4);
    for (int i = 0; i < NUM_OF_SECTORS; ++i) {
      write_sector(i, def_sectors[i]);
    }
  } else {
    uint8_t nvmi[3] = {0, 0, 0};  // just to be explicit
    float volts[3] = {0.0, 0.0, 0.0};

    for (int i = 0; i < 3; ++i) {
      load_pdo(PDO_NUM(i + 1));
      float ival = (pdo_data.value & 0x3ff) * 0.01;
      if (ival > 5.0) ival = 5.0;
      if (ival < 0.5)
        nvmi[i] = 0;
      else if (ival <= 3.0)
        nvmi[i] = (uint8_t)(ival * 4.0) - 1;
      else {
        nvmi[i] = (uint8_t)(ival * 2.0) + 5;
        uint32_t digv = (pdo_data.value >> 10) & 0x3ff;
        float voltage = digv / 20.0;
        if (voltage < 5.0) {
          voltage = 5.0;
        } else if (voltage > 20.0) {
          voltage = 20.0;
        }
        volts[i] = voltage;
        sector[3][2] &= 0x0f;
        sector[3][2] |= nvmi[0] << 4;

        sector[3][4] &= 0xf0;
        sector[3][4] |= nvmi[1];

        sector[3][5] &= 0x0f;
        sector[3][5] |= nvmi[2] << 4;

        // PDO 1: Fixed at 5V .5A

        // PDO 2:
        digv = volts[1] * 20;
        sector[4][0] &= 0x3f;
        sector[4][0] |= (digv & 0x03) << 6;
        sector[4][1] = digv >> 2;

        // PDO 3:
        digv = volts[2] * 20;
        sector[4][2] = digv & 0xff;
        sector[4][3] &= 0xfc;
        sector[4][3] |= digv >> 8;

        uint8_t *data = new uint8_t[1]();
        read_from_reg(DPM_PDO_NUMB, 1, data);
        sector[3][2] &= 0xf9;
        sector[3][2] |= data[0] << 1;
        enter_write_mode(SECTOR_0 | SECTOR_1 | SECTOR_2 | SECTOR_3 | SECTOR_4);
        for (int i = 0; i < NUM_OF_SECTORS; ++i) {
          write_sector(i, sector[i]);
        }
      }
    }
  }
  exit_test_mode();
}
