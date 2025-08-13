#include "BluettiConfig.h"
#include "MQTT.h"
#include "BWifi.h"
#include "BTooth.h"
#include "utils.h"
#include "display.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiClientSecure.h> // <-- MODIFICATION: Include the secure client library
#include <PubSubClient.h>

//
// --- IMPORTANT: USER ACTION REQUIRED ---
// Replace TLS cerificate
//

// -- TLS Certificate
const char* mqtt_server_ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFFTCCAv2gAwIBAgIUIA4jcRF5tSDwihVltH0+iX11gO8wDQYJKoZIhvcNAQEL
BQAwGjEYMBYGA1UEAwwPTG9pc2FpZGFNUVRUX0NBMB4XDTI1MDgwNDE3MjQ1M1oX
DTMwMDgwMzE3MjQ1M1owGjEYMBYGA1UEAwwPTG9pc2FpZGFNUVRUX0NBMIICIjAN
BgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAtcW0f30HOzPeJsmgo+vl3bZ5UaM9
HRhkxIOHZiZ5Y4fcGkILTUxfq1iAW2l4MpUmJ4mWmiZ8N0sxC29kPmdGS+hXDv8L
yTlO04sIi7QXdS6uWHm1MInNTpkcO8nro5sEAL4oC9tNz1fEv4QFcO/yVryoElH/
vL4O2436AYGkfuwBdXOsMWYlnixijrypmSgds/fVh9u0pGv2rgbH5ib6/VLCudfz
3Yir0rPkFcmhdKnXPhFbW4fDwlnbr0xwvthKRCOsRQ33aMvRZwTRF/IVYAOBR2kE
aOsNak1XtTPcHGXRATh3YWtG3bBtNQkocYvsbzfHEtzVpj+Pkbp3oeOIzk7dBFCp
4fCFSxRUchr8/pA/IiZaVWZzFzyYs4/TWSOaBX8MDEnYgkgbToR1gV49Aqj+1DI7
2g3j0AE1Tacoz7ylweLctSMKmK55ggd8BI5fmYlhUIf3JMubPGvcSXvEQZFwJv32
BnXZMNWNr9TrMYHdv8wq5MzZvqxn/lteuHq8fKA5yhLviWkfPPkNoQhF3T61enzp
vHOt6pSeXuIAmaeEArU9oD+VH6T6ZYIPuyYyCBQUQa82KflwZPhQNMkaBdvGu8ut
BM/c9j4tCe2lQGYOTw/iRbzmusx21HeqsQI35KaXqrwcm5JiS2d7HHtjw59g86hR
Dm2KMvFGyHDWZUsCAwEAAaNTMFEwHQYDVR0OBBYEFIdsgJkihruKwyH6PPUxFlA7
suXMMB8GA1UdIwQYMBaAFIdsgJkihruKwyH6PPUxFlA7suXMMA8GA1UdEwEB/wQF
MAMBAf8wDQYJKoZIhvcNAQELBQADggIBAFbby1EUjNuONbzm9S+B/qYLzCidA2yw
4MIk4wIol2EpVGA5c9lLXa+wL8OkVLcNniW8K0gJjj4xMFvgC/ZHM8KFHoSPgIHc
j7bhGI9fXFQ8hOANtjPxmSQ+YnyH1TsEyrEldi8l6H8kEzRXFeRy2cGcK2HXzhuu
9YwjNxS85LiPziC6hE7yeEizmA04TdSSOt5rTBqKDF/kXgcVy4IAoPmJAt9z6mKH
9FmCcxmYULvnyznEogYbAZvYClS98PJJrv2Y07JJuQtA1d58G/XfBp4IAzqGk3fC
mTYcBG0uPXD88kF5qAEI4fRa6GWUilIxtWjKDo+6JnbY4cHbwF6tMwICnYXPHCcb
DSVyaHHzGiJNa9FsfvHyICYyDtlm04WSXgF5rhIwMlGxXUwF128WZ0kSUs1OCOEC
VFwd/kKkPrm07om8i74fX09uONbcV5A/XV2aUuLgGD8geNjWV/TcwoPHWhfaR0r7
X4A5Kss0DFOW3CN9PIJkeWLQiFFsEPIhpZqcceUuofdtx6UHScTQOSxG1J1YFqlj
8Sl8aXishDeeSJOteS4XSlhbD3kz1IleLaEEN9Rs4IreyYc2JVDu4fB+hCWEIl8i
BsDj5KjrNH+Zsvi2t4tYt+H/iMocAsJzmDI4rCWSArSqZq8wEe2leySq3oSVq+GG
Ygs/6kZxpwsG
-----END CERTIFICATE-----
)EOF";


