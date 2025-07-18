#include <esp_now.h> //for NTP time
#include <Arduino.h>
#include <SensirionI2cSen66.h>
#include <Wire.h>

#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include <FS.h>
#include <SD.h>
#include "SPI.h"
#include <time.h>
#include <ESP32Time.h>

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include "ui.h"
#include "dataToCSV.h"
#include "message_struct.h"

// ---- Webpage Related values and HTML ------------

//const char *soft_ap_ssid = "IndoorModuleLab12"; //change according to each module'sIndoorModuleLabXX
const char *soft_ap_ssid = "IndoorModuleLab13"; //change according to each module'sIndoorModuleLabXX

//essentially the ESP 32 creates it's own Wifi, this is what shows up as the "hotspot id" 
const char *soft_ap_password = "IndoorModuleLab13";  //and this is the password

AsyncWebServer server(80);
String serialBuffer = "";

TFT_eSPI tft = TFT_eSPI();
// -------------------------------------------------

// ---- Sensor related values ---- 
SensirionI2cSen66 sen66;

#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

#define SDA_PIN 22
#define SCL_PIN 27

static char errorMessage[64];
static int16_t error;

float massConcentrationPm1p0 = 0.0;
float massConcentrationPm2p5 = 0.0;
float massConcentrationPm4p0 = 0.0;
float massConcentrationPm10p0 = 0.0;
float ambientHumidity = 0.0;
float humidity = 0.0;
float ambientTemperature = 0.0;
float temperature = 0.0;
float vocIndex = 0.0;
float noxIndex = 0.0;
uint16_t co2 = 0;

enum SensorValueType {
  PM1P0,
  PM2P5,
  PM4P0,
  PM10P0,
  HUMIDITY,
  TEMPERATURE,
  VOCINDEX,
  NOXINDEX,
  CO2
};
// ------------------------

// ---- Display related values ----
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

unsigned long lastUpdate = 0;
// --------------------------------

// ---- Color and Text Indicator values -----
lv_color_t primary_color;
lv_color_t secondary_color;
const char* indicator_text;

enum Indicator {
  GOOD,
  FAIR,
  BAD,
  POOR
};
// ------------------------------------------

// ---- Touchscreen related values ----

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(HSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;
// ------------------------------------

// ---- Wi-Fi related values ----
TaskHandle_t scanTaskHandler = NULL;
TaskHandle_t wiFiTaskHandle = NULL;

String savedSSID, savedPassword;
bool canScanAgain = true;
// ------------------------------

// ---- SD Card related values ----
#define SD_CS 5

ESP32Time rtc(0);
File myFile;
int countfn = 0;
bool canWriteAgain = true;
unsigned long lastTimeWrittenToSD = 0;
bool first_time = 0;
// --------------------------------

// ---- Time Synch related values ----
//struct has already been created in message_struct.h
message_struct myData;
message_struct myData_copy;
int stored_min = 0;
// -----------------------------------

String readSensorValue(SensorValueType type) {

  sen66.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, humidity, temperature, vocIndex, noxIndex,
    co2);

  switch (type) {
    case PM1P0:
      return String(massConcentrationPm1p0);
    case PM2P5:
      return String(massConcentrationPm2p5);
    case PM4P0:
      return String(massConcentrationPm4p0);
    case PM10P0:
      return String(massConcentrationPm10p0);
    case HUMIDITY:
      return String(humidity);
    case TEMPERATURE:
      return String(temperature);
    case VOCINDEX:
      return String(vocIndex);
    case NOXINDEX:
      return String(noxIndex);
    case CO2:
      return String(co2);
    default:
      return String("Error");
  }
}

String processor(const String &str) {

  if (str == "PM1P0") {
    return readSensorValue(PM1P0);
  } else if (str == "PM2P5") {
    return readSensorValue(PM2P5);
  } else if (str == "PM4P0") {
    return readSensorValue(PM4P0);
  } else if (str == "PM10P0") {
    return readSensorValue(PM10P0);
  } else if (str == "AMBIENTHUMIDITY") {
    return readSensorValue(HUMIDITY);
  } else if (str == "AMBIENTTEMPERATURE") {
    return readSensorValue(TEMPERATURE);
  } else if (str == "VOCINDEX") {
    return readSensorValue(VOCINDEX);
  } else if (str == "NOXINDEX") {
    return readSensorValue(NOXINDEX);
  } else if (str == "CO2") {
    return readSensorValue(CO2);
  } else if (str == "SERIALOUTPUT") {
    return "serialBuffer";
  }
  return String("Value doesn't exist");
}

