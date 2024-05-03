#include "ap33772.hpp"

AP33772 *AP33772::inst = nullptr;

AP33772::AP33772()
    : i2c(I2C()), num_pdo(0), req_pps_volt(0), exist_pps(0), pps_index(0) {
  reset();
  begin();
}

void AP33772::begin() {
  read_from_reg(CMD_STATUS, 1);
  status.read_status = read_buff[0];

  if (status.is_ovp)
    event_flag.ovp = 1;
  if (status.is_ocp)
    event_flag.ocp = 1;

  if (status.is_ready) { // negotiation finished
    if (status.is_new_pdo) {
      if (status.is_successful)
        event_flag.new_neg_success = 1;
      else
        event_flag.new_neg_fail = 1;
    } else {
      if (status.is_successful)
        event_flag.neg_success = 1;
      else
        event_flag.neg_failed = 1;
    }
  }
  sleep_ms(10);

  // if negotiation is good, load in PDOs from charger
  if (event_flag.new_neg_success) {
    event_flag.new_neg_success = 0;

    read_from_reg(CMD_PDONUM, 1);
    num_pdo = read_buff[0];

    read_from_reg(CMD_SRCPDO, SRCPDO_LENGTH);
    for (int i = 0; i < num_pdo; ++i) {
      pdo_data[i].byte0 = read_buff[i * 4];
      pdo_data[i].byte1 = read_buff[i * 4 + 1];
      pdo_data[i].byte2 = read_buff[i * 4 + 2];
      pdo_data[i].byte3 = read_buff[i * 4 + 3];

      if ((pdo_data[i].byte3 & 0xF0) == 0xC0) { // PPS profile found
        pps_index = i;                          // store index
        exist_pps = 1;                          // turn on flag
      }
    }
  }
}

void AP33772::set_voltage(uint16_t target_voltage) {

  /*
  Step 1: Check if PPS can satisfy request
  Step 2: Scan PDOs to see what is the closest voltage to request (rounded down)
  Step 3: Compare found PDOs voltage and PPS max voltage
  */
  uint8_t temp_index = 0;

  if ((exist_pps) &&
      (pdo_data[pps_index].pps.max_voltage * 100 >= target_voltage) &&
      (pdo_data[pps_index].pps.min_voltage * 100 <= target_voltage)) {
    index_pdo = pps_index;
    req_pps_volt = target_voltage / 20; // unit in 20mV/LSB
    rdo_data.pps.obj_pos = pps_index + 1;
    rdo_data.pps.op_current = pdo_data[pps_index].pps.max_current;
    rdo_data.pps.voltage = req_pps_volt;
    write_rdo();
    return;
  } else {
    // Step 2: Scan PDOs to see what is the closest voltage to request (rounded
    // down)
    for (int i = 0; i < num_pdo - exist_pps; ++i) {
      if (pdo_data[i].fixed.voltage * 50 <= target_voltage)
        temp_index = i;
    }

    // Step 3: Compare found PDOs voltage and PPS max voltage
    if (pdo_data[temp_index].fixed.voltage * 50 >
        pdo_data[pps_index].pps.max_voltage * 100) {
      index_pdo = temp_index;
      rdo_data.fixed.obj_pos = temp_index + 1;
      rdo_data.fixed.max_current = pdo_data[index_pdo].fixed.max_current;
      rdo_data.fixed.op_current = pdo_data[index_pdo].fixed.max_current;
      write_rdo();
      return;
    } else { // PPS voltage >=  fixed PDO
      index_pdo = pps_index;
      req_pps_volt = pdo_data[pps_index].pps.max_voltage *
                     5; // Convert units from 100mV -> 20mV
      rdo_data.pps.obj_pos = pps_index + 1;
      rdo_data.pps.op_current = pdo_data[pps_index].pps.max_current;
      rdo_data.pps.voltage = req_pps_volt;
      write_rdo();
      return;
    }
  }
}

void AP33772::set_max_current(uint16_t target_max_current) {

  if (index_pdo == pps_index) {
    if (target_max_current <= pdo_data[pps_index].pps.max_current * 50) {
      rdo_data.pps.obj_pos = pps_index + 1;
      rdo_data.pps.op_current = target_max_current / 50; // 50mA LSB
      rdo_data.pps.voltage = req_pps_volt;
      write_rdo();
    }
  } else {
    if (target_max_current <= pdo_data[index_pdo].fixed.max_current * 10) {
      rdo_data.fixed.obj_pos = index_pdo + 1;
      rdo_data.fixed.max_current = target_max_current / 10; // 10mA LSB
      rdo_data.fixed.op_current = target_max_current / 10;
      write_rdo();
    }
  }
}

