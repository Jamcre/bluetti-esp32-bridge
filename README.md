# ecolibrium2025-sensors

## Current tasks 07/17/25
- clean up code to reduce lines and compiled memory footprint
- add antennae
- print a case
 
We are ready to deploy one module for continuous monitoring.

## Recently completed
- integrate GUI with SD card logic
- integrate RTC with that
- integrate wifi hotspot and html logic to be compatable with data aggregation

# Set up changes
lvgl: usersetup montserral fonts -> 1
eTSPI: left one pin unassigned
we changed partition to Huge APP (Tools -> Partition)

## Hardware Setup
Components:
The ESP CYD (Mainboard) The SEN66 sensor 3D printed case (CAD file available in Github @ ecolibrium2025-sensors/_hardware/_CAD)

Where each cable goes:
Sensor -> CYD

White -> Red Blue -> Black Green -> Blue Yellow -> Yellow Black is unused Red is unused

Pins definitions
A specific JST port is used (note which) SDA is set to SCL is set to

How to install CYD firmware
Pull the CYD_firmware.ino and the libraries (Arduino libraries folder) folder from github. Move them to their correct locations (clarify which)

Upload via Arduino IDE (elaborate)

How to set up wifi details
Physical Mounting and Case
Now these devices are full fledged air quality sensors.

## Project Goals
To develop a system for continous monitoring of indoor + outdoor air conditions and quality. We hope to use the gathered data to inform building retrofits to reduce carbon emmisions, harm done to people's health, and energy bills.

### What we measure
We measure temperature, humidity, CO2, VOC index, NOx index, PM1, PM2,5, PM4, and PM10 avoid acronyms and add units!.
Sensirion Gas Index: https://github.com/Sensirion/gas-index-algorithm

### How this can inform building retrofits
A "tight" building envelope (minimizing natural infiltration) often reduces the amount of active heating and cooling needed. However, this also leads to worse air quality since less outdoor air enters to replace pollutatns generated indoors by occupants and other sources explain. Our system can show these phenomena in real time and help diagnose the needs of a room.

We use the system to understand the pollutant exposure over time for the interns working at Loisaida. When the indoor air quality is too low, we can open a door and use a fan to circulate fresh air into the room. This quantifies and mitigates pollutant exposure.

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

test