void updateIndicatorBasedOnThreshold(float current_value, float first_threshold, 
                                    float second_threshold, float third_threshold, float fourth_threshold){

  if(current_value >= first_threshold && current_value <= second_threshold){
      setColorAndTextBasedOnIndicator(GOOD);
    }
    else if(current_value > second_threshold && current_value <= third_threshold){
      setColorAndTextBasedOnIndicator(FAIR);
    }
    else if(current_value > third_threshold && current_value <= fourth_threshold){
      setColorAndTextBasedOnIndicator(BAD);
    }
    else if(current_value > fourth_threshold){
      setColorAndTextBasedOnIndicator(POOR);
    }
}

void updateArcWidget(lv_obj_t * ui_arc, float sensor_value, lv_color_t primary_color, lv_color_t secondary_color){
  lv_arc_set_value(ui_arc, sensor_value);
  lv_obj_set_style_arc_color(ui_arc, primary_color, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_arc, secondary_color, LV_PART_KNOB | LV_STATE_DEFAULT);
}

void updateTextWidget(lv_obj_t * ui_state_text, const char* state_text, lv_obj_t * ui_value_text,
                      const char* value_text, lv_obj_t * ui_label_text, lv_color_t primary_color){
  lv_label_set_text(ui_state_text, state_text);
  lv_obj_set_style_text_color(ui_state_text, primary_color, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_label_set_text(ui_value_text, value_text);
  lv_obj_set_style_text_color(ui_value_text, primary_color, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_text_color(ui_label_text, primary_color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void setColorAndTextBasedOnIndicator(Indicator indicator){
  switch(indicator){
    case GOOD:
      primary_color = lv_color_hex(0x0DB863);
      secondary_color = lv_color_hex(0x84D4AC);
      indicator_text = "GOOD";
      return;
    case FAIR:
      primary_color = lv_color_hex(0xDADA12);
      secondary_color = lv_color_hex(0xDDDDA5);
      indicator_text = "FAIR";
      return;
    case BAD:
      primary_color = lv_color_hex(0xEB8A16);
      secondary_color = lv_color_hex(0xE7B46A);
      indicator_text = "BAD";
      return;
    case POOR:
      primary_color = lv_color_hex(0xDF1919);
      secondary_color = lv_color_hex(0xD48484);
      indicator_text = "POOR";
      return;
    default:
      primary_color = lv_color_hex(0x000000);
      secondary_color = lv_color_hex(0x000000);
      indicator_text = "NULL";
      return;
  }
}

void updateSensorValuesOnDisplay(lv_timer_t *timer){
  sen66.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, humidity, temperature, vocIndex, noxIndex,
    co2);

  if(lv_scr_act() == ui_pmScreen){
    Serial.print("PM 1.0: ");
    Serial.println(massConcentrationPm1p0);

    String pm10_str = String(massConcentrationPm1p0);
    const char* pm10_chars = pm10_str.c_str();

    String pm25_str = String(massConcentrationPm2p5);
    const char* pm25_chars = pm25_str.c_str();

    String pm40_str = String(massConcentrationPm4p0);
    const char* pm40_chars = pm40_str.c_str();

    String pm100_str = String(massConcentrationPm10p0);
    const char* pm100_chars = pm100_str.c_str();


    // ---- Updating PM 1.0 gauge ----
        updateIndicatorBasedOnThreshold(massConcentrationPm1p0, 0, 12, 35, 56);
        updateArcWidget(ui_pm10Arc, massConcentrationPm1p0, primary_color, secondary_color);
        updateTextWidget(ui_pm10StateText, indicator_text, ui_pm10ValueText, pm10_chars, ui_pm10LabelText, primary_color);
    // -------------------------------

    // ---- Updating PM 2.5 gauge ----
        updateIndicatorBasedOnThreshold(massConcentrationPm2p5, 0, 12, 35, 56);
        updateArcWidget(ui_pm25Arc, massConcentrationPm2p5, primary_color, secondary_color);
        updateTextWidget(ui_pm25StateText, indicator_text, ui_pm25ValueText, pm25_chars, ui_pm25LabelText, primary_color);
    // -------------------------------

    // ---- Updating PM 4.0 gauge ----
        updateIndicatorBasedOnThreshold(massConcentrationPm4p0, 0, 20, 40, 70);
        updateArcWidget(ui_pm40Arc, massConcentrationPm4p0, primary_color, secondary_color);
        updateTextWidget(ui_pm40StateText, indicator_text, ui_pm40ValueText, pm40_chars, ui_pm40LabelText, primary_color);
    // -------------------------------

    // ---- Updating PM 10.0 gauge ----
        updateIndicatorBasedOnThreshold(massConcentrationPm10p0, 0, 54, 154,254);
        updateArcWidget(ui_pm100Arc, massConcentrationPm10p0, primary_color, secondary_color);
        updateTextWidget(ui_pm100StateText, indicator_text, ui_pm100ValueText, pm100_chars, ui_pm100LabelText, primary_color);
    // -------------------------------
  } else if(lv_scr_act() == ui_tempScreen) {

    String temp_str = String(temperature);
    const char* temp_chars = temp_str.c_str();

    String hum_str = String(humidity);
    const char* hum_chars = hum_str.c_str();

    // ---- Updating Temperature gauge ----
        updateIndicatorBasedOnThreshold(temperature, 18, 20, 24, 26);
        updateArcWidget(ui_tempArc, temperature, primary_color, secondary_color);
        updateTextWidget(ui_tempStateText, indicator_text, ui_tempValueText, temp_chars, ui_tempLabelText, primary_color);
    // -------------------------------

    // ---- Updating Humidity gauge ----
        updateIndicatorBasedOnThreshold(humidity, 20, 30, 50, 70);
        updateArcWidget(ui_humArc, humidity, primary_color, secondary_color);
        updateTextWidget(ui_humStateText, indicator_text, ui_humValueText, hum_chars, ui_humLabelText, primary_color);
    // -------------------------------
  } else if(lv_scr_act() == ui_vocScreen) {

    String voc_str = String((int)vocIndex);
    const char* voc_chars = voc_str.c_str();

    String nox_str = String((int)noxIndex);
    const char* nox_chars = nox_str.c_str();


    // ---- Updating VOC gauge ----
        updateIndicatorBasedOnThreshold(vocIndex, 0, 100, 250, 450);
        updateArcWidget(ui_vocArc, vocIndex, primary_color, secondary_color);
        updateTextWidget(ui_vocStateText, indicator_text, ui_vocValueText, voc_chars, ui_vocLabelText, primary_color);
    // -------------------------------

    // ---- Updating NOx gauge ----
        updateIndicatorBasedOnThreshold(noxIndex, 0, 100, 250, 450);
        updateArcWidget(ui_noxArc, noxIndex, primary_color, secondary_color);
        updateTextWidget(ui_noxStateText, indicator_text, ui_noxValueText, nox_chars, ui_noxLabelText, primary_color);
    // -------------------------------
  } else if(lv_scr_act() == ui_co2Screen) {

    String co2_str = String(co2);
    const char* co2_chars = co2_str.c_str();

    // ---- Updating CO2 gauge ----
        updateIndicatorBasedOnThreshold(co2, 400, 650, 900, 1200);
        updateArcWidget(ui_CO2Arc, co2, primary_color, secondary_color);
        updateTextWidget(ui_CO2StateText, indicator_text, ui_CO2ValueText, co2_chars, ui_CO2LabelText, primary_color);
    // -------------------------------
  }
}

void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;

    // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
    /* Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Pressure = ");
    Serial.print(z);
    Serial.println();*/
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ---- WiFi Selection and Scanning Functions ----

  void setWiFiToDropdown(lv_event_t * e) {
    char buf[32];
    lv_dropdown_get_selected_str(ui_WiFiSelectionScreenDropdown, buf, sizeof(buf));

    savedSSID = String(buf);
    savedSSID.trim();
    lv_dropdown_set_text(ui_WiFiSelectionScreenDropdown, savedSSID.c_str());

    vTaskDelete(scanTaskHandler);
    vTaskDelay(100);
      
    canScanAgain = true;
    lastUpdate = millis();

  }

  void scanWiFiTask(void *pvParameters) {  
    vTaskDelay(500); 
    while (true) {        
      int n = WiFi.scanNetworks();
      if (n <= 0 && WiFi.scanComplete()) {
        lv_dropdown_add_option(ui_WiFiSelectionScreenDropdown, "No networks found", LV_DROPDOWN_POS_LAST);     
      } else if(WiFi.scanComplete()) {
        lv_dropdown_clear_options(ui_WiFiSelectionScreenDropdown); 
        vTaskDelay(5);
        for (int i = 0; i < n; ++i) {
          lv_dropdown_add_option(ui_WiFiSelectionScreenDropdown, WiFi.SSID(i).c_str(), LV_DROPDOWN_POS_LAST);
          vTaskDelay(10);
        }                  
      }

      WiFi.scanDelete();
      vTaskDelay(10000); 
    } 
  }

  void networkScanner(){
    xTaskCreate(
      connectToWiFiTask,   // WiFi Task
      "ConnectToWiFiTask",    
      4048,                // RAM For task (4KB)
      NULL,                // Input parameter (SSID)
      1,                   // Priority (1 = low)
      &wiFiTaskHandle     
    );
  }

// -----------------------------------------------

void assignPwd(){
  char wifi_password[34] = {0};
  snprintf(wifi_password, sizeof(wifi_password), "%s", lv_textarea_get_text(ui_WiFiSelectionScreenPwdInputTextArea));
  savedPassword = String(wifi_password);
}

void connectToWiFiTask(void *parameter) {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

  //If after 10 seconds (20 iterations x 500ms) cannot connect, stop.
  int timeout = 20;
  while ( WiFi.status() != WL_CONNECTED && timeout > 0 ) {
    vTaskDelay(500);
    Serial.print(".__.");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.println(savedSSID);
    Serial.println(savedPassword);

    _ui_flag_modify(ui_LoadingPanelWiFiSelectionScreen, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
    _ui_flag_modify(ui_ConnectedWiFiPanelWiFiSelectionScreen, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
    _ui_flag_modify(ui_ForgetWiFiButtonWiFiSelectionScreen, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
    lv_label_set_text(ui_ConnectedWiFiNameLabelWiFiSelectionScreen, savedSSID.c_str());
  } else {
    Serial.println("\nConnect fail! Please check SSID & Password.");
    Serial.println(savedSSID);
    Serial.println(savedPassword);
    _ui_flag_modify(ui_FailedConnectionPanelWiFiSelectionScreen, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
    vTaskDelay(2000);
    _ui_flag_modify(ui_FailedConnectionPanelWiFiSelectionScreen, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);

    //just turn off WiFi and end the task
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

    canScanAgain = true;
    lastUpdate = millis();

    wiFiTaskHandle = NULL;
    vTaskDelete(NULL);  
}

void tryConnectingToWiFi(){
  assignPwd();

  if (wiFiTaskHandle != NULL) return;

  canScanAgain = false;
  vTaskDelete(scanTaskHandler);
  vTaskDelay(500);

  const char* ssid = savedSSID.c_str();
  if (*ssid) {
    xTaskCreate(
        connectToWiFiTask,   // WiFi Task
        "ConnectToWiFiTask",    
        4048,                // RAM For task (4KB)
        NULL,                // Input parameter (SSID)
        1,                   // Priority (1 = low)
        &wiFiTaskHandle     
      );
  }
  
}

// -------- SD Card Functions ---------
// moved to dataToCSV .cpp and .h!!

// -------- Time Sync Related Functions -------
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

      // ----
      rtc.setTime(myData.second, myData.minute, myData.hour, myData.day, myData.month, myData.year);  //-N- Automatically SETTING REAL TIME CLOCK
      Serial.println(myData.hour); //-N- Printing the time
    } else {
      Serial.print("Unexpected data length: ");
      Serial.println(len);
    }
  }
// ---------------------------------------------

// --
void printAndBuffer(String message, bool newline=true) {
  if (newline) {
    Serial.println(message);
    serialBuffer += message + "\n";
  } else {
    Serial.print(message);
    serialBuffer += message;
  }

while (serialBuffer.length() > 2000) {  // Adjust the size as necessary
    int nextLineBreak = serialBuffer.indexOf('\n') + 1;
    if (nextLineBreak > 0) {
      serialBuffer = serialBuffer.substring(nextLineBreak);
    } else {
      serialBuffer = serialBuffer.substring(100);  // Default trimming
    }
  }
}

void appendToBuffer(char c) {
    serialBuffer += c;
    if (serialBuffer.length() > 2000) {
        serialBuffer = "";  // Clear buffer when size limit is reached
    }
}
String readScdCo2() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(co2);
}
String readPm1p0() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(massConcentrationPm1p0);
}

String readPm2p5() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(massConcentrationPm2p5);
}

String readPm4p0() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(massConcentrationPm4p0);
}

String readPm10p0() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(massConcentrationPm10p0);
}

String readAmbientHumidity() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(ambientHumidity);
}