void AP33772::set_NTC(uint16_t TR25, uint16_t TR50, uint16_t TR75,
                      uint16_t TR100) {
  write_buff[0] = TR25 & 0xFF;
  write_buff[1] = (TR25 >> 8) & 0xFF;
  write_to_reg(CMD_TR25, 2);
  sleep_ms(5);

  write_buff[0] = TR50 & 0xFF;
  write_buff[1] = (TR50 >> 8) & 0xFF;
  write_to_reg(CMD_TR50, 2);
  sleep_ms(5);

  write_buff[0] = TR75 & 0xFF;
  write_buff[1] = (TR75 >> 8) & 0xFF;
  write_to_reg(CMD_TR75, 2);
  sleep_ms(5);

  write_buff[0] = TR100 & 0xFF;
  write_buff[1] = (TR100 >> 8) & 0xFF;
  write_to_reg(CMD_TR100, 2);
}

void AP33772::set_derating_temp(uint8_t temp) {
  write_buff[0] = temp;
  write_to_reg(CMD_DRTHRESH, 1);
}

void AP33772::set_mask(AP33772_MASKS mask) {
  read_from_reg(CMD_MASK, 1);
  write_buff[0] = read_buff[0] | mask;
  sleep_ms(5);
  write_to_reg(CMD_MASK, 1);
}

void AP33772::clear_mask(AP33772_MASKS mask) {
  read_from_reg(CMD_MASK, 1);
  write_buff[0] = read_buff[0] & ~mask;
  sleep_ms(5);
  write_to_reg(CMD_MASK, 1);
}

void AP33772::read_from_reg(AP33772_CMDS cmd, uint8_t num_bytes) {
  /* clear the read buffer */
  for (int i = 0; i < READ_BUFF_LENGTH; ++i) {
    read_buff[i] = 0;
  }
  i2c.read_bytes(AP33772_ADDRESS, cmd, read_buff, num_bytes);
}

void AP33772::write_to_reg(AP33772_CMDS cmd, uint8_t num_bytes) {
  i2c.write_bytes(AP33772_ADDRESS, cmd, write_buff, num_bytes);
}

void AP33772::write_rdo() {
  write_buff[0] = rdo_data.byte0;
  write_buff[1] = rdo_data.byte1;
  write_buff[2] = rdo_data.byte2;
  write_buff[3] = rdo_data.byte3;
  write_to_reg(CMD_RDO, 4);
}

uint16_t AP33772::read_voltage() {
  read_from_reg(CMD_VOLTAGE, 1);
  return read_buff[0] * 80; // returns 80mV / LSB
}

uint16_t AP33772::read_current() {
  read_from_reg(CMD_CURRENT, 1);
  return read_buff[0] * 16; // returns 24mA / LSB
}

uint8_t AP33772::read_temp() {
  read_from_reg(CMD_TEMP, 1);
  return read_buff[0];
}

void AP33772::reset() {
  write_buff[0] = 0x00;
  write_buff[1] = 0x00;
  write_buff[2] = 0x00;
  write_buff[3] = 0x00;
  write_to_reg(CMD_RDO, 4);
}

void AP33772::print_pdo() {
  printf("Source PDO Number: %i\n", num_pdo);

  for (int i = 0; i < num_pdo; ++i) {
    if ((pdo_data[i].byte3 & 0xF0) == 0xC0) { // PPS PDO
      printf("PDO[%i] - PPS: %4.3fV~", i + 1,
             (float)pdo_data[i].pps.min_voltage * 100 / 1000);
      printf("%4.3f @ ", (float)pdo_data[i].pps.max_voltage * 100 / 1000);
      printf("%f4.3A\n", (float)pdo_data[i].pps.max_current * 50 / 1000);
    } else if ((pdo_data[i].byte3 & 0xC0) == 0x00) {
      printf("PDO[%i] - Fixed: %4.3fV ", i + 1,
             (float)pdo_data[i].fixed.voltage * 50 / 1000);
      printf("@ %4.3fA\n", (float)pdo_data[i].fixed.max_current * 10 / 1000);
    }
  }
  printf("=========================================================\n");
}
