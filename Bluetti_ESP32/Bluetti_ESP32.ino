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

unsigned long lastTime1 = 0;
unsigned long timerDelay1 = 3000;

void updateArc(uint8_t battery_level) {
    if (battery_level > 100) battery_level = 100;
    lv_arc_set_value(objects.battery_arc, battery_level);
}

// A library for interfacing with the touch screen
//
// Can be installed from the library manager (Search for "XPT2046")
//https://github.com/PaulStoffregen/XPT2046_Touchscreen
// ----------------------------
// Touch Screen pins
// ----------------------------

// The CYD touch uses some non default
// SPI pins

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

/*Set to your screen resolution*/
#define TFT_HOR_RES 320
#define TFT_VER_RES 240

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  /*Call it to tell LVGL you are ready*/
  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    //Some very basic auto calibration so it doesn't go out of range
    if (p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
    if (p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
    if (p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
    if (p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;
    //Map this to the pixel position
    data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 1, TFT_HOR_RES); /* Touchscreen X calibration */
    data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 1, TFT_VER_RES); /* Touchscreen Y calibration */
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

lv_indev_t *indev;      //Touchscreen input device
uint8_t *draw_buf;      //draw_buf is allocated on heap otherwise the static area is too big on ESP32 at compile
uint32_t lastTick = 0;  //Used to track the tick timer

//CN1 Extended IO PIN 3.3v(wire red) -> 27(wire yellow) -> 22(wire blue) -> GND(black)
//P3 Extended IO PIN 21(red - Can't use as it's the backlight) -> 22(wire yellow) -> 35(wire blue) -> GND(black)
//https://randomnerdtutorials.com/esp32-cyd-lvgl-temperature-ds18b20/
//https://esp32io.com/tutorials/esp32-rgb-led
//pins and setups
const int ledPin22 = 22;  //

void setup() {

  //Some basic info on the Serial console
  String LVGL_Arduino = "LVGL demo ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  //Initialise the touchscreen
  touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
  touchscreen.begin(touchscreenSpi);                                         /* Touchscreen init */
  touchscreen.setRotation(3);                                                /* Inverted landscape orientation to match screen */

  //Initialise LVGL GUI
  lv_init();

  draw_buf = new uint8_t[DRAW_BUF_SIZE];
  lv_display_t *disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);

  //Initialize the XPT2046 input device driver
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  //Done
  Serial.println("LVGL Setup done");

  //Integrate EEZ Studio GUI
  ui_init();

  //Set pin modes
  //CN1 Extended IO PIN 3.3v(red) -> 27(yellow) -> 22(blue) -> GND(black)
  //P3 Extended IO PIN 21(red - also dims backlight) -> 22(wire yellow) -> 35(wire blue) -> GND(black)
  pinMode(ledPin22, OUTPUT);  // Sets it to OUTPUT

  //Register event handler
  //
  //lv_obj_add_event_cb(objects.arc_dimmer0to100, arc_dimmer_event_handler, LV_EVENT_VALUE_CHANGED, objects.label_dim_value);

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
  initBWifi(false);
  initBluetooth();
  // initMQTT();
  #ifdef DISPLAYSSD1306
    wrDisp_Status("Running!");
  #endif
  
}

static void arc_dimmer_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);              //Get the event code
  lv_obj_t *arc = (lv_obj_t *)lv_event_get_target(e);       //Switch that generated the event
  lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);  //Label or other UI elements we want to update (Optional)

  if (code == LV_EVENT_VALUE_CHANGED) {
    int brightness = lv_arc_get_value(arc);  // 0 to 100 so we need to map it 0 to 255
    brightness = map(brightness, 0, 100, 0, 255);
    analogWrite(ledPin22, brightness);
    lv_label_set_text_fmt(label, "%d", brightness);
    //Set the LED UI element to be more visible
    lv_obj_set_style_bg_opa(objects.led1, brightness, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

//Requires LVGL 9.0+
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
    char result[100];   // array to hold the result.
    strcpy(result,AC_InputPower); // copy string one into the result.
    strcat(result," Watts"); // append string two to the result.

    lv_label_set_text(objects.ac_input_num, result);
  }

  if (AC_OutputPower != NULL && isdigit(*AC_OutputPower)) {
    char result[100];   // array to hold the result.
    strcpy(result,AC_OutputPower); // copy string one into the result.
    strcat(result," Watts"); // append string two to the result.
    
    lv_label_set_text(objects.ac_output_num, result);
  }

  if (DC_InputPower != NULL && isdigit(*DC_InputPower)) {
    char result[100];   // array to hold the result.
    strcpy(result,DC_InputPower); // copy string one into the result.
    strcat(result," Watts"); // append string two to the result.

    lv_label_set_text(objects.dc_input_num, result);
  }

  if (DC_OutputPower != NULL && isdigit(*DC_OutputPower)) {
    char result[100];   // array to hold the result.
    strcpy(result,DC_OutputPower); // copy string one into the result.
    strcat(result," Watts"); // append string two to the result.

    // lv_label_set_text(objects.dc_output_num, DC_OutputPower);
    lv_label_set_text(objects.dc_output_num, result);
  }

  if (AC_Stats != NULL && isdigit(*AC_Stats)) {
    int offChecker = strchr(AC_Stats, '0') != NULL;
    if (offChecker == 0) {
      lv_obj_set_style_text_color(objects.ac_status, lv_color_hex(0xff00ff26), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else {
      lv_obj_set_style_text_color(objects.ac_status, lv_color_hex(0xffc00000), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }

  if (DC_Stats != NULL && isdigit(*DC_Stats)) {
    int offChecker = strchr(DC_Stats, '0') != NULL;
    if (offChecker == 0) {
      lv_obj_set_style_text_color(objects.dc_status, lv_color_hex(0xff00ff26), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else {
      lv_obj_set_style_text_color(objects.dc_status, lv_color_hex(0xffc00000), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }

  //0xff00ff26 green
  //0xffc00000 red

  lv_tick_inc(millis() - lastTick);  //Update the tick timer. Tick is new for LVGL 9
  lastTick = millis();
  lv_timer_handler();  //Update the UI

  #ifdef DISPLAYSSD1306
    //handleDisplay();
  #endif
  handleBluetooth();
  //handleMQTT(); 
  handleWebserver();
  delay(5);
}