// --- MODIFICATION: Use WiFiClientSecure instead of WiFiClient ---
WiFiClientSecure mqttSecureClient;
PubSubClient client(mqttSecureClient); // Pass the secure client to PubSubClient

int publishErrorCount = 0;
unsigned long lastMQTTMessage = 0;
unsigned long previousDeviceStatePublish = 0;
unsigned long previousDeviceStateStatusPublish = 0;
unsigned long previousMqttReconnect = 0;
String batteryPercent;
String AC_Input;
String AC_Output;
String DC_Input;
String DC_Output;
String AC_Status;
String DC_Status;

// New global variables for JSON payload aggregation
String jsonPayload = "{";
unsigned long lastDataPointTime = 0;

// --- END of declarations ---

// ADDED this global variable to track our 5-second publish interval.
unsigned long previousDataPublishTime = 0;


extern "C" const char* getBatteryPercent() {
  return batteryPercent.c_str();
}

extern "C" const char* getAC_Input() {
  return AC_Input.c_str();
}

extern "C" const char* getAC_Output() {
  return AC_Output.c_str();
}

extern "C" const char* getDC_Input() {
  return DC_Input.c_str();
}

extern "C" const char* getDC_Output() {
  return DC_Output.c_str();
}

extern "C" const char* getAC_Status() {
  return AC_Status.c_str();
}

extern "C" const char* getDC_Status() {
  return DC_Status.c_str();
}


String map_field_name(enum field_names f_name){
   switch(f_name) {
      case DC_OUTPUT_POWER: return "dc_output_power";
      case AC_OUTPUT_POWER: return "ac_output_power";
      case DC_OUTPUT_ON: return "dc_output_on";
      case AC_OUTPUT_ON: return "ac_output_on";
      case AC_OUTPUT_MODE: return "ac_output_mode";
      case POWER_GENERATION: return "power_generation";
      case TOTAL_BATTERY_PERCENT: return "total_battery_percent";
      case DC_INPUT_POWER: return "dc_input_power";
      case AC_INPUT_POWER: return "ac_input_power";
      case AC_INPUT_VOLTAGE: return "ac_input_voltage";
      case AC_INPUT_FREQUENCY: return "ac_input_frequency";
      case PACK_VOLTAGE: return "pack_voltage";
      case INTERNAL_PACK_VOLTAGE: return "internal_pack_voltage";
      case SERIAL_NUMBER: return "serial_number";
      case ARM_VERSION: return "arm_version";
      case DSP_VERSION: return "dsp_version";
      case DEVICE_TYPE: return "device_type";
      case UPS_MODE: return "ups_mode";
      case AUTO_SLEEP_MODE: return "auto_sleep_mode";
      case GRID_CHARGE_ON: return "grid_charge_on";
      case INTERNAL_AC_VOLTAGE: return "internal_ac_voltage";
      case INTERNAL_AC_FREQUENCY: return "internal_ac_frequency";
      case INTERNAL_CURRENT_ONE: return "internal_current_one";
      case INTERNAL_POWER_ONE: return "internal_power_one";
      case INTERNAL_CURRENT_TWO: return "internal_current_two";
      case INTERNAL_POWER_TWO: return "internal_power_two";
      case INTERNAL_CURRENT_THREE: return "internal_current_three";
      case INTERNAL_POWER_THREE: return "internal_power_three";
      case PACK_NUM_MAX: return "pack_max_num";
      case PACK_NUM: return "pack_num";
      case PACK_BATTERY_PERCENT: return "pack_battery_percent";
      case INTERNAL_DC_INPUT_VOLTAGE: return "internal_dc_input_voltage";
      case INTERNAL_DC_INPUT_POWER: return "internal_dc_input_power";
      case INTERNAL_DC_INPUT_CURRENT: return "internal_dc_input_current";
      case INTERNAL_CELL01_VOLTAGE: return "internal_cell01_voltage";
      case INTERNAL_CELL02_VOLTAGE: return "internal_cell02_voltage";
      case INTERNAL_CELL03_VOLTAGE: return "internal_cell03_voltage";
      case INTERNAL_CELL04_VOLTAGE: return "internal_cell04_voltage";
      case INTERNAL_CELL05_VOLTAGE: return "internal_cell05_voltage";
      case INTERNAL_CELL06_VOLTAGE: return "internal_cell06_voltage";
      case INTERNAL_CELL07_VOLTAGE: return "internal_cell07_voltage";
      case INTERNAL_CELL08_VOLTAGE: return "internal_cell08_voltage";
      case INTERNAL_CELL09_VOLTAGE: return "internal_cell09_voltage";
      case INTERNAL_CELL10_VOLTAGE: return "internal_cell10_voltage";
      case INTERNAL_CELL11_VOLTAGE: return "internal_cell11_voltage";
      case INTERNAL_CELL12_VOLTAGE: return "internal_cell12_voltage";
      case INTERNAL_CELL13_VOLTAGE: return "internal_cell13_voltage";
      case INTERNAL_CELL14_VOLTAGE: return "internal_cell14_voltage";
      case INTERNAL_CELL15_VOLTAGE: return "internal_cell15_voltage";
      case INTERNAL_CELL16_VOLTAGE: return "internal_cell16_voltage";
      case LED_MODE: return "led_mode";
      case POWER_OFF: return "power_off";
      case ECO_ON: return "eco_on";
      case ECO_SHUTDOWN: return "eco_shutdown";
      case CHARGING_MODE: return "charging_mode";
      case POWER_LIFTING_ON: return "power_lifting_on";
      case AC_INPUT_POWER_MAX: return "ac_input_power_max";
      case AC_INPUT_CURRENT_MAX: return "ac_input_current_max";
      case AC_OUTPUT_POWER_MAX: return "ac_output_power_max";
      case AC_OUTPUT_CURRENT_MAX: return "ac_output_current_max";
      case BATTERY_MIN_PERCENTAGE: return "battery_min_percentage";
      case AC_CHARGE_MAX_PERCENTAGE: return "ac_charge_max_percentage";
      default:
        #ifdef DEBUG
          Serial.println(F("Info 'map_field_name' found unknown field!"));
        #endif
        return "unknown";
   }
}

