#include "secrets.h"
#include <WiFi.h>
#include "time.h"
#include <esp_now.h>

// MAC: 5c:01:3b:4f:8c:80
uint8_t broadcastAddress1[] = {0x5c, 0x01, 0x3b, 0x51, 0x2e, 0x64};
uint8_t broadcastAddress2[] = {0x5c, 0x01, 0x3b, 0x4f, 0xbd, 0xd8};
uint8_t broadcastAddress3[] = {0x5c, 0x01, 0x3b, 0x4f, 0x9f, 0xf0}; //nafis

// NTP Config
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;      // EST
const int   daylightOffset_sec = 3600;   // DST

// Structured message format
typedef struct message_struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} message_struct;

message_struct msg;
esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);

  // Dual Mode Setup
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);      // Start AP
  WiFi.begin(sta_ssid, sta_password);     // Connect to external WiFi

  // Wait for connection
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

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

   //register second peer  
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  //register third peer  
  memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

}

void loop() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    msg.year   = timeinfo.tm_year + 1900;
    msg.month  = timeinfo.tm_mon + 1;
    msg.day    = timeinfo.tm_mday;
    msg.hour   = timeinfo.tm_hour;
    msg.minute = timeinfo.tm_min;
    msg.second = timeinfo.tm_sec;
  } else {
    Serial.println("Failed to get local time");
    return;
  }

  // Send the message
  esp_err_t result = esp_now_send(0, (uint8_t*)&msg, sizeof(msg));

  if (result == ESP_OK) {
    Serial.printf("Sent time: %04d-%02d-%02d %02d:%02d:%02d\n",
                  msg.year, msg.month, msg.day,
                  msg.hour, msg.minute, msg.second);
  } else {
    Serial.println("Error sending the data");
  }

  // Print number of Soft AP clients
  Serial.print("Clients connected: ");
  Serial.println(WiFi.softAPgetStationNum());

  delay(500);  // send every 0.5 seconds
}

// ESP-NOW send status callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Packet to: ");
  Serial.print(macStr);
  Serial.print(" \t");
  
}

