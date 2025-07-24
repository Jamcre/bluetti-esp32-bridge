#include "secrets.h"
#include <WiFi.h>
#include "time.h"
#include <esp_now.h>

// Peer MAC Addresses
uint8_t broadcastAddress1[] = {0x5c, 0x01, 0x3b, 0x51, 0x2e, 0x64};
uint8_t broadcastAddress2[] = {0x5c, 0x01, 0x3b, 0x4f, 0xbd, 0xd8}; // james
uint8_t broadcastAddress3[] = {0x5c, 0x01, 0x3b, 0x4f, 0x9f, 0xf0}; // nafis
uint8_t broadcastAddress4[] = {0x5c, 0x01, 0x3b, 0x4f, 0xf1, 0x78}; // sumit
uint8_t broadcastAddress5[] = {0x5c, 0x01, 0x3b, 0x4f, 0x19, 0x64}; // gaudi
uint8_t broadcastAddress6[] = {0x5c, 0x01, 0x3b, 0x4f, 0x8c, 0x80}; // alviee

uint8_t* peerAddresses[] = {
  broadcastAddress1,
  broadcastAddress2,
  broadcastAddress3,
  broadcastAddress4,
  broadcastAddress5,
  broadcastAddress6
};

// NTP Config
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;      // EST
const int   daylightOffset_sec = 3600;   // DST

// Time Struct
typedef struct message_struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} message_struct;

message_struct msg;

// Register all peers
void addPeer(uint8_t *address) {
  esp_now_peer_info_t pInfo = {};
  memcpy(pInfo.peer_addr, address, 6);
  pInfo.channel = 0;
  pInfo.encrypt = false;

  if (esp_now_add_peer(&pInfo) != ESP_OK) {
    Serial.print("Failed to add peer: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02x", address[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);

  // Dual Mode: AP + Station
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);       // Start AP
  WiFi.begin(sta_ssid, sta_password);      // Connect to WiFi

  // Wait for WiFi connection
  Serial.print("Connecting to ");
  Serial.println(sta_ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  // Time sync
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Optional: Register callback if you want async logging
  // esp_now_register_send_cb(OnDataSent);

  // Add all peers
  for (auto address : peerAddresses) {
    addPeer(address);
  }
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get local time");
    return;
  }

  // Fill message struct
  msg.year   = timeinfo.tm_year + 1900;
  msg.month  = timeinfo.tm_mon + 1;
  msg.day    = timeinfo.tm_mday;
  msg.hour   = timeinfo.tm_hour;
  msg.minute = timeinfo.tm_min;
  msg.second = timeinfo.tm_sec;

  // Send to each peer
  for (auto address : peerAddresses) {
    esp_err_t result = esp_now_send(address, (uint8_t*)&msg, sizeof(msg));
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             address[0], address[1], address[2], address[3], address[4], address[5]);

    if (result == ESP_OK) {
      Serial.printf("Sent to %s: %04d-%02d-%02d %02d:%02d:%02d\n",
                    macStr, msg.year, msg.month, msg.day,
                    msg.hour, msg.minute, msg.second);
    } else {
      Serial.printf("Failed to send to %s\n", macStr);
    }
  }

  Serial.println("|-------------------------------------------------|");
  delay(1500); // Send every 1.5 seconds
}