String map_command_value(String command_name, String value){
  String toRet = value;
  value.toUpperCase();
  command_name.toUpperCase();
  if(command_name == "POWER_OFF" || command_name == "AC_OUTPUT_ON" || command_name == "DC_OUTPUT_ON" || command_name == "ECO_ON" || command_name == "POWER_LIFTING_ON") {
    if (value == "ON") { toRet = "1"; }
    if (value == "OFF") { toRet = "0"; }
  }
  if(command_name == "LED_MODE"){
    if (value == "LED_LOW") { toRet = "1"; }
    if (value == "LED_HIGH") { toRet = "2"; }
    if (value == "LED_SOS") { toRet = "3"; }
    if (value == "LED_OFF") { toRet = "4"; }
  }
  if(command_name == "ECO_SHUTDOWN"){
    if (value == "ONE_HOUR") { toRet = "1"; }
    if (value == "TWO_HOURS") { toRet = "2"; }
    if (value == "THREE_HOURS") { toRet = "3"; }
    if (value == "FOUR_HOURS") { toRet = "4"; }
  }
  if(command_name == "CHARGING_MODE"){
    if (value == "STANDARD") { toRet = "0"; }
    if (value == "SILENT") { toRet = "1"; }
    if (value == "TURBO") { toRet = "2"; }
  }
  return toRet;
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topic_path = String(topic);
  topic_path.toLowerCase();
  Serial.print("MQTT Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(" Payload: ");
  String strPayload = String((char * ) payload);
  Serial.println(strPayload);
  bt_command_t command;
  command.prefix = 0x01;
  command.field_update_cmd = 0x06;
  for (int i=0; i< sizeof(bluetti_device_command)/sizeof(device_field_data_t); i++){
      if (topic_path.indexOf(map_field_name(bluetti_device_command[i].f_name)) > -1){
            command.page = bluetti_device_command[i].f_page;
            command.offset = bluetti_device_command[i].f_offset;
            String current_name = map_field_name(bluetti_device_command[i].f_name);
            strPayload = map_command_value(current_name,strPayload);
    }
  }
  Serial.print(" Payload - switched: ");
  Serial.println(strPayload);
  command.len = swap_bytes(strPayload.toInt());
  command.check_sum = modbus_crc((uint8_t*)&command,6);
  lastMQTTMessage = millis();
  sendBTCommand(command);
}

void subscribeTopic(enum field_names field_name) {
  #ifdef DEBUG
    Serial.println("[MQTT] subscribe to topic: " +  map_field_name(field_name));
  #endif
  char subscribeTopicBuf[512];
  ESPBluettiSettings settings = get_esp32_bluetti_settings();
  sprintf(subscribeTopicBuf, "bluetti/%s/command/%s", settings.bluetti_device_id, map_field_name(field_name).c_str() );
  client.subscribe(subscribeTopicBuf);
  lastMQTTMessage = millis();
}

String getFriendlyFieldName(enum field_names field) {
  switch (field) {
    case DEVICE_TYPE: return "Device Type";
    case SERIAL_NUMBER: return "Serial Number";
    case ARM_VERSION: return "ARM Version";
    case DSP_VERSION: return "DSP Version";
    case DC_INPUT_POWER: return "DC Input Power";
    case AC_INPUT_POWER: return "AC Input Power";
    case AC_OUTPUT_POWER: return "AC Output Power";
    case DC_OUTPUT_POWER: return "DC Output Power";
    case POWER_GENERATION: return "Power Generation";
    case TOTAL_BATTERY_PERCENT: return "Total Battery %";
    case AC_OUTPUT_ON: return "AC Output On";
    case DC_OUTPUT_ON: return "DC Output On";
    case AC_INPUT_VOLTAGE: return "AC Input Voltage";
    case INTERNAL_DC_INPUT_VOLTAGE: return "Internal DC Input Voltage";
    default: return map_field_name(field);
  }
}

// --- MODIFICATION ---
// This function's only job now is to update the global state variables.
// All JSON aggregation logic has been removed from here.
void publishTopic(enum field_names field_name, String value){
  String friendlyName = getFriendlyFieldName(field_name);
  
  // We can still print for debugging purposes
  Serial.print("[DATA] ");
  Serial.print(friendlyName);
  Serial.print(": ");
  Serial.println(value);

  // Update the global state variables. This is now the primary purpose.
  if (field_name == TOTAL_BATTERY_PERCENT) { batteryPercent = value; }
  if (field_name == AC_INPUT_POWER) { AC_Input = value; }
  if (field_name == DC_INPUT_POWER) { DC_Input = value; }
  if (field_name == AC_OUTPUT_POWER) { AC_Output = value; }
  if (field_name == DC_OUTPUT_POWER) { DC_Output = value; }
  if (field_name == AC_OUTPUT_ON) { AC_Status = value; }
  if (field_name == DC_OUTPUT_ON) { DC_Status = value; }

  AddtoMsgView(String(millis()) + ": " + friendlyName + " -> " + value);
}

//
// The functions publishDeviceState, publishDeviceStateStatus, and initMQTT are unchanged.
//
// ... (code for publishDeviceState, etc. remains the same) ...
//

void publishDeviceState(){
  char publishTopicBuf[1024];
  ESPBluettiSettings settings = get_esp32_bluetti_settings();
  sprintf(publishTopicBuf, "bluetti/%s/state/%s", settings.bluetti_device_id, "device" );
  String value = "{\"IP\":\"" + WiFi.localIP().toString() + "\", \"MAC\":\"" + WiFi.macAddress() + "\", \"Uptime\":" + millis() + "}";
  #ifdef DEBUG
    Serial.println("[MQTT] PublishingDeviceState: "+value);
  #endif
  if (!client.publish(publishTopicBuf, value.c_str() )){
    publishErrorCount++;
  }
  lastMQTTMessage = millis();
  previousDeviceStatePublish = millis();
}

void publishDeviceStateStatus(){
  char publishTopicBuf[1024];
  ESPBluettiSettings settings = get_esp32_bluetti_settings();
  sprintf(publishTopicBuf, "bluetti/%s/state/%s", settings.bluetti_device_id, "device_status" );
  String value = "{\"MQTTconnected\":" + String(isMQTTconnected()) + ", \"BTconnected\":" + String(isBTconnected()) + "}";
  #ifdef DEBUG
    Serial.println("[MQTT] PublishingDeviceStateStatus: "+value);
  #endif
  if (!client.publish(publishTopicBuf, value.c_str() )){
    publishErrorCount++;
  }
  lastMQTTMessage = millis();
  previousDeviceStateStatusPublish = millis();
}


void initMQTT(){
    ESPBluettiSettings settings = get_esp32_bluetti_settings();
    Serial.println("[MQTT] init MQTT");
    if (strlen(settings.mqtt_server) == 0){
      Serial.println("[MQTT] No MQTT server configured");
      return;
    }
    Serial.println(F("[MQTT] Configuring for TLS connection..."));
    mqttSecureClient.setCACert(mqtt_server_ca_cert);
    Serial.print("[MQTT] Connecting to MQTT at: ");
    Serial.print(settings.mqtt_server);
    Serial.print(":");
    Serial.println(settings.mqtt_port);
    client.setServer(settings.mqtt_server, atoi(settings.mqtt_port));
    client.setCallback(callback);
    bool connect_result;
    const char connect_id[] = "Bluetti_ESP32";
    if (settings.mqtt_username) {
        connect_result = client.connect(connect_id, settings.mqtt_username, settings.mqtt_password);
    } else {
        connect_result = client.connect(connect_id);
    }
    if (connect_result) {
      Serial.println(F("[MQTT] Connected to MQTT Server... "));
      for (int i=0; i< sizeof(bluetti_device_command)/sizeof(device_field_data_t); i++){
        subscribeTopic(bluetti_device_command[i].f_name);
      }
      publishDeviceState();
      publishDeviceStateStatus();
    } else {
      Serial.print(F("[MQTT] Connection failed! Error code: "));
      Serial.println(client.state());
      Serial.println(F("Check MQTT server, port, credentials, and CA certificate."));
    }
}

// This function is completely rewritten to use a simple 5-second timer.
void handleMQTT(){
    ESPBluettiSettings settings = get_esp32_bluetti_settings();
    if (strlen(settings.mqtt_server) == 0){
      return; // Do nothing if no server is configured
    }

    // First, handle reconnection logic so we don't try to publish if disconnected.
    if (!client.connected()) {
        long now = millis();
        if (now - previousMqttReconnect > 5000) { // Try to reconnect every 5 seconds
            previousMqttReconnect = now;
            Serial.println(F("[MQTT] Connection lost. Attempting to reconnect..."));
            initMQTT(); // Attempt to re-establish the connection
        }
        return; // Don't proceed further if not connected
    }

    // If connected, ensure the client loop runs
    client.loop();

    // Now, handle the periodic publishing every 5 seconds
    if (millis() - previousDataPublishTime > 5000) {
        previousDataPublishTime = millis(); // Reset the timer immediately

        // Build the JSON payload from scratch using our global state variables
        String payload = "{";
        if (batteryPercent.length() > 0) { payload += "\"total_battery_percent\":" + batteryPercent; }
        if (AC_Input.length() > 0) { payload += ",\"ac_input_power\":" + AC_Input; }
        if (DC_Input.length() > 0) { payload += ",\"dc_input_power\":" + DC_Input; }
        if (AC_Output.length() > 0) { payload += ",\"ac_output_power\":" + AC_Output; }
        if (DC_Output.length() > 0) { payload += ",\"dc_output_power\":" + DC_Output; }

        if (AC_Status.length() > 0) {
            payload += ",\"ac_output_on\":";
            payload += (AC_Status == "1" ? "true" : "false");
        }
        if (DC_Status.length() > 0) {
            payload += ",\"dc_output_on\":";
            payload += (DC_Status == "1" ? "true" : "false");
        }

        payload += "}";

        // Define the topic to publish to
        char publishTopicBuf[512];
        sprintf(publishTopicBuf, "bluetti/state/%s", settings.bluetti_device_id);

        #ifdef DEBUG
            Serial.print(F("[MQTT] Publishing state: "));
            Serial.println(payload);
        #endif

        // Publish the payload
        if (!client.publish(publishTopicBuf, payload.c_str(), true)) {
            publishErrorCount++;
        }
        lastMQTTMessage = millis();
    }

    // Handle other periodic status updates (this can be kept)
    if (millis() - previousDeviceStatePublish > (DEVICE_STATE_UPDATE * 60000)){
      publishDeviceState();
    }
    if ((millis() - previousDeviceStateStatusPublish > (DEVICE_STATE_STATUS_UPDATE * 60000))){
      publishDeviceStateStatus();
    }
  }

bool isMQTTconnected(){
    return client.connected();
}

int getPublishErrorCount(){
    return publishErrorCount;
}

unsigned long getLastMQTTMessageTime(){
    return lastMQTTMessage;
}

unsigned long getLastMQTTDeviceStateMessageTime(){
    return previousDeviceStatePublish;
}

unsigned long getLastMQTTDeviceStateStatusMessageTime(){
    return previousDeviceStateStatusPublish;
}