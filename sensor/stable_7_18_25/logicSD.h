#ifndef LOGICSD_H
#define LOGICSD_H

// Libraries
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <ESP32Time.h>
#include <SensirionI2cSen66.h>
#include <esp_now.h>
#include <WiFi.h>
#include <time.h>
#include "message_struct.h"

// dataToCSV variable used
extern bool canWriteAgain;

// Sensor values
extern float massConcentrationPm1p0;
extern float massConcentrationPm2p5;
extern float massConcentrationPm4p0;
extern float massConcentrationPm10p0;
extern float humidity;
extern float temperature;
extern float vocIndex;
extern float noxIndex;
extern uint16_t co2;

// Sensor, RTC, SD, and custom struct dependencies
extern ESP32Time rtc;
extern message_struct myData;
extern message_struct myData_copy;
extern int countfn;
extern SensirionI2cSen66 sen66;

// Function
void dataToCSV(int cfn_input);

// Function dependencies
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len);

#endif