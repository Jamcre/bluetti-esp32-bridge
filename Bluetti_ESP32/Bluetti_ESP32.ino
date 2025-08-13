#include "BWifi.h"
#include "BTooth.h"
#include "MQTT.h"
#include "config.h"
#include "display.h"
#include "batteryConversion.h"
#include <string.h>

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>

#include "ui.h"
#include "screens.h"

// --- (All your declarations remain the same) ---

unsigned long lastTime1 = 0;
unsigned long timerDelay1 = 3000;

// ... (All functions down to setup() remain exactly the same) ...

void updateArc(uint8_t battery_level) {
    if (battery_level > 100) battery_level = 100;
    lv_arc_set_value(objects.battery_arc, battery_level);
}

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;
#define TFT_HOR_RES 320
#define TFT_VER_RES 240
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  lv_disp_flush_ready(disp);
}
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    if (p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
    if (p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
    if (p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
    if (p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;
    data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 1, TFT_HOR_RES);
    data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 1, TFT_VER_RES);
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

lv_indev_t *indev;
uint8_t *draw_buf;
uint32_t lastTick = 0;
const int ledPin22 = 22;

void setup() {

  String LVGL_Arduino = "LVGL demo ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  delay(2000); // Add a small delay to ensure the serial monitor can connect
  Serial.println(LVGL_Arduino);

  touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSpi);
  touchscreen.setRotation(3);

  lv_init();
  draw_buf = new uint8_t[DRAW_BUF_SIZE];
  lv_display_t *disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
  Serial.println("LVGL Setup done");
  ui_init();
  pinMode(ledPin22, OUTPUT);

  #ifdef RELAISMODE
    pinMode(RELAIS_PIN, OUTPUT);
    #ifdef DEBUG
      Serial.println(F("deactivate relais contact"));
    #endif
    digitalWrite(RELAIS_PIN, RELAIS_LOW);
  #endif
  #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_ON_BT_NOT_AVAIL * 60 * 1000000ULL);
  #endif
  #ifdef DISPLAYSSD1306
    initDisplay();
  #endif

  // --- MINIMAL CHANGE SECTION ---
  // The order of these next three calls is CRITICAL for TLS to work.

  // 1. Connect to WiFi first.
  initBWifi(false);

  // 2. Sync time now that we have internet.
  syncNtpTime();
  
  // 3. Now initialize MQTT.
  initMQTT();

  // 4. Initialize Bluetooth last.
  // (It's a blocking call and should not prevent network setup)
  initBluetooth();

  // --- END OF CHANGE SECTION ---

  #ifdef DISPLAYSSD1306
    wrDisp_Status("Running!");
  #endif
}

static void arc_dimmer_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *arc = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    int brightness = lv_arc_get_value(arc);
    brightness = map(brightness, 0, 100, 0, 255);
    analogWrite(ledPin22, brightness);
    lv_label_set_text_fmt(label, "%d", brightness);
    lv_obj_set_style_bg_opa(objects.led1, brightness, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void syncNtpTime() {
    configTime(0, 0, "pool.ntp.org");
    Serial.print("Waiting for NTP time sync... ");
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 15000)) { // 15 second timeout
        Serial.println(" FAILED. TLS connections may fail.");
        return;
    }
    Serial.println(" SUCCESS.");
    Serial.println(&timeinfo, "Current time: %A, %B %d %Y %H:%M:%S");
}

void loop() {
  const char *batPercentage = getBatteryPercent();
  const char *AC_InputPower = getAC_Input();
  const char *AC_OutputPower = getAC_Output();
  const char *DC_InputPower = getDC_Input();
  const char *DC_OutputPower = getDC_Output();
  const char *AC_Stats = getAC_Status();
  const char *DC_Stats = getDC_Status();

  if (batPercentage != NULL && isdigit(*batPercentage)) {
    lv_label_set_text(objects.percentage, batPercentage);
    int8_t batPercentNum = atoi(batPercentage);
    updateArc(batPercentNum);
  }

  if (AC_InputPower != NULL && isdigit(*AC_InputPower)) {
    char result[100];
    strcpy(result,AC_InputPower);
    strcat(result," Watts");
    lv_label_set_text(objects.ac_input_num, result);
  }

  if (AC_OutputPower != NULL && isdigit(*AC_OutputPower)) {
    char result[100];
    strcpy(result,AC_OutputPower);
    strcat(result," Watts");
    lv_label_set_text(objects.ac_output_num, result);
  }

  if (DC_InputPower != NULL && isdigit(*DC_InputPower)) {
    char result[100];
    strcpy(result,DC_InputPower);
    strcat(result," Watts");
    lv_label_set_text(objects.dc_input_num, result);
  }

  if (DC_OutputPower != NULL && isdigit(*DC_OutputPower)) {
    char result[100];
    strcpy(result,DC_OutputPower);
    strcat(result," Watts");
    lv_label_set_text(objects.dc_output_num, result);
  }

  if (AC_Stats != NULL && isdigit(*AC_Stats)) {
    int offChecker = strchr(AC_Stats, '0') != NULL;
    if (offChecker == 0) {
      lv_obj_set_style_text_color(objects.ac_status, lv_color_hex(0xff00ff26), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      lv_obj_set_style_text_color(objects.ac_status, lv_color_hex(0xffc00000), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }

  if (DC_Stats != NULL && isdigit(*DC_Stats)) {
    int offChecker = strchr(DC_Stats, '0') != NULL;
    if (offChecker == 0) {
      lv_obj_set_style_text_color(objects.dc_status, lv_color_hex(0xff00ff26), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      lv_obj_set_style_text_color(objects.dc_status, lv_color_hex(0xffc00000), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }

  lv_tick_inc(millis() - lastTick);
  lastTick = millis();
  lv_timer_handler();

  #ifdef DISPLAYSSD1306
    //handleDisplay();
  #endif
  
  // --- MINIMAL CHANGE ---
  // Add this call to the loop. It handles reconnects and publishing.
  handleMQTT(); 

  handleBluetooth();
  handleWebserver();
  delay(5);
}