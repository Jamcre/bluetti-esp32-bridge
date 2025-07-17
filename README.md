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