String readAmbientTemperature() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(ambientTemperature);
}

String readVocIndex() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(vocIndex);
}

String readNoxIndex() {
    
    sen66.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex, co2);
    
    return String(noxIndex);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://kit.fontawesome.com/d91e44f906.js" crossorigin="anonymous"></script>
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Ecolibrium IAQ 13</h2>
  <p>
    <i class="fas fa-smog" style="color:#000000;"></i> 
    <span class="dht-labels">PM 1.0</span>
    <span id="pm1p0">%PM1P0%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#a0a0a0;"></i>
    <span class="dht-labels">PM 2.5</span>
    <span id="pm2p5">%PM2P5%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#afafaf;"></i>
    <span class="dht-labels">PM 4.0</span>
    <span id="pm4p0">%PM4P0%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#f0f0f0;"></i> 
    <span class="dht-labels">PM 10.0</span>
    <span id="pm10p0">%PM10P0%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-project-diagram" style="color:#631fc4;"></i> 
    <span class="dht-labels">Carbon Dioxide</span>
    <span id="co2">%CO2%</span>
    <sup class="units">ppm</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Ambient Humidity</span>
    <span id="ambienthumidity">%AMBIENTHUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#bada55;"></i>
    <span class="dht-labels">Ambient Temperature</span>
    <span id="ambienttemperature">%AMBIENTTEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-lungs" style="color:#ffb6c1;"></i> 
    <span class="dht-labels">Voc Index</span>
    <span id="vocindex">%VOCINDEX%</span>
    <sup class="units">-</sup>
  </p>
  <p>
    <i class="fas fa-grin-squint-tears" style="color:#9b870c;"></i> 
    <span class="dht-labels">Nox Index</span>
    <span id="noxindex">%NOXINDEX%</span>
    <sup class="units">-</sup>
  </p>
  <p>
  <i class="fas fa-terminal" style="color:#ff6600;"></i>
  <span class="dht-labels">Serial Monitor Output:</span>
