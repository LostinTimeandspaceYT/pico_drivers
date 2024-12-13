/**
 * Access: RO
 * Num data bytes: 4
 * Unique per port: no
 * Description: Indicates operational state of the port.
 * */
#define USB_PD_MODE (0x03)
/*
 * Access: RO
 * Num data bytes: 4
 * Unique per port: no
 * Description: Default resposne is 'I2C '
 * */
#define USB_PD_TYPE (0x04)
/*
 * Access: RO
 * Num data bytes: 8
 * Unique per port: no
 * Description: Default resposne is 'I2C '
 * */
#define USB_PD_CUSTUSE (0x06)
/*
 * Access: RW
 * Num data bytes: 4
 * Unique per port: yes
 * Description: Command resister for the primary command interface.
 * */
#define USB_PD_CMD1 (0x08)
/*
 * Access: RW
 * Num data bytes: 64
 * Unique per port: yes
 * Description: Data register for the primary command interface (CMD1).
 * */
#define USB_PD_DATA1 (0x09)

/*
 * Access: RO
 * Num data bytes: 4
 * Unique per port: no
 * Description: Description of supported features
 * */
#define USB_PD_DEVICE_CAPABILITIES (0x0D)

/*
 * Access: RO
 * Num data bytes: 4
 * Unique per port: no
 * Description: BCD version number, bootloader/application voder version. 
 * Represented as VVVV.MM.RR with leading 0s.
 * */
#define USB_PD_VERSION (0x0F)

/*
 * Access: RO
 * Num data bytes: 11
 * Unique per port: yes
 * Description: Interrupt event bit field for I2Cs_IRQ. 
 * If any bit in this register is 1, then the I2C_IRQ pin is pulled low.
 * */
#define USB_PD_INT_EVENT1 (0x14)

/*
 * Access: RW
 * Num data bytes: 11
 * Unique per port: yes
 * Description: Interrupt mask bit field for INT_EVENT1. 
 * A bit in in INT_EVENT1 cannot be set if it cleared in this register
 * */
#define USB_PD_INT_MASK1 (0x16)

/*
 * Access: RW
 * Num data bytes: 11
 * Unique per port: yes
 * Description: Interrupt clear bit field for INT_EVENT1. 
 * Bits set in this register are cleared from INT_EVENT1.
 * */
#define USB_PD_INT_CLEAR1 (0x18)

/*
 * Access: RO
 * Num data bytes: 5
 * Unique per port: yes
 * Description: Status bit field for non-interrupt events.
 * */
#define USB_PD_STATUS (0x1A)

/*
 * Access: RO
 * Num data bytes: 5
 * Unique per port: no
 * Description: Power path status.
 * */
#define USB_PD_POWER_PATH_STATUS (0x26)

/*
 * Access: RW
 * Num data bytes: 4
 * Unique per port: yes
 * Description: Configuration bits affecting system policy. These bits may
 * change during normal operation and are used for controller the respective port.
 * The PD controller does not take immediate action upon writing. Changes made 
 * to this register take effect the next time in the appropriate policy is invoked.
 * */
#define USB_PD_POWER CONTROL (0x29)

/*
 * Access: RO
 * Num data bytes: 5
 * Unique per port: no
 * Description: Detailed status of boot process. This register provides details 
 * on PD controller boot flags, customer OTP configuration, and silicon revision.
 * */
#define USB_PD_BOOT_STATUS (0x2D)

/*
 * Access: RO
 * Num data bytes: 49
 * Unique per port: no
 * Description: Build description. This is an ASCII string that uniquiely 
 * identifies custom build information.
 * */
#define USB_PD_BUILD_DESCRIPTION (0x2E)

/*
 * Access: RO
 * Num data bytes: 40
 * Unique per port: no
 * Description: Device information. This is an ASCII string with hardware and 
 * firmware version information of the PD controller.
 * */
#define USB_PD_DEVICE_INFO (0x2F)

/*
 * Access: RO
 * Num data bytes: 29
 * Unique per port: yes
 * Description: Received Source Capabilites. This register stores latest 
 * Source Capabailites message received over BMC.
 * */
#define USB_PD_RX_SOURCE_CAPS (0x30)

/*
 * Access: RO
 * Num data bytes: 29
 * Unique per port: yes
 * Description: Received Sink Capabilites. This register stores latest 
 * Sink Capabailites message received over BMC.
 * */
#define USB_PD_RX_SINK_CAPS (0x31)

/*
 * Access: RW
 * Num data bytes: 31
 * Unique per port: yes
 * Description: Source Capabilites for sending. This register stores PDOs and 
 * settings for outgoing Source Capabailites PD messages.
 * */
#define USB_PD_TX_SOURCE_CAPS (0x32)

/*
 * Access: RW
 * Num data bytes: 29
 * Unique per port: yes
 * Description: Sink Capabilites for sending. This register stores PDOs and 
 * settings for outgoing Sink Capabailites PD messages.
 * */
#define USB_PD_TX_SINK_CAPS (0x33)

/*
 * Access: RO
 * Num data bytes: 6
 * Unique per port: yes
 * Description: Power data object for active contract. This register stores 
 * PDO data for the current explicit USB PD contract, or all 0s if not contract.
 * */
#define USB_PD_ACTIVE_CONTRACT_PDO (0x34)

/*
 * Access: RO
 * Num data bytes: 4
 * Unique per port: yes
 * Description: Power data object for active contract. This register stores 
 * the RPO for the current explicit USB PD contract, or all 0s if not contract.
 * */
#define USB_PD_ACTIVE_CONTRACT_RDO (0x35)

/*
 * Access: RO
 * Num data bytes: 2
 * Unique per port: yes
 * Description: Details about the power of the connection. This register 
 * reports status regarding the power of the connection
 * */
#define USB_PD_POWER_STATUS (0x3F)

/*
 * Access: RO
 * Num data bytes: 4
 * Unique per port: yes
 * Description: Status of PD and Type-C state machine. This register contains 
 * details regarding the status of PD messages and the Type-C state machine.
 * */
#define USB_PD_PD_STATUS (0x40)

/*
 * Access: RO
 * Num data bytes: 4
 * Unique per port: yes
 * Description: Contains current status of both CCn pins.
 * */
#define USB_PD_TYPEC_STATE (0x69)

/*
 * Access: RO
 * Num data bytes: 8
 * Unique per port: no
 * Description: Captures status and settings of all GPIO pins.
 * */
#define USB_PD_GPIO_STATUS (0x72)
