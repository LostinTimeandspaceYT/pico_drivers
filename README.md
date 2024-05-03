# Pico Drivers
*A compilation of drivers I have written for RP2040 projects*


## Tmux Debug Script
*A script used to simplify the debugging process without the need for an IDE*

### Prerequisites

- `tmux` (obviously)

you can install tmux with the following command:

#### Debian/Ubuntu

```sh
sudo apt install tmux
```

#### Fedora

```sh
sudo dnf install tmux
```

For more information on using tmux, run `man tmux`.

When using the `tmux_debug.sh` to debug, be sure `openocd` can be run with sudo privileges and without need for a password. This can be done by performing the following steps:

1. run `sudo visudo`
2. add the following line to your `/etc/sudoers`:
    `user_name ALL=(ALL) NOPASSWD:/usr/bin/openocd`
    **Note:** replace `user_name` with your username.
3. Save the file.



# Drivers

## USB-PD

### AP33772: USB-PD Sink Controller

Driver for the AP33772, a high performance USB-PD Sink Controller. The module I used for writing this code was the [USB sink click 2: Mikroe Electronics](https://www.mikroe.com/usb-c-sink-2-click) 

*via Mikroe Electronics*

>The USB Sink Click 2 supports dead battery mode to allow a system to be powered from an external source directly, establishes a valid source-to-sink connection, and negotiates a USB power delivery (PD) contract with a PD-capable source device. It also supports a flexible PD3.0 and PPS for applications that require direct voltage and current requests, with fine-tuning capabilities. This Click board™ makes the perfect solution for the development of USB Type-C connector-equipped battery-powered devices or DC-power input devices, USB PD3.0 PPS testers, USB Type-C to traditional barrel-connector power-adapter cables, and more.
>
>
>This USB Type-C power delivery sink controller requires power from a standard USB source adapter, in our case from the USB connector labeled USB-C PD-IN, and then delivers power to connected devices on the VSINK connector. Between USB and VSINK terminal stands a pair of MOSFETs, according to the AP33772's driver for N-MOS VBUS power switch support. The PD controller can control the external NMOS switch ON or OFF (all control is done via the I2C interface). The USB C connector acts as a PD-IN discharge path terminal with a USB Type-C configuration channels 1 and 2. The presence of the power supply on the USB C is indicated over the VBUS LED.
>
>
>The host MCU can control the PPS with 20mV/step voltage and 50mA/step current. The PD controller supports overtemperature protection (OTP), OVP with auto-restart, OCP with auto-restart, one-time programming (OTP), power-saving mode, and a system monitor and control status register. For OTP, this Click board™ comes with an NTC temperature sensor, with selectable temperature points (25°C, 50°C, 75°C, 100°C) as a temperature threshold. The onboard FAULT LED serves as a visual presentation of the negotiation mismatch. The Multi-time programming (MTP) is reserved for future configuration.

**Links:**

[Datasheet](https://www.diodes.com/part/view/AP33772/)

[USB sink click 2: Mikroe Electronics](https://www.mikroe.com/usb-c-sink-2-click)


### STUSB4500

Driver for the STUSB4500, a USB-PD Sink Controller made by STMicroelectronics. The module I used for writing this code was the [USB Sink Click from Mikroe Electronics](https://www.mikroe.com/usb-c-sink-click)


*via Mikroe Electronics*
>the STUSB4500, a USB-C sink-only controller compatible with Power-Delivery (PD) from STMicroelectronics. It supports dead battery mode to allow a system to be powered from an external source directly, establishes a valid source-to-sink connection, and negotiates a USB power delivery (PD) contract with a PD capable source device.
>
> the STUSB4500, a USB-C sink-only controller compatible with Power-Delivery (PD) from STMicroelectronics. Based on the default power profiles (PDO) configuration stored in internal non-volatile memory, the stand-alone controller STUSB4500 implements proprietary algorithms to negotiate a Power Delivery contract with a source without any internal or external software support (Auto-Run Mode), making it the ideal device for automatic High Power Profile charging, especially from a Dead Battery Power state.
>
>The STUSB4500 communicates with MCU using the standard I2C interface that supports transfers up to 400 Kbit/s (Fast Mode) used to configure, control, and read the status of the device. It also has the possibility of the USB Power Delivery communication over CC1 and CC2 configuration channel pins used for connection and attachment detection, plug orientation determination, and system configuration management across USB Type-C cable. Four 7-bit device address is available by default (0x28 or 0x29 or 0x2A or 0x2B) depending on the setting of the address pin ADDR0 and ADDR1. 

**Links:**

[Datasheet](https://www.st.com/en/interfaces-and-transceivers/stusb4500.html)

[USB Sink Click: Mikroe Electronics](https://www.mikroe.com/usb-c-sink-2-click)

