# Pico Drivers
*A compilation of drivers I have written for RP2040 projects*


## USB-PD

### AP33772: USB-PD Sink Controller

Driver for the AP33772, a high performance USB-PD Sink Controller. The module I used for writing this code was the [USB sink click 2: Mikroe Electronics](https://www.mikroe.com/usb-c-sink-2-click) 

*via Mikroe Electronics*

```
The USB Sink Click 2 supports dead battery mode to allow a system to be powered from an external source directly, establishes a valid source-to-sink connection, and negotiates a USB power delivery (PD) contract with a PD-capable source device. It also supports a flexible PD3.0 and PPS for applications that require direct voltage and current requests, with fine-tuning capabilities. This Click board™ makes the perfect solution for the development of USB Type-C connector-equipped battery-powered devices or DC-power input devices, USB PD3.0 PPS testers, USB Type-C to traditional barrel-connector power-adapter cables, and more.

...

This USB Type-C power delivery sink controller requires power from a standard USB source adapter, in our case from the USB connector labeled USB-C PD-IN, and then delivers power to connected devices on the VSINK connector. Between USB and VSINK terminal stands a pair of MOSFETs, according to the AP33772's driver for N-MOS VBUS power switch support. The PD controller can control the external NMOS switch ON or OFF (all control is done via the I2C interface). The USB C connector acts as a PD-IN discharge path terminal with a USB Type-C configuration channels 1 and 2. The presence of the power supply on the USB C is indicated over the VBUS LED.

...

The host MCU can control the PPS with 20mV/step voltage and 50mA/step current. The PD controller supports overtemperature protection (OTP), OVP with auto-restart, OCP with auto-restart, one-time programming (OTP), power-saving mode, and a system monitor and control status register. For OTP, this Click board™ comes with an NTC temperature sensor, with selectable temperature points (25°C, 50°C, 75°C, 100°C) as a temperature threshold. The onboard FAULT LED serves as a visual presentation of the negotiation mismatch. The Multi-time programming (MTP) is reserved for future configuration.
```


**Links:**

[Datasheet](https://www.diodes.com/part/view/AP33772/)

[USB SINK CLICK 2: MIKROE ELECTRONICS](https://www.mikroe.com/usb-c-sink-2-click)
