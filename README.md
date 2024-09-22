# Pico Drivers
*A compilation of drivers I have written for RP2040 projects*

## Updates

**09/14/24**
I was able to compile my drivers on the new version of the sdk. I was not able to get the vscode extension to import the project properly. The error arises from `ninja` failing to create the necessary build files for cross-compilation. Current workaround is to create a new project and manually copy the code over.
Tmux script still functions as intented.


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

## Build Script

The build script acts a simple way to create a clean build of your project. It does not require `sudo` privileges.


# Picoshell: Port of [Microshell](https://github.com/marcinbor85/microshell/tree/main)

>A lightweight pure C implementation of virtual shell, compatible with VT100 terminal. Support root tree, run-time mounting paths, global commands, and much more. Works out-of-the-box on Arduino-compatible boards. Dedicated for bare-metal embedded systems. Thanks to the asynchronous architecture it does not block operations and can be easily implemented even on small microcontrollers with relatively small resources.

This library is a port of the amazing [Microshell](https://github.com/marcinbor85/microshell/tree/main) Arduino library to the [Pico series](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html) of microcontrollers. Picoshell replaces the Arduino API calls to use their equivalent [pico-sdk](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html) calls.

>Why bother? Isn't the Pico already supported by Arduino anyway?

That is true! And the [earlephilhower](https://github.com/earlephilhower/arduino-pico/) core is great! But what if you already have a project using the `pico-sdk` *(like say a repository of device drivers)*? This library has got you covered.

This port doesn't come with the baggage of the Arduino framework--for better or for worse--reducing the code size significantly. There is also something to be said about reinventing the wheel. 

>It's called Picoshell but the API still refers to it as `ush`, what the heck?

Also true. The intention was to maintain backwards compatability with existing Microshell code already in the wild, and to make it easier for me to maintain this port in the future.

## Features

*Features of Picoshell are largely the same as the Arduino counterpart with a few exceptions.*

- Built-in commands 
    - `ls` 
    - `cat`
    - `pwd`
    - `help`
    - `xxd`
    - `echo`
- Tab auto-completion
- Backspace-key functionality
- Works on any Rp2 based board, RISC is fine too!
- Written in C, but fully compatible with C++
- Easy to extend for your own projects
- Simple to integrate into existing projects
- Example code provided in this repo
- Fully asynchronous architecture (static callbacks are not a problem)
- Non-blocking API (just a single non-blocking function call in the main loop)
- Supports customizable root tree containing static virtual files with callbacks
- No dynamic memory allocations (no memory leaks--no problems)
- No internal buffers (support unlimited data stream lengths)
- No static variables (making it possible to use multiple independent shells on a single system)
- Compatible vith VT100 standard (works out of the box with default putty configuration)

**WISHLIST**
- ~~unit and functional tests (for greater certainty that nothing will break down)~~ 
- ~~translation-ready (do You want Hindi translation? no problem!)~~


## Using Picoshell

### Define an I/O Interface:


**Example Serial Interface:**
```cpp
// non-blocking read interface
static int ush_read(struct ush_object *self, char *ch) {
  // Implement as a FIFO
  if (stdio_get_until(ch, BUF_IN_SIZE, PICO_ERROR_TIMEOUT) > 0) {
    return 1;
  }
  return 0;
}

// non-blocking write interface
static int ush_write(struct ush_object *self, char ch) {
  // Implement as a FIFO
  return (printf("%c", ch) == 1);
}

// I/O interface
static const struct ush_io_interface ush_iface = {
  .read = ush_read,
  .write = ush_write,
};
```

- [] TODO: Provide Example Interface using an I2C [display driver](#ssd1306-oled-display-driver)

### Define the Shell instance:

```cpp
// Buffer sizes (size can be customized)
#define BUF_IN_SIZE 128
#define BUF_OUT_SIZE 128
#define PATH_MAX_SIZE 128

static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];

// Picoshell instance handler
static struct ush_object ush;

// Picoshell descriptor
static const struct ush_descriptor ush_desc = {
  .io = &ush_iface,                           // I/O interface pointer
  .input_buffer = ush_in_buf,                 // working input buffer
  .input_buffer_size = sizeof(ush_in_buf),    // working input buffer size
  .output_buffer = ush_out_buf,               // working output buffer
  .output_buffer_size = sizeof(ush_out_buf),  // working output buffer size
  .path_max_length = PATH_MAX_SIZE,           // path maximum length (stack)
  .hostname = "Pico2",                        // hostname (in prompt)
};

// root directory handler
static struct ush_node_object root;
```

### Setup and operation:

```cpp
void setup() {
  // Uncomment the following line if using UART for the console
  // setup_default_uart();
  stdio_init_all();
  ush_init(&ush, &ush_desc);
  
  // Mount directories (root must be first)
  ush_node_mount(&ush, "/", &root, NULL, 0);
}

int main() {
  setup();
  while (1) {
    ush_service(&ush);
    /* Other non-blocking code goes here */
  }
  return 0;
}
```


### Customization:

In this example we will add a `bin` directory to our filesystem and two files we can execute to control the onboard led. To do requires a bit of setup. 

First we must create callbacks for our shell to execute.
```cpp
// toggle file execute callback
static void toggle_exec_callback(struct ush_object *self,
                                 struct ush_file_descriptor const *file, int argc,
                                 char *argv[]) {
  // simple toggle led, without any arguments validation
  gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
}

// set file execute callback
static void set_exec_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, int argc,
                              char *argv[]) {
  // arguments count validation
  if (argc != 2) {
    // return predefined error message
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }

  // arguments validation
  if (strcmp(argv[1], "1") == 0) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
  } else if (strcmp(argv[1], "0") == 0) {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
  } else {
    // return predefined error message
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }
}
```

Then we create the `bin` directory:
```cpp
// bin directory files descriptor
static const struct ush_file_descriptor bin_files[] = {
    {
        .name = "toggle",              // toggle file name
        .description = "toggle led",   // optional file description
        .help = "usage: toggle\r\n",   // optional help manual
        .exec = toggle_exec_callback,  // optional execute callback
    },
    {.name = "set",  // set file name
     .description = "set led",
     .help = "usage: set {0,1}\r\n",
     .exec = set_exec_callback},
};

// bin directory handler
static struct ush_node_object bin;
```

Then during `setup()` we can mount bin to our filesystem:
```cpp
void setup() {
  /* setup serial port and mount root */
  ush_node_mount(&ush, "/bin", &bin, bin_files, sizeof(bin_files) / sizeof(bin_files[0]));
}
```
For more examples, see `main.cpp`.


# Drivers

## Displays

*LCDs, OLEDs, Seven-Segment LEDs, Bar-graph LEDs, and more...*

### SSD1306: OLED Display Driver

Library for SSD1306 OLED display driver based on the works presented in the official [pico examples](https://github.com/raspberrypi/pico-examples/tree/master/i2c/ssd1306_i2c) and this [repo](https://github.com/MR-Addict/Pi-Pico-SSD1306-C-Library/tree/main). I have made this library compatible with the `I2C` wrapper class presented in this repository.

#### Displaying Text

```cpp
#include "ssd1306.hpp"

int main() {
  OLED oled = OLED(128, 64, true);
  oled.print(0,0, (uint8_t*)"Hello World!");
  oled.show();
  return 0;
}
```


#### Drawing Shapes

```cpp
#include "ssd1306.hpp"

int main() {
  OLED oled = OLED(128, 64, true);

  // Draws an outline of the shape
  oled.draw_rectangle(0, 0, 50, 30);

  // Draws a solid of the shape
  oled.draw_filled_circle(64, 32, 10);

  oled.show();

  return 0;
}
```


#### Drawing Images

```cpp
#include "ssd1306.hpp"
#include "bitmap.hpp"

int main() {
  OLED oled = OLED(128, 64, true);

  oled.drawBitmap(0, 0, 32, 32, temperature_32x32);
  oled.show();

  return 0;
}
```


#### Other Cool Stuff

- You can create your own [custom fonts](http://oleddisplay.squix.ch/#/home).

- You can modifty characters using the [Adafruit Font Customizer](https://tchapi.github.io/Adafruit-GFX-Font-Customiser/).


## USB-PD

*For projects that require high amounts of power, USB-PD can be a great option for battery chargers, LED drivers, Power banks, and more...*

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


### STUSB4500: USB-PD Sink Controller

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

### TPS25750: DRP USB-PD Controller

THE TPS25750 is Dual Role Port (DRP) USB-PD controller with integrated power paths with support for dead-battery mode. The module I used for writing the code was the [USB Power Click from Mikroe Electronics](https://www.mikroe.com/usb-c-power-click). The TPS25750 includes a number of GPIO pins for additional customization such as indictator LEDs or switches.

**NOTE:** This library is a WIP.

*via Texas Instruments*
>The TPS25750 is a highly integrated stand-alone USB Type-C and Power Delivery (PD) controller optimized for applications supporting USB-C PD Power. The TPS25750 integrates fully managed power paths with robust protection for a complete USB-C PD solution. The TPS25750 also integrates control for external battery charger ICs for added ease of use and reduced time to market.

 [Link to datasheet:](https://www.ti.com/lit/ds/symlink/tps25750.pdf?ts=1714726260022&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTPS25750%253FkeyMatch%253DTPS25750%2526tisearch%253Dsearch-everything%2526usecase%253DGPN-ALT)
 
 [Link to Host Interface Technical Reference Manual:](https://www.ti.com/lit/ug/slvuc05a/slvuc05a.pdf?ts=1714750459366&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTPS25750)