</p>
<div style="height:300px; overflow:auto; border:1px solid #ccc; padding:5px;">
  <pre id="serialOutput">%SERIALOUTPUT%</pre>
</div>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pm1p0").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm1p0", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pm2p5").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm2p5", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pm4p0").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm4p0", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pm10p0").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pm10p0", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("co2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/co2", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ambienthumidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ambienthumidity", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ambienttemperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ambienttemperature", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("vocindex").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/vocindex", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("noxindex").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/noxindex", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("serialOutput").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/serial", true);
  xhttp.send();
}, 5000 );

</script>
</html>)rawliteral";


void setup() {

// ---- Serial Setup ----
  Serial.begin(115200);
  while (!Serial) {
    Serial.print(".");
    delay(100);
  }
// ----------------------

// ----- Sensor Setup -----
  Wire.begin(SDA_PIN, SCL_PIN);
  sen66.begin(Wire, SEN66_I2C_ADDR_6B);
  WiFi.mode(WIFI_STA);  // Set device as Wi-Fi Station
  error = sen66.deviceReset();
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute deviceReset(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }
  delay(1200);
  int8_t serialNumber[32] = { 0 };
  error = sen66.getSerialNumber(serialNumber, 32);
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }
  Serial.print("serialNumber: ");
  Serial.print((const char *)serialNumber);
  Serial.println();
  error = sen66.startContinuousMeasurement();
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute startContinuousMeasurement(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }
// ------------------------

