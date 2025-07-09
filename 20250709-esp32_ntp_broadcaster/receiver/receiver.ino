#include <esp_now.h>
#include <WiFi.h>

// Match this exactly with the sender's struct
typedef struct message_struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} message_struct;

message_struct myData;

// Callback function when data is received
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  if (len == sizeof(myData)) {
    // Copy the received data into the local struct
    memcpy(&myData, incomingData, sizeof(myData));

    // Convert MAC address to human-readable format
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
             recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);

    // Print sender and byte size info
    Serial.print("Message received from: ");
    Serial.println(macStr);
    Serial.print("Bytes received: ");
    Serial.println(len);

    // Print each struct member on a separate line
    Serial.println("Parsed Time Fields:");
    Serial.print("Year: ");
    Serial.println(myData.year);
    Serial.print("Month: ");
    Serial.println(myData.month);
    Serial.print("Day: ");
    Serial.println(myData.day);
    Serial.print("Hour: ");
    Serial.println(myData.hour);
    Serial.print("Minute: ");
    Serial.println(myData.minute);
    Serial.print("Second: ");
    Serial.println(myData.second);
    Serial.println();
  } else {
    Serial.print("Unexpected data length: ");
    Serial.println(len);
  }
}

void setup() {
  Serial.begin(115200);

  // Set WiFi to Station mode
  WiFi.mode(WIFI_STA);

  // Print this device's MAC address for pairing on sender side
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the receive callback
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Passive loop
}
