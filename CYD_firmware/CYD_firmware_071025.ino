
#include <Arduino.h>
#include <SensirionI2cSen66.h>
#include <Wire.h>

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include "ui.h"

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
  float humidity = 0.0;
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

  SPIClass touchscreenSPI = SPIClass(VSPI);
  XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

  // Touchscreen coordinates: (x, y) and pressure (z)
  int x, y, z;
// ------------------------------------

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

void updateIndicatorBasedOnThreshold(float current_value, float first_threshold, float second_threshold, float third_threshold, float fourth_threshold){
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

void updateTextWidget(lv_obj_t * ui_state_text, const char* state_text,  lv_obj_t * ui_value_text, const char* value_text, lv_obj_t * ui_label_text, lv_color_t primary_color){
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
  }
  else if(lv_scr_act() == ui_tempScreen){

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
  }
  else if(lv_scr_act() == ui_vocScreen){

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
  }
  else if(lv_scr_act() == ui_co2Screen){

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

  lastUpdate = millis();

}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);

}
