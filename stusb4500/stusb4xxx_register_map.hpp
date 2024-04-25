/** @file stusb4xxx_register_map.hpp
 * 
 * @brief Register Map for the ST4XXX USB PD controllers.
 * 
 * @author Nathan Winslow
 * 
 * @par 
 * For information about the specific IC's, see their respective datasheet.
 * 
*/

#pragma once
#include<stdint.h>


/* Register Map */
static const uint8_t BCD_TYPEC_REV_LOW =            0x06;
static const uint8_t BCD_TYPEC_REV_HIGH =           0x07;
static const uint8_t BCD_USBPD_REV_LOW =            0x08;
static const uint8_t BCD_USBPD_REV_HIGH =           0x09;
static const uint8_t DEVICE_CAPAB_HIGH =            0x0a;
static const uint8_t ALERT_STATUS_1 =               0x0b;
static const uint8_t ALERT_STATUS_1_MASK =          0x0c;
static const uint8_t PORT_STATUS_0 =                0x0d;
static const uint8_t PORT_STATUS_1 =                0x0e;
static const uint8_t TYPEC_MONITORING_STATUS_0 =    0x0f;
static const uint8_t TYPEC_MONITORING_STATUS_1 =    0x10;
static const uint8_t CC1_CONNECTION_STATUS =        0x11;
static const uint8_t CC_HW_FAULT_STATUS_0 =         0x12;
static const uint8_t CC_HW_FAULT_STATUS_1 =         0x13;
static const uint8_t PD_TYPEC_STATUS =              0x14; //Sink only
static const uint8_t TYPEC_STATUS =                 0x15; //Sink only
static const uint8_t PRT_STATUS =                   0x16;
static const uint8_t PHY_STATUS =                   0x17; //Source only
/* 0x18, 0x19 Reserved */
static const uint8_t PD_COMMAND_CTRL =              0x1a; 
/* 0x1b, 0x1c Reserved */
static const uint8_t DEVICE_CTRL =                  0x1d; //Source only
static const uint8_t ANALOG_CTRL =                  0x1e; //Source only
/* 0x1f reserved */
static const uint8_t MONITORING_CTRL_0 =            0x20;
static const uint8_t MONITORING_CTRL_1 =            0x21; //Source only
static const uint8_t MONITORING_CTRL_2 =            0x22;
static const uint8_t RESET_CTRL =                   0x23;
static const uint8_t POWER_ACCESSORY_CTRL =         0x24;  //Source only
static const uint8_t VBUS_DISCHARGE_TIME_CTRL =     0x25;
static const uint8_t VBUS_DISCHARGE_CTRL =          0x26;
static const uint8_t VBUS_CTRL =                    0x27;
/* 0x28 reserved */
static const uint8_t PE_FSM =                       0x29; 
// 0x2a -> 0x2c Reserved
static const uint8_t GPIO_SW_GPIO =                 0x2d; //Sink only
static const uint8_t SPARE_BITS =                   0x2e; //Source only
static const uint8_t DEVICE_ID =                    0x2f;
// 0x30 Reserved

/* Rx Registers */
static const uint8_t RX_HEADER_LOW =                0x31;
static const uint8_t RX_HEADER_HIGH =               0x32;

static const uint8_t RX_DATA_OBJ1_0 =               0x33;
static const uint8_t RX_DATA_OBJ1_1 =               0x34;
static const uint8_t RX_DATA_OBJ1_2 =               0x35;
static const uint8_t RX_DATA_OBJ1_3 =               0x36;

static const uint8_t RX_DATA_OBJ2_0 =               0x37;
static const uint8_t RX_DATA_OBJ2_1 =               0x38;
static const uint8_t RX_DATA_OBJ2_2 =               0x39;
static const uint8_t RX_DATA_OBJ2_3 =               0x3a;

static const uint8_t RX_DATA_OBJ3_0 =               0x3b;
static const uint8_t RX_DATA_OBJ3_1 =               0x3c;
static const uint8_t RX_DATA_OBJ3_2 =               0x3d;
static const uint8_t RX_DATA_OBJ3_3 =               0x3e;

