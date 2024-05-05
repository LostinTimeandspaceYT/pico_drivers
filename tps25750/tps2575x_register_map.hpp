enum TPS2575X_USB_PD_REGISTER {
  /**
   * Access: RO
   * Num data bytes: 4
   * Unique per port: no
   * Description: Indicates operational state of the port.
   * */
  USB_PD_MODE = 0x03,
  /*
   * Access: RO
   * Num data bytes: 4
   * Unique per port: no
   * Description: Default resposne is 'I2C '
   * */
  USB_PD_TYPE = 0x04,
  /*
   * Access: RO
   * Num data bytes: 8
   * Unique per port: yes
   * Description: Default resposne is 'I2C '
   * */
  USB_PD_CUSTUSE = 0x06,
  /*
   * Access: RW
   * Num data bytes: 4
   * Unique per port: yes
   * Description: Command resister for the primary command interface.
   * */
  USB_PD_CMD1 = 0x08,
  /*
   * Access: RW
   * Num data bytes: 64
   * Unique per port: yes
   * Description: Data register for the primary command interface (CMD1).
   * */
  USB_PD_DATA1 = 0x09,
};
