#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

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
            // battery arc
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.battery_arc = obj;
            lv_obj_set_pos(obj, 62, 32);
            lv_obj_set_size(obj, 211, 196);
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
            // battery scale
            lv_obj_t *obj = lv_scale_create(parent_obj);
            objects.battery_scale = obj;
            lv_obj_set_pos(obj, 82, 52);
            lv_obj_set_size(obj, 175, 156);
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
            lv_obj_set_pos(obj, 130, 100);
            lv_obj_set_size(obj, 61, 59);
            lv_led_set_color(obj, lv_color_hex(0xff0000ff));
            lv_led_set_brightness(obj, 255);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // percentage
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.percentage = obj;
            lv_obj_set_pos(obj, 0, 10);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff03fbde), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "N/A");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, -124, -104);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AC Input: \nDC Input:");
        }
        {
            // ac input num
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ac_input_num = obj;
            lv_obj_set_pos(obj, 70, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "N/A");
        }
        {
            // dc input num
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dc_input_num = obj;
            lv_obj_set_pos(obj, 70, 16);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "N/A");
        }
        {
            // ac output text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ac_output_text = obj;
            lv_obj_set_pos(obj, -114, 85);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AC Output:");
        }
        {
            // dc output num
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dc_output_num = obj;
            lv_obj_set_pos(obj, 114, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "N/A");
        }
        {
            // ac output num
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ac_output_num = obj;
            lv_obj_set_pos(obj, -113, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "N/A");
        }
        {
            // ac status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ac_status = obj;
            lv_obj_set_pos(obj, 43, 159);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffc00000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AC");
        }
        {
            // dc status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dc_status = obj;
            lv_obj_set_pos(obj, 257, 159);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffc00000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "DC");
        }
        {
            // dc output text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dc_output_text = obj;
            lv_obj_set_pos(obj, 113, 85);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "DC Output:");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
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
}