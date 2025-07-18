# ecolibrium2025-sensors

---

## Current tasks 07/17/25
- clean up code to reduce lines and compiled memory footprint
- add antennae
- print a case
 
We are ready to deploy one module for continuous monitoring.

## Recently completed
- integrated GUI,SD card logic, real-time clock, wifi hotspot, and html logic to be compatable with data aggregation

---

## Project Goals
To develop a system for continous monitoring of indoor + outdoor air conditions and quality. We hope to use the gathered data to inform building retrofits to reduce carbon emmisions, harm done to people's health, and energy bills.

### What we measure
We measure temperature, humidity, CO2, VOC index, NOx index, PM1, PM2,5, PM4, and PM10 avoid acronyms and add units!.
Sensirion Gas Index: https://github.com/Sensirion/gas-index-algorithm

### How this can inform building retrofits
A "tight" building envelope (minimizing natural infiltration) often reduces the amount of active heating and cooling needed. However, this also leads to worse air quality since less outdoor air enters to replace pollutatns generated indoors by occupants and other sources explain. Our system can show these phenomena in real time and help diagnose the needs of a room.

We use the system to understand the pollutant exposure over time for the interns working at Loisaida. When the indoor air quality is too low, we can open a door and use a fan to circulate fresh air into the room. This quantifies and mitigates pollutant exposure.

---

## Hardware Setup
Components:
- The ESP CYD (Mainboard) 
- The SEN66 sensor 
- 3D printed case (CAD file available in Github @ ecolibrium2025-sensors/_hardware/_CAD)
- JST cable connector
- microSD
- ESP32 providing the real time clock value via NTP (code in hub folder)

Where each cable goes:

- Sensor -> CYD
- White -> Red 
- Blue -> Black 
- Green -> Blue 
- Yellow -> Yellow 
- Black is unused 
- Red is unused

Pins definitions:
The JST port on the CYD with IO22 and IO27 pins is used. SDA (data) is set to pin 22 SCL (clock) is set to pin 27.

Insert the SD card into the CYD.

Ensure the RTC is being provided via NTP (us the code in the hub folder)

## Software Setup
How to install and set up CYD firmware:
- Pull the CYD_firmware.ino and the libraries (Arduino libraries folder) folder from github. 
- Replace your old Arduino 'libraries' folder with the one provided @ ecolibrium2025-sensors/_sensor/libraries

Changes we made to default libraries (will be taken care of for you if you pull directly from our Github)
- lvgl: usersetup montserral fonts -> 1
- eTSPI: left one pin unassigned
- in User_Setup for the TFT library, left one line commnented out (double check with original library version)

Upload via Arduino IDE:
- Tools -> Partition set to 'Huge APP'
- set baud rate to 115200
  set the module name and password "IndoorModuleXX" to the desired unique name
- if code uploads and the board has power it should run. check the serial monitor output. it should look like ...

How to set up wifi details:
- connect to wifi access point 
- enter wifi settings username and password defined in code as "IndoorModuleXX"
- it will create a new hotspot with its name "IndoorModuleXX"
- you can connect to this new wifi hotspot on your laptop and navigate to '192.168.4.1' to see the data!
- the code also interfaces with our local data polling hub at Loisaida

Physical Mounting and Case:
- CAD files located in repo @ ecolibrium2025-sensors/_hardware/CAD

Now these devices are full fledged air quality sensors.

---












---
## Other (probably to be sorted into technical docs)
### Ventilation Standards
Usually standards regulate ventilation or filtration rather than pollutant levels so thats why there isnt much available.

Sources in "_docs":

ASHRAE Standard 62.1-2013.pdf - perscribes minimum ventilation rate (cfm/person, cfm/ft^2, L/s per person, L/s per m^2)

GRIHA IAQ Standard Limits and Thresholds.jpg - limits proposed by Indian research team. Does not include NOx or Pm4. Considers HCHO but not VOc's as a category. Tentative CO2 recomendation of <1000 ppm

GRIHA Comparison of Regulations and Guidelines.jpg - image includes EPA, OSHA, WHO, and other recomendations.

# ESP32-2432S028R Cheap yellow Display (CYD)

## Overview

This module is a compact all-in-one ESP32-based development board designed for graphical interfaces and interactive applications that uses the ESP32-WROOM-32 as the base. It combines a dual-core microcontroller with a resistive touchscreen LCD, integrated peripherals, and versatile connectivity options. Ideal for prototyping embedded UI, IoT devices, and sensor-driven systems.

---

## 🔧 Specifications

| Feature               | Description                                   |
|----------------------|-----------------------------------------------|
| **Microcontroller**  | ESP32-WROOM-32 (dual-core)                    |
| **CPU Frequency**    | Up to 240 MHz                                 |
| **SRAM**             | 520 KB                                        |
| **ROM**              | 448 KB                                        |
| **Flash Memory**     | 4 MB                                          |
| **Display**          | 2.4" TFT LCD, 240x320 resolution              |
| **Touchscreen**      | Resistive touch                               |
| **WiFi**             | 802.11 b/g/n (2.4 GHz)                        |
| **Bluetooth**        | Bluetooth v4.2 BR/EDR and BLE                 |
| **Operating Voltage**        | 5 V                 |

---

## 🖥️ Display & Peripherals

- **LCD Screen** with backlight control
- **Resistive Touchscreen** controller
- **Speaker Drive Circuit** for audio output
- **Photosensitive Circuit** (ambient light sensing)
- **RGB LED Control Circuit**
- **TF (microSD) Card Interface**
- **Serial Interface (UART)**
- **DHT11 Sensor Interface** for temperature and humidity
- **Addition General IO Ports** for general-purpose expansion

---

## 🔌 Interfaces

- SPI for display and SD card
- I2C for sensors
- UART for serial communication
- PWM for backlight or buzzer control
- ADC/DAC for sensor input/output

---

## 📦 Software & Development Support

This module is compatible with multiple development environments:
- ✅ **Arduino IDE**
- ✅ **ESP-IDF**
- ✅ **MicroPython**
- ✅ **Mixly (Graphical Programming)**

---

## 🔍 Applications

- Environmental monitoring
- Interactive control panels
- IoT dashboards
- Sensor data logging
- DIY handheld consoles

---

## 🔗 Resources

- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [ESP32 Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/README.md)
- [ESP32-WROOM-32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf)
---







