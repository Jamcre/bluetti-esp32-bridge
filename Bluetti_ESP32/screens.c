#include <string.h>
#include <stdio.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"
#include <lvgl.h>

int32_t time_left = 20;

int32_t get_var_time_left() {
    return time_left;
}

void set_var_time_left(int32_t value) {
    time_left = value;
}

void goToScreenTwo(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        loadScreen(SCREEN_ID_SCREEN2);
    }
}

void goToMainMenu(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        loadScreen(SCREEN_ID_MAIN);
    }
}

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 53, 13);
            lv_obj_set_size(obj, 215, 215);
            lv_arc_set_value(obj, 2525);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM);
            lv_obj_set_style_arc_width(obj, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_rounded(obj, false, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_rounded(obj, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xff2122f3), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 20, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_scale_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 74, 32);
            lv_obj_set_size(obj, 175, 175);
            lv_scale_set_mode(obj, LV_SCALE_MODE_ROUND_INNER);
            lv_scale_set_range(obj, 0, 100);
            lv_scale_set_total_tick_count(obj, 26);
            lv_scale_set_major_tick_every(obj, 5);
            lv_scale_set_label_show(obj, true);
            lv_obj_set_style_length(obj, 5, LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xffdb0b0b), LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_length(obj, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xff05eed3), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff05eed3), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
        {
            // led1
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.led1 = obj;
            lv_obj_set_pos(obj, 121, 80);
            lv_obj_set_size(obj, 80, 80);
            lv_led_set_color(obj, lv_color_hex(0xff0000ff));
            lv_led_set_brightness(obj, 255);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_win_create(parent_obj);
            lv_obj_set_pos(obj, 64, 199);
            lv_obj_set_size(obj, 195, 32);
        }
        {
            //percentage
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 138, 100);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff03fbde), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_label_set_text(obj, "N/A"); //to be changed to variable
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 105, 207);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Time Left:");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 176, 207);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
        }
        {
            // screenTwo
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.screen_two = obj;
            lv_obj_set_pos(obj, 229, 7);
            lv_obj_set_size(obj, 78, 25);
            //lv_obj_add_event_cb(obj, goToScreenTwo, LV_EVENT_PRESSED, (void *)0);
            lv_obj_add_event_cb(obj, goToScreenTwo, LV_EVENT_ALL, NULL);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Screen 2");
                }
            }
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    {
        //const char *new_val = get_var_time_left();

        int32_t time_left = get_var_time_left();
        static char buffer[16];  // Make sure the buffer is large enough
        snprintf(buffer, sizeof(buffer), "%ld", time_left);
        const char *new_val = buffer;

        const char *cur_val = lv_label_get_text(objects.obj3);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.obj3;
            lv_label_set_text(objects.obj3, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_screen2() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.screen2 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // mainScreen
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_screen = obj;
            lv_obj_set_pos(obj, 233, 11);
            lv_obj_set_size(obj, 77, 30);
            //lv_obj_add_event_cb(obj, goToMainMenu, LV_EVENT_PRESSED, (void *)0);
            lv_obj_add_event_cb(obj, goToMainMenu, LV_EVENT_ALL, NULL);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Main Menu");
                }
            }
        }
    {
      // table_data
      lv_obj_t *obj = lv_table_create(parent_obj);
      objects.table_data = obj;
      lv_obj_set_pos(obj, 11, 55);
      lv_obj_set_size(obj, 299, 168);
    lv_table_set_col_cnt(objects.table_data, 7); // 7 columns
    lv_table_set_row_cnt(objects.table_data, 2); // 2 rows

    // Header row (row 0)
    lv_table_set_cell_value(objects.table_data, 0, 0, "A");
    lv_table_set_cell_value(objects.table_data, 0, 1, "B");
    lv_table_set_cell_value(objects.table_data, 0, 2, "C");
    lv_table_set_cell_value(objects.table_data, 0, 3, "D");
    lv_table_set_cell_value(objects.table_data, 0, 4, "E");
    lv_table_set_cell_value(objects.table_data, 0, 5, "F");
    lv_table_set_cell_value(objects.table_data, 0, 6, "G");

    // Data row (row 1)
    lv_table_set_cell_value(objects.table_data, 1, 0, "10");
    lv_table_set_cell_value(objects.table_data, 1, 1, "20");
    lv_table_set_cell_value(objects.table_data, 1, 2, "30");
    lv_table_set_cell_value(objects.table_data, 1, 3, "40");
    lv_table_set_cell_value(objects.table_data, 1, 4, "50");
    lv_table_set_cell_value(objects.table_data, 1, 5, "60");
    lv_table_set_cell_value(objects.table_data, 1, 6, "70");
    }
    }
    
    tick_screen_screen2();
}

void tick_screen_screen2() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_screen2,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_screen2();
}