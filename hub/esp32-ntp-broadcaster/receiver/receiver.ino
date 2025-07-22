#include <esp_now.h>
#include <WiFi.h>

// Define the structure to receive
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
    // Copy incoming data into our struct
    memcpy(&myData, incomingData, sizeof(myData));

    // Convert MAC address to readable string
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
             recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);

    // Print sender info
    Serial.print("Message received from: ");
    Serial.println(macStr);
    Serial.print("Bytes received: ");
    Serial.println(len);

    // Print full time in one line
    Serial.print("Received Time: ");
    Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                  myData.year, myData.month, myData.day,
                  myData.hour, myData.minute, myData.second);
    Serial.println();

  } else {
    Serial.print("Unexpected data length: ");
    Serial.println(len);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // Set device as Wi-Fi Station

  // Print this device's MAC address
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Nothing to do here, just wait for incoming messages
}
