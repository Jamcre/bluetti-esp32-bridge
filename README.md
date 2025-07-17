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
We measure temperature, humidity, CO2, VOCs, NOx, PM1, PM2,5, PM4, and PM10 avoid acronyms and add units!.

### How this can inform building retrofits
A "tight" building envelope (minimizing natural infiltration) often reduces the amount of active heating and cooling needed. However, this also leads to worse air quality since less outdoor air enters to replace pollutatns generated indoors by occupants and other sources explain. Our system can show these phenomena in real time and help diagnose the needs of a room.

We use the system to understand the pollutant exposure over time for the interns working at Loisaida. When the indoor air quality is too low, we can open a door and use a fan to circulate fresh air into the room. This quantifies and mitigates pollutant exposure.

### Ventilation Standards
Usually standards regulate ventilation or filtration rather than pollutant levels so thats why there isnt much available.

Sources in "_docs":

ASHRAE Standard 62.1-2013.pdf - perscribes minimum ventilation rate (cfm/person, cfm/ft^2, L/s per person, L/s per m^2)

GRIHA IAQ Standard Limits and Thresholds.jpg - limits proposed by Indian research team. Does not include NOx or Pm4. Considers HCHO but not VOc's as a category. Tentative CO2 recomendation of <1000 ppm

GRIHA Comparison of Regulations and Guidelines.jpg - image includes EPA, OSHA, WHO, and other recomendations.
