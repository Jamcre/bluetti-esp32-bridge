# ecolibrium2025-sensors

---

## Project Goals
Our goal is to develop a system for continuous monitoring of indoor & outdoor air conditions and quality. We hope to use the data gathered to inform building retrofits aimed at reducing carbon emmissions, improving public health, and reducing energy spending.

### What We Measure
- Temperature (in Fahrenheit)
- Humidity
- CO<sub>2</sub> (carbon dioxide) levels
- VOX (volitaile organics index)
- NOx (nitric oxide gasses index)
- Particulate Matter
  - PM 1
  - PM2.5
  - PM4
  - PM10

Sensirion Gas Index: https://github.com/Sensirion/gas-index-algorithm

### How This Can Inform Building Retrofits
A "tight" building envelope (minimizing natural infiltration) often reduces the amount of active heating and cooling needed. However, this also leads to worse air quality since less outdoor air enters to replace pollutatns generated indoors by occupants and other sources explain. Our system can show these phenomena in real time and help diagnose the needs of a room.

We use the system to understand the pollutant exposure over time for the interns working at Loisaida. When the indoor air quality is too low, we can open a door and use a fan to circulate fresh air into the room. This quantifies and mitigates pollutant exposure.

---

## Hardware Setup
Requirements:
- ESP32-2432S028R CYD (Mainboard) 
- SEN66 sensor with 6 cable JST connector (provided by SENSIRION with sensor purchase)
- 3D printed case (CAD file available in Github @ ecolibrium2025-sensors/_hardware/_CAD)
- 4-cable JST cable connector (for CYD)
- microSD
- Secondary ESP32 (does not have to be a CYD) providing the Real Time Clock (RTC) value via NTP (code in hub folder)
- Type C to compatible laptop port cable (must support data transfers)
- ***NOTE***: The provided JST by SENISIRION has exposed wire at the ends. To connect to the 4-cable JST for the CYD, you can use male and female jumper wire connectors to connect the wires.

Where each wire goes:

| Sensor               | CYD Pins               |
|----------------------|------------------------|
| White                |3.3 V                   |
| Blue                 |  GND                   |
| Green                | IO22                   |
| Yellow               | IO27                   |
| Black                | Unconnected            |
| Red                  | Unconnected            |

These are what the pins on the CYD board look like up close:

<picture>
  <img src="https://imgur.com/sOC1x62.jpeg" style="width:auto;">
</picture>

Pin definitions:
The JST port on the CYD with IO22 and IO27 pins are used. SDA (data) is set to pin 22 and SCL (clock) is set to pin 27.

Insert the SD card into the CYD.

Once everything is connected and situated, the components should look like this:

<picture>
  <img src="https://imgur.com/1J5RoLy.jpeg" style="width:auto;">
</picture>

## Sensor Software Setup
This setup has three main parts. First, you will setup your sensor node and get its MAC (Media Access Control) address. Then, you will set up your RTC hub with the Mac Address you received. Finally, you will complete the wifi setup.

