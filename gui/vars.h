#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_TIME_LEFT = 0,
    FLOW_GLOBAL_VARIABLE_AC_INPUT = 1,
    FLOW_GLOBAL_VARIABLE_DC_INPUT = 2,
    FLOW_GLOBAL_VARIABLE_AC_OUTPUT = 3,
    FLOW_GLOBAL_VARIABLE_DC_OUTPUT = 4,
    FLOW_GLOBAL_VARIABLE_AC_STATUS = 5,
    FLOW_GLOBAL_VARIABLE_DC_STATUS = 6
};

// Native global variables

extern int32_t get_var_time_left();
extern void set_var_time_left(int32_t value);
extern float get_var_ac_input();
extern void set_var_ac_input(float value);
extern float get_var_dc_input();
extern void set_var_dc_input(float value);
extern float get_var_ac_output();
extern void set_var_ac_output(float value);
extern float get_var_dc_output();
extern void set_var_dc_output(float value);
extern int32_t get_var_ac_status();
extern void set_var_ac_status(int32_t value);
extern int32_t get_var_dc_status();
extern void set_var_dc_status(int32_t value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/