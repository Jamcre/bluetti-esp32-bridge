# ESP-NOW Time Synchronization Project

## Overview

This project demonstrates a time synchronization system using ESP-NOW protocol between multiple ESP32 devices. One device acts as a sender (time server) that gets accurate time from NTP and broadcasts it to multiple receiver devices.

## Features

- **Sender Device**:
  - Connects to WiFi and synchronizes time with NTP server
  - Broadcasts current time to multiple ESP32 devices via ESP-NOW
  - Supports 9 peer devices with their MAC addresses
  - Dual mode operation (AP + Station)
- **Receiver Device**:
  - Listens for time updates via ESP-NOW
  - Displays received time information with sender MAC address
  - Simple callback-based architecture ## Hardware Requirements
  - ESP32 development boards (1 sender + multiple receivers)
  - WiFi network access for the sender device

## Setup Instructions

### Sender Setup

1. Create a `secrets.h` file with your WiFi credentials:
   ```cpp
   const char* sta_ssid = "YOUR_WIFI_SSID";
   const char* sta_password = "YOUR_WIFI_PASSWORD";
   const char* ap_ssid = "ESP32_AP";
   const char* ap_password = "esp32password";
   ```
2. Update the peer MAC addresses in `sender.ino` with your receiver devices' addresses
3. Upload the sender sketch to your master ESP32 device

### Receiver Setup

1. Upload the receiver sketch to your ESP32 devices
2. Note each device's MAC address (printed to Serial on startup)
3. Add these MAC addresses to the sender's peer list

## Configuration

- **Time Zone**: Configured for EST (-18000) with DST (3600) in `sender.ino`
- **Broadcast Interval**: Currently set to 1.5 seconds (adjustable in sender's loop delay)
- **NTP Server**: Using pool.ntp.org (configurable in sender)

## Serial Output Examples

### Sender Output

```
Sent to 5c:01:3b:51:2e:64: 2023-11-15 14:30:45
Sent to 5c:01:3b:4f:bd:d8: 2023-11-15 14:30:45 ...
|-------------------------------------------------|
```

### Receiver Output

```
Receiver MAC Address: 5C:01:3B:4F:BD:D8
Message received from: 5C:01:3B:51:2E:64
Bytes received: 24
Received Time: 2023-11-15 14:30:45
```

## Troubleshooting

1. **Connection Issues**:

   - Verify MAC addresses are correct
   - Ensure all devices are powered on
   - Check WiFi connectivity for sender

2. **Time Sync Problems**:

   - Verify NTP server is accessible
   - Check timezone settings

3. **ESP-NOW Errors**:
   - Ensure all devices are within range
   - Try reducing broadcast interval if packets are lost

## Customization

- Add more peers by extending the `peerAddresses` array
- Modify time format in both sender and receiver
- Add additional data to the message struct if needed

## License

This project is open-source. Feel free to modify and distribute.

## Future Enhancements

- Add acknowledgment mechanism for reliable delivery
- Implement time drift correction
- Add battery saving features for receiver devices
- Include error statistics and reporting