### Prerequisites:
- Arduino IDE
    - You can download the latest release [here](https://www.arduino.cc/en/software/)

### Download the latest release from Github:
- Navigate to the [releases section](https://github.com/ecolibrium-nyc/ecolibrium2025-sensors/releases) and download the "Source code" zip file from the latest release
- Open file explorer, right-click the `.zip` you just downloaded, and extract the zip into your destination of choice

<picture>
  <div align="center">
    <img src="https://imgur.com/lcfGY0Q.jpeg" style="width:auto;">
  </div>
</picture>
<picture>
  <div align="center">
    <img src="https://imgur.com/rlCR3fE.jpeg" style="width:auto;">
  </div>
</picture>

- This will create a new folder with the same name. You will find all the code needed to run everything inside it

### How to download and set up the ESP32 - CYD firmware:
- Within the extracted 'ecolobrium-2025' folder, navigate to the 'sensor' subfolder. Inside should be a folder labeled 'stable' and a CSV file labeled 'example_sd_data'
- Open a new file manager tab and navigate to the 'Arduino' folder, which should have 'libraries' and 'sketches' subfolders inside of it
  - If you are unsure how to find it, navigate to Preferences (<kbd>Ctrl</kbd> + <kbd>,</kbd> / <kbd>&#8984; Cmd</kbd> + <kbd>,</kbd>)
  - This will open the preferences setting and show your sketchbook location. Clicking 'browse' will reveal it in your file explorer
  
  <picture>
    <div align="center">
      <img src="https://imgur.com/aOKDMxw.jpeg"   style="width:auto;">
    </div>
  </picture>

- From the file manager tab that has the 'sensor' folder open, move the subfolder labeled 'stable' inside Arduino's 'sketches' folder
- Before proceeding, read the NOTE written below to make sure you do not lose any files
- Replace your old Arduino 'libraries' folder with the one provided @ ecolibrium2025-sensors/_sensor/libraries

***NOTE***: 

By replacing your old 'libraries' folder with the provided one on Git, you would lose any libraries that are downloaded on your laptop and but not in this Git folder. We suggest that to recover those libraries, you should either   redownload them or save them elsewhere and move them back into your Arduino 'libraries' folder after downloading this one.

Changes we made to default libraries will be taken care of for you if you pull directly from our Github (changes explained in later section)

### Uploading Sketch & Finding MAC Address via Arduino IDE
- The contents of the 'stable' folder will contain the `stable.ino` file along with its dependencies
- Open the `stable.ino` file with Arduino IDE
- Plug your CYD board into your laptop using a USB C cable
- In Arduino IDE, navigate to the 'Select Board' menu at the top, select the port you are using and select your board to be the "ESP32-2432S028R CYD" (you can type CYD into the search bar to get the board)
- Click Tools (top menu) -> Set Partition Scheme to 'Huge APP'

<picture>
  <img src="https://imgur.com/XM4l5Na.jpeg" style="width:auto;">
</picture>

- Open Serial Monitor (maginifying glass icon in top right)
- Set baud rate (a dropdown menu on the right side of the Serial Monitor window) to 115200

<picture>
  <div align="center">
    <img src="https://imgur.com/DdT4BEW.jpeg" style="width:25%">
  </div>
</picture>

- In the `stable.ino` file, at lines 28 and 31, set the module name and password "IndoorModuleXX" to the desired unique name

<picture>
  <div align="center">
    <img src="https://imgur.com/qUyDH40.jpeg" style="width:auto;">
  </div>
</picture>

- Now, click the Upload button on the top left (arrow icon) to upload the file to one of your CYDs (this will take a few moments)
- If code uploads and the board has power, it should compile. Check the serial monitor output. These outputs attempt to explain what the program is doing
- After uploading the code, look at the start of the 'Output' window in Arduino IDE and record the node's MAC Address, this will be used to set up the RTC hub

<picture>
  <div align="center">
    <img src="https://imgur.com/CpjTIR0.jpeg" style="width:auto;">
  </div>
</picture>

- Your sensor will not start recording data until you set up the RTC 'hub'

### Physical Mounting and Case:
- CAD files located in repo @ ecolibrium2025-sensors/_hardware/CAD.
- Ensure the sensor node is mounted such that water does not infiltrate.

## Real Time Clock 'Hub'
### How to download and set up RTC hub firmware:
- In the 'ecolibrium-2025' folder you extracted, navigate to the 'hub' subfolder
- Inside the 'hub' folder, open the subfolder named 'sender' where you should see a header (.h) file labeled `secrets_example.h` and an ino file labeled `sender.ino`
- Rename `secrets_example.h` to `secrets.h` and open it in a text editor of your choice (Notepad works fine if you do not have any other text editors)
- Add your wifi details to the contents of `secrets.h` by filling in where the quotes are (do not delete the quotes).
    - change const char* sta_ssid = "your wifi username";
    - change const char* sta_password = "your wifi password";
- Using the MAC address previously recorded from the 'Output' menu, replace the contents of `sender.ino` to include the MAC addresses of your sensor nodes at the top of the file.
    - To find a board's MAC address, refer to [this section](https://github.com/ecolibrium-nyc/ecolibrium2025-sensors/tree/main?tab=readme-ov-file#uploading-sketch--finding-mac-address-via-arduino-ide)
    - Replace the comma seperated codes inside the {} in the lines that look like this:
    `
    uint8_t broadcastAddress1[] = {0x5c, 0x01, 0x3b, 0x51, 0x2e, 0x64};
    `
    (keep the `0x`, only change the last two characters of each part of the address)

- Look for the repeated logic to register peers in void setup. ensure all of your nodes are registered by copying this logic including your broadcastAddress' (remember they are numbered 1, 2, 3, 4, ...).

<picture>
  <div align="center">
    <img src="https://imgur.com/LK2398K.jpeg" style="width:auto;">
  </div>
</picture>

### Upload via Arduino IDE:
- The same Arduino libraries setup used for the sensor node will allow you to compile sender.ino
- Upload your script
- You should see frequent outputs indicating the time is being broadcast
- You will likely see zero registered clients even while your sensor nodes are working properly. This is because your nodes only briefly connect for the time then immediately disconnect

## How to set up wifi details for your sensor node:
- You will need another device capable of connecting to wifi for the following steps
- Connect to your board's wifi access point
- Enter wifi settings username and password defined in code as "IndoorModuleXX"
- It will create a new hotspot with its name "IndoorModuleXX"
- You can connect to this new wifi hotspot on your device and navigate to [192.168.4.1](https://192.168.4.1) to monitor the data in real time!
- ***NOTE***: Only connect to the CYD with one device at a time
- The code also interfaces with our local data polling hub at Loisaida

The boards should now function as fully fledged air quality sensors. Indoor air quality measurements will be recorded and stored onto the slotted SD card, and the built-in screen will display real-time readings. An example file of what the recorded data looks like is included in the sensor folder named `example_sd_data.csv`

***NOTE***: This system was designed specifically to be run at Loisaida Lab. At Loisaida Lab, a seperate system scrapes the generated html and renders the measurements on a live dashboard.

---
### Disclaimers
- This system has not been tested or designed for cyber-security
- This system was designed specifically to be used at Loisiada lab and to interface with a custom local data polling system

---
## End of Main README
---
# ESP32-2432S028R Cheap Yellow Display (CYD)

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
## End of CYD focused README
---

## Notes on changes made in the 'libraries' folder compared to the default

### LVGL Config Changes:
----------
***NOTE***: The library version is 9.1.0 but the config file is 9.2.0
* Line 15: set 0 to 1 to enable content
* Line 79: #define LV_DEF_REFR_PERIOD  30 (originally 33)
* Line 217: #define LV_USE_LOG 1 (originally 0)
* Line 231: #define LV_LOG_PRINTF 1 (originally 0)
* Line 388: #define LV_USE_PRIVATE_API 1 (originally 0)
* Lines 396-416: enabled all monsterrat font sizes to 1
* Lines 700-701 were added: 
Memory used by FreeType to cache characters in kilobytes #define LV_FREETYPE_CACHE_SIZE 768
* Lines 706-709 were added: 
    /* Maximum number of opened FT_Face/FT_Size objects managed by this cache instance. */
    /* (0:use system defaults) */
    #define LV_FREETYPE_CACHE_FT_FACES 8
    #define LV_FREETYPE_CACHE_FT_SIZES 8

- Line 911: #define LV_USE_TFT_ESPI 1 (originally 0)
- Line 920: #define LV_USE_ILI9341 1 (originally 0)

### TFT_espi User Setup Changes
- Line 56: commented out #define ILI9341_DRIVER (generic driver)
- Line 57: UNcommented #define ILI9341_2_DRIVER (using this driver)
- Line 98: UNcommented #define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
- Line 102: #define TFT_HEIGHT 320 // ST7789 240 x 320
- Line 142: UNcommented #define TFT_BL   21 (original value was 32)
- Lines 180-183: Commented multiple pin definitions (PIN_D8, PIN_D3, AND PIN_D4) and TFT_RST
- Lines 217: Commented #define TFT_MISO 12
- Line 218: UNcommented #define TFT_MOSI 13 (originally was 23)
- Line 219: UNcommented #define TFT_SCLK 14 (originally was 18)
- Line 220: UNcommented #define TFT_CS   15  // Chip select control pin
- Line 221: UNcommented #define TFT_DC    2  // Data Command control pin
- Line 223: UNcommented #define TFT_RST  -1 (display reset is connected to ESP32 reset)
- Line 225: ADDED #define TOUCH_CS 33     // Chip select pin (T_CS) of touch screen
- Line 370: Commented #define SPI_FREQUENCY  27000000
- Line 372: UNcommented #define SPI_FREQUENCY  55000000
- Line 384: UNcommented #define USE_HSPI_PORT
---
## End of Library README
---
# Ventilation Standards
Usually standards regulate ventilation or filtration rather than pollutant levels so thats why there isnt much available.

Sources in "_docs":

ASHRAE Standard 62.1-2013.pdf - perscribes minimum ventilation rate (cfm/person, cfm/ft^2, L/s per person, L/s per m^2)

GRIHA IAQ Standard Limits and Thresholds.jpg - limits proposed by Indian research team. Does not include NOx or Pm4. Considers HCHO but not VOc's as a category. Tentative CO2 recomendation of <1000 ppm

GRIHA Comparison of Regulations and Guidelines.jpg - image includes EPA, OSHA, WHO, and other recomendations.
