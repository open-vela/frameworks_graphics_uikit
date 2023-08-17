/*
 * Copyright (C) 2020 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "page.h"

#if 0

static lv_obj_t * screen;

#define BX_NAME "BandX"
#define BX_VERSION "v1.0"

LV_IMG_DECLARE(img_src_icon_volume_reduce);
LV_IMG_DECLARE(img_src_icon_volume_add);
LV_IMG_DECLARE(img_src_icon_minus);
LV_IMG_DECLARE(img_src_icon_plus);

typedef struct {
    const char * text;
    const lv_img_dsc_t * img_src_left;
    const lv_img_dsc_t * img_src_right;
    lv_coord_t y;

    lv_obj_t * img_left;
    lv_obj_t * img_right;
    lv_obj_t * bar;
} bar_setting_t;

static bar_setting_t bar_setting_grp[] = {
    {.text = "Volume", .img_src_left = &img_src_icon_volume_reduce, .img_src_right = &img_src_icon_volume_add, 24},
    {.text = "Backlight", .img_src_left = &img_src_icon_minus, .img_src_right = &img_src_icon_plus, 98},
};

static lv_auto_event_data_t ae_grp[] = {
    {&(bar_setting_grp[0].img_left), LV_EVENT_CLICKED, 500},
    {&(bar_setting_grp[0].img_left), LV_EVENT_CLICKED, 500},
    {&(bar_setting_grp[0].img_right), LV_EVENT_CLICKED, 500},
    {&(bar_setting_grp[0].img_right), LV_EVENT_CLICKED, 500},
    {&(bar_setting_grp[1].img_left), LV_EVENT_CLICKED, 500},
    {&(bar_setting_grp[1].img_left), LV_EVENT_CLICKED, 500},
    {&(bar_setting_grp[1].img_right), LV_EVENT_CLICKED, 500},
    {&(bar_setting_grp[1].img_right), LV_EVENT_CLICKED, 500},
    {&screen, LV_EVENT_LEAVE, 1000},
};

static void bar_setting_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        bar_setting_t * bar_setting = (bar_setting_t *)lv_obj_get_user_data(obj);
        const lv_img_dsc_t * img_src = lv_img_get_src(obj);
        int add_value = (img_src == bar_setting->img_src_left) ? -10 : +10;
        int16_t old_value = lv_bar_get_value(bar_setting->bar);
        lv_bar_set_value(bar_setting->bar, old_value + add_value, LV_ANIM_ON);
    }
}

static void bar_setting_create(lv_obj_t * par, bar_setting_t * bar_setting, int len)
{
    for(int i = 0; i < len; i++) {
        lv_obj_t * obj_base = lv_obj_create(par, NULL);
        lv_obj_set_size(obj_base, lv_obj_get_width(par) - 40, 50);
        lv_obj_set_style_default(obj_base);
        lv_obj_align(obj_base, NULL, LV_ALIGN_IN_TOP_MID, 0, bar_setting[i].y);

        lv_obj_t * label = lv_label_create(obj_base, NULL);
        lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_bahnschrift_20);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x66, 0x66, 0x66));
        lv_label_set_text(label, bar_setting[i].text);
        lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

        lv_obj_t * bar = lv_bar_create(obj_base, NULL);
        lv_obj_set_size(bar, 74, 6);
        lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x89, 0xFF));
        lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
        lv_obj_align(bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        lv_bar_set_value(bar, 50, LV_ANIM_OFF);
        bar_setting[i].bar = bar;

        lv_obj_t * img1 = lv_img_create(obj_base, NULL);
        lv_obj_set_user_data(img1, &(bar_setting[i]));
        lv_obj_set_event_cb(img1, bar_setting_event_handler);
        lv_img_set_src(img1, bar_setting[i].img_src_left);
        lv_obj_set_click(img1, true);
        lv_obj_set_style_local_image_recolor_opa(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_obj_set_style_local_image_recolor(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x66, 0x66, 0x66));
        lv_obj_set_style_local_image_recolor(img1, LV_IMG_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x00, 0x89, 0xFF));
        lv_obj_align(img1, bar, LV_ALIGN_OUT_LEFT_MID, -10, 0);

        lv_obj_t * img2 = lv_img_create(obj_base, img1);
        lv_img_set_src(img2, bar_setting[i].img_src_right);
        lv_obj_align(img2, bar, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

        bar_setting[i].img_left = img1;
        bar_setting[i].img_right = img2;
    }
}

static void sw_auto_show_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        //page_set_autoshow_enable(!page_get_autoshow_enable());
    }
}

static void sw_auto_show_create(lv_obj_t * par)
{
    lv_obj_t * label = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x66, 0x66, 0x66));
    lv_label_set_text(label, "Auto-show");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 23, 187);

    lv_obj_t * sw = lv_switch_create(par, NULL);
    lv_obj_set_size(sw, 41, 23);
    lv_obj_align(sw, label, LV_ALIGN_OUT_RIGHT_MID, 12, -5);
    lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
    lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x89, 0xFF));
    lv_obj_set_style_local_bg_color(sw, LV_SWITCH_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xAA, 0xAA, 0xAA));
    lv_obj_set_style_local_outline_width(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_event_cb(sw, sw_auto_show_event_handler);
    page_get_autoshow_enable() ? lv_switch_on(sw, LV_ANIM_OFF) : lv_switch_off(sw, LV_ANIM_OFF);
}

static void label_info_create(lv_obj_t * par)
{
    lv_obj_t * label = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label, BX_NAME " " BX_VERSION "\n"__DATE__
                      "\nBuild");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 270);
}

static void page_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_GESTURE) {
        lv_gesture_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_GESTURE_DIR_RIGHT) {
            lv_obj_send_event(obj, LV_EVENT_LEAVE, NULL);
        }
    }
    else if(event == LV_EVENT_LEAVE) {
        page_return_menu(true);
    }
    else if(event == LV_EVENT_DELETE) {
    }
}

lv_obj_t * page_settings_create(void)
{
    AUTO_EVENT_CREATE(ae_grp);

    lv_obj_t * scr = page_screen_create();
    screen = scr;
    lv_obj_set_event_cb(scr, page_event_handler);

    bar_setting_create(scr, bar_setting_grp, ARRAY_SIZE(bar_setting_grp));
    sw_auto_show_create(scr);
    label_info_create(scr);

    return scr;
}

#endif