// ---- LVGL and Touchscreen Setup ----

  lv_init();

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);

  // Set the Touchscreen rotation in landscape mode
  touchscreen.setRotation(3);
  lv_display_t *disp;

  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);
    
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);

  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);
  ui_init();
  lv_timer_t* updateDisplay = lv_timer_create(updateSensorValuesOnDisplay, 2000, NULL);

// ------------------------------------

// ---- RTC and SD Setup ----

  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  int cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %u\n", cardSize);
  Serial.printf("Total space: %fMB\n", SD.totalBytes() / (1024.0 * 1024.0));
  Serial.printf("Used space: %fMB\n", SD.usedBytes() / (1024.0 * 1024.0));

  int findMax();
  countfn = findMax() + 1;

// --------------------------
  //networkScanner();
  lastUpdate = millis();
  lastTimeWrittenToSD = millis();

  // ---- Webserver Setup
  WiFi.mode(WIFI_AP_STA);
  WiFiManager wm;   // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around

  bool res;

  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
    printAndBuffer("Failed to connect");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    printAndBuffer("connected...yeey :)");
  }
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  //start sensors
  printAndBuffer("ESP32 IP as soft AP: ");
  printAndBuffer(WiFi.softAPIP().toString());

  printAndBuffer("Local network server:");
  printAndBuffer("http://",false);
  printAndBuffer(WiFi.localIP().toString());

