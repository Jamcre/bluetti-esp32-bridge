#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *battery_arc;
    lv_obj_t *battery_scale;
    lv_obj_t *led1;
    lv_obj_t *percentage;
    lv_obj_t *ac_input_num;
    lv_obj_t *dc_input_num;
    lv_obj_t *ac_output_text;
    lv_obj_t *dc_output_num;
    lv_obj_t *ac_output_num;
    lv_obj_t *ac_status;
    lv_obj_t *dc_status;
    lv_obj_t *dc_output_text;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/