static const uint8_t RX_DATA_OBJ4_0 =               0x3f;
static const uint8_t RX_DATA_OBJ4_1 =               0x40;
static const uint8_t RX_DATA_OBJ4_2 =               0x41;
static const uint8_t RX_DATA_OBJ4_3 =               0x42;

static const uint8_t RX_DATA_OBJ5_0 =               0x43;
static const uint8_t RX_DATA_OBJ5_1 =               0x44;
static const uint8_t RX_DATA_OBJ5_2 =               0x45;
static const uint8_t RX_DATA_OBJ5_3 =               0x46;

static const uint8_t RX_DATA_OBJ6_0 =               0x47;
static const uint8_t RX_DATA_OBJ6_1 =               0x48;
static const uint8_t RX_DATA_OBJ6_2 =               0x49;
static const uint8_t RX_DATA_OBJ6_3 =               0x4a;

static const uint8_t RX_DATA_OBJ7_0 =               0x4b;
static const uint8_t RX_DATA_OBJ7_1 =               0x4c;
static const uint8_t RX_DATA_OBJ7_2 =               0x4d;
static const uint8_t RX_DATA_OBJ7_3 =               0x4e;
/* 0x4f, 0x50 Reserved */
static const uint8_t TX_HEADER_LOW =                0x51;  //Sink only
static const uint8_t TX_HEADER_HIGH =               0x52;  //Sink only
/* 0x63 -> 0x6f Reserved */

/* Contract Registers */
static const uint8_t DPM_PDO_NUMB =                 0x70;

static const uint8_t DPM_SRC_PDO1_0 =               0x71; //Start Source 
static const uint8_t DPM_SRC_PDO1_1 =               0x72;
static const uint8_t DPM_SRC_PDO1_2 =               0x73;
static const uint8_t DPM_SRC_PDO1_3 =               0x74;

static const uint8_t DPM_SRC_PDO2_0 =               0x75;
static const uint8_t DPM_SRC_PDO2_1 =               0x76;
static const uint8_t DPM_SRC_PDO2_2 =               0x77;
static const uint8_t DPM_SRC_PDO2_3 =               0x78;

static const uint8_t DPM_SRC_PDO3_0 =               0x79;
static const uint8_t DPM_SRC_PDO3_1 =               0x7a;
static const uint8_t DPM_SRC_PDO3_2 =               0x7b;
static const uint8_t DPM_SRC_PDO3_3 =               0x7c;

static const uint8_t DPM_SRC_PDO4_0 =               0x7d;
static const uint8_t DPM_SRC_PDO4_1 =               0x7e;
static const uint8_t DPM_SRC_PDO4_2 =               0x7f;
static const uint8_t DPM_SRC_PDO4_3 =               0x80;

static const uint8_t DPM_SRC_PDO5_0 =               0x81;
static const uint8_t DPM_SRC_PDO5_1 =               0x82;
static const uint8_t DPM_SRC_PDO5_2 =               0x83;
static const uint8_t DPM_SRC_PDO5_3 =               0x84; //End Source

static const uint8_t DPM_SNK_PDO1_0 =               0x85;  //Start Sink
static const uint8_t DPM_SNK_PDO1_1 =               0x86;
static const uint8_t DPM_SNK_PDO1_2 =               0x87;
static const uint8_t DPM_SNK_PDO1_3 =               0x88;

static const uint8_t DPM_SNK_PDO2_0 =               0x89;
static const uint8_t DPM_SNK_PDO2_1 =               0x8A;
static const uint8_t DPM_SNK_PDO2_2 =               0x8B;
static const uint8_t DPM_SNK_PDO2_3 =               0x8C;

static const uint8_t DPM_SNK_PDO3_0 =               0x8D;
static const uint8_t DPM_SNK_PDO3_1 =               0x8E;
static const uint8_t DPM_SNK_PDO3_2 =               0x8F;
static const uint8_t DPM_SNK_PDO3_3 =               0x90;  //End Sink

static const uint8_t DPM_REQ_RDO3_0 =               0x91;
static const uint8_t DPM_REQ_RDO3_1 =               0x92;
static const uint8_t DPM_REQ_RDO3_2 =               0x93;
static const uint8_t DPM_REQ_RDO3_3 =               0x94;