//_____________ Webpage Code ______________________ 
 
  // Route for web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/html", index_html, processor);
          return;
      }
    });
  server.on("/pm1p0", HTTP_GET, [](AsyncWebServerRequest *request){
      if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readPm1p0().c_str());
          return;
      }
    });
  server.on("/pm2p5", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readPm2p5().c_str());
          return;
    }
  });

  server.on("/pm4p0", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readPm4p0().c_str());
          return;
    }
  });

  server.on("/pm10p0", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readPm10p0().c_str());
          return;
    }
  });

  server.on("/co2", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readScdCo2().c_str());
          return;
    }
  });

  server.on("/ambienthumidity", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readAmbientHumidity().c_str());
          return;
    }
  });

  server.on("/ambienttemperature", HTTP_GET, [](AsyncWebServerRequest *request){
        if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readAmbientTemperature().c_str());
          return;
    }
  });

    server.on("/vocindex", HTTP_GET, [](AsyncWebServerRequest *request){
          if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readVocIndex().c_str());
          return;
    }
  });

    server.on("/noxindex", HTTP_GET, [](AsyncWebServerRequest *request){
          if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", readNoxIndex().c_str());
          return;
    }  
  });
    server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request){
          if (ON_STA_FILTER(request) || ON_AP_FILTER(request)) {
          request->send_P(200, "text/plain", serialBuffer.c_str());
          return;
    }
  });

  // Start server
  server.begin();
  printAndBuffer("Paste into web browser:");
  printAndBuffer("http://", false); // false indicates that we're not adding a newline
  printAndBuffer(WiFi.localIP().toString());

}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);

  if (lv_scr_act() == ui_wifiSelectionScreen && canScanAgain && millis() - lastUpdate > 10000) {
    canScanAgain = false;
    networkScanner();
    lastUpdate = millis();
  }

  if(canWriteAgain && millis() - lastTimeWrittenToSD > 3000){ //-N- set to 3000 again please!
    canWriteAgain = false;
    dataToCSV(countfn);
    lastTimeWrittenToSD = millis();
  }
}