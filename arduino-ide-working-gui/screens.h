#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *screen2;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *led1;
    lv_obj_t *obj2;
    lv_obj_t *screen_two;
    lv_obj_t *obj3;
    lv_obj_t *main_screen;
    lv_obj_t *table_data;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SCREEN2 = 2,
};

void create_screen_main();
void tick_screen_main();

void create_screen_screen2();
void tick_screen_screen2();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

void goToScreenTwo(lv_event_t *e);
void goToMainMenu(lv_event_t *e);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/