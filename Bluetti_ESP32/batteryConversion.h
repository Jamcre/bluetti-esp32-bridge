#ifndef BATTERYCONVERSION_H
#define BATTERYCONVERSION_H

#ifdef __cplusplus
extern "C" {
#endif

    extern const char *getBatteryPercent();
    extern const char *getAC_Input();
    extern const char *getAC_Output();
    extern const char *getDC_Input();
    extern const char *getDC_Output();
    extern const char *getAC_Status();
    extern const char *getDC_Status();

#ifdef __cplusplus
}

#endif

#endif