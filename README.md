# MicroTuya
light weight C library for smart Tuya devices

many thanks to Jason Cox, this repo is inspired from his tinytuya repo (https://github.com/jasonacox/tinytuya)

## Description
Light C library to control and monitor through WiFi various [Tuya](https://en.tuya.com/) smart devices. In order to use the library devices must already be activated and the following information is required for each device:

* The key used by the device for encryption
* The device ID
* The UUID
* The IP address of the device in your local network

## Setup
To build the project you currently simply need the GCC compiler, the Makefile contains all the building routines. Simply run the Makefile using the command:
```bash
make
```
Once the building is completed, the executable will be located in the `build` folder, the default executable is the `main`.

NOTE Currently this is only tested on macOS and Linux.

## Launching a command for a specific device
In order to run a command, you can simply use the following command:
```bash
./build/main ip-address port key device-id UUID command-code extra-parameters
```
Currently debugging is enabled by default, the `stdout` will provide all the routine information.

### Implemented commands
Current supported commands are:
|  Command name  |     Functionality     | Command code | Quantity of parameters |
|----------------|:---------------------:|:------------:|:----------------------:|
| SWITCH-ON      | turn light on         |      0       |           0            |
| SWITCH-OFF     | turn light off        |      1       |           0            |
| SET-BRIGHTNESS | set brightness        |      2       |           1            |
| SET-COLOR-TEMP | set color temperature |      3       |           1            |
| SET-WHITE      | set white color       |      4       |           2            |
| SET-COLOR      | set RGB color         |      5       |           3            |

### Examples
1. Turn on the light:
```bash
./build/main 192.168.0.1 6668 XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXX 0
```
2. Set the light color to red:
```bash
./build/main 192.168.0.1 6668 XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXX 5 255 0 0
```

## This README is still a work in progress, more details will be provided in the future
