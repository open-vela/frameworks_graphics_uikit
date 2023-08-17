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

LV_IMG_DECLARE(img_src_icon_start);
LV_IMG_DECLARE(img_src_icon_pause);
LV_IMG_DECLARE(img_src_icon_reset);
LV_IMG_DECLARE(img_src_icon_flag);

static lv_obj_t * label_time;
static lv_obj_t * list_history;

typedef struct {
    const lv_img_dsc_t * img_src;
    lv_obj_t * btn;
    lv_obj_t * img;
} btn_sw_t;

typedef struct {
    lv_obj_t * list_btn;
    const char * text;
    const uint16_t btn_val;
} list_btn_t;

enum BtnGrpEnum {
    BTN_START_PAUSE,
    BTN_FLAG_RESET,
};

static btn_sw_t btn_grp[] = {
    {&img_src_icon_start},
    {&img_src_icon_reset},
};

static const lv_coord_t BTN_WIDTH = 83;
static const lv_coord_t BTN_HEIGHT = 60;

static const lv_coord_t label_time_record_y = 18;
static const lv_coord_t label_time_ready_y = 64;

static lv_timer_t * task_sw_update = NULL;
static bool sw_is_pause = true;
static uint32_t sw_current_time = 0;
static uint32_t sw_interval_time = 0;
static uint32_t sw_last_time = 0;
static uint8_t sw_history_record_cnt = 0;

static lv_auto_event_data_t ae_grp[] = {
    {&(btn_grp[BTN_START_PAUSE].btn), LV_EVENT_CLICKED, 2000},
    {&(btn_grp[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 453},
    {&(btn_grp[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 888},
    {&(btn_grp[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1605},
    {&(btn_grp[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 785},
    {&(btn_grp[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1200},
    {&(btn_grp[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1456},
    {&(btn_grp[BTN_START_PAUSE].btn), LV_EVENT_CLICKED, 2000},
    {&(btn_grp[BTN_FLAG_RESET].btn), LV_EVENT_CLICKED, 1000},
    {&screen, LV_EVENT_LEAVE, 2000},
};

static void sw_show_time(uint32_t ms)
{
    uint16_t min = 0, sec = 0, msec = 0;
    min = ms / 60000;
    sec = (ms - (min * 60000)) / 1000;
    msec = ms - (sec * 1000) - (min * 60000);
    lv_label_set_text_fmt(label_time, "%02d:%02d.%03d", min, sec, msec);
}

static void sw_pause(void)
{
    if(sw_is_pause) {
        sw_interval_time = lv_tick_get() - sw_last_time;
        lv_timer_set_prio(task_sw_update, lv_timer_PRIO_HIGH);
        sw_is_pause = false;
    }
    else {
        sw_last_time = sw_current_time;
        lv_timer_set_prio(task_sw_update, lv_timer_PRIO_OFF);
        sw_is_pause = true;
    }
}

static void sw_reset(void)
{
    sw_last_time = 0;
    sw_current_time = 0;
    sw_interval_time = 0;
    sw_is_pause = true;
    sw_history_record_cnt = 0;
    lv_timer_set_prio(task_sw_update, lv_timer_PRIO_OFF);
    sw_show_time(sw_current_time);
    lv_list_clean(list_history);

    if(lv_obj_get_y(label_time) == label_time_record_y) {
        LV_OBJ_ADD_ANIM(label_time, y, label_time_ready_y, 500);
    }
}

static void sw_update(lv_timer_t * task)
{
    sw_current_time = lv_tick_get() - sw_interval_time;
    sw_show_time(sw_current_time);
}

static void sw_record(void)
{
    lv_obj_t * list_btn;
    lv_obj_t * label;
    uint16_t min = 0, sec = 0, msec = 0;

    if(sw_history_record_cnt < 10 && !sw_is_pause) {
        min = sw_current_time / 60000;
        sec = (sw_current_time - (min * 60000)) / 1000;
        msec = sw_current_time - (sec * 1000) - (min * 60000);

        char buf[64];
        lv_snprintf(
            buf,
            sizeof(buf),
            "#87FFCE %d. #%02d:%02d.%03d",
            sw_history_record_cnt + 1,
            min,
            sec,
            msec);

        list_btn = lv_list_add_btn(list_history, NULL, buf);
        label = lv_obj_get_child(list_btn, NULL);
        lv_label_set_recolor(label, true);
        lv_list_focus(list_btn, LV_ANIM_ON);
        lv_obj_set_style_default(list_btn);
        lv_obj_set_style_local_text_decor(
            list_btn,
            LV_BTN_PART_MAIN,
            LV_STATE_FOCUSED,
            LV_TEXT_DECOR_NONE);
        sw_history_record_cnt++;
    }

    if(lv_obj_get_y(label_time) == label_time_ready_y) {
        LV_OBJ_ADD_ANIM(label_time, y, label_time_record_y, 500);
    }
}

static void btn_grp_event_handler(lv_obj_t * obj, lv_event_t event)
{

    if(event == LV_EVENT_CLICKED) {
        lv_obj_t * img = lv_obj_get_child(obj, NULL);
        const lv_img_dsc_t * img_src = lv_img_get_src(img);

        if(obj == btn_grp[BTN_START_PAUSE].btn) {
            sw_pause();
            if(sw_is_pause) {
                lv_img_set_src(btn_grp[BTN_START_PAUSE].img, &img_src_icon_start);
                lv_img_set_src(btn_grp[BTN_FLAG_RESET].img, &img_src_icon_reset);
            }
            else {
                lv_img_set_src(btn_grp[BTN_START_PAUSE].img, &img_src_icon_pause);
                lv_img_set_src(btn_grp[BTN_FLAG_RESET].img, &img_src_icon_flag);
                lv_obj_set_state(btn_grp[BTN_FLAG_RESET].btn, LV_STATE_DEFAULT);
            }
        }
        else if(obj == btn_grp[BTN_FLAG_RESET].btn) {
            if(img_src == &img_src_icon_flag) {
                sw_record();
            }
            else {
                if(sw_is_pause) {
                    sw_reset();
                    lv_obj_set_state(btn_grp[BTN_FLAG_RESET].btn, LV_STATE_DISABLED);
                    sw_history_record_cnt = 0;
                }
            }
        }
    }
}

static void label_time_create(lv_obj_t * par)
{
    lv_obj_t * label = lv_label_create(par, NULL);

    lv_obj_set_style_local_text_font(
        label,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        &font_bahnschrift_48);
    lv_obj_set_style_local_text_color(
        label,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x87, 0xFF, 0xCE));

    lv_label_set_text(label, "00:00.000");
    lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(label, 190);
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, label_time_ready_y);

    label_time = label;
}

static void line_sw_create(lv_obj_t * par)
{
    static lv_point_t line_points[] = {{0, 0}, {170, 0}};

    lv_obj_t * line1 = lv_line_create(label_time, NULL);
    lv_line_set_points(line1, line_points, ARRAY_SIZE(line_points));

    lv_obj_set_style_local_line_color(
        line1,
        LV_LINE_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x80, 0x80, 0x80));
    lv_obj_set_style_local_line_width(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_align(line1, label_time, LV_ALIGN_IN_BOTTOM_MID, 0, -3);

    lv_obj_t * line2 = lv_line_create(par, line1);
    lv_obj_align(line2, NULL, LV_ALIGN_IN_TOP_MID, 0, 286);
}

static void list_history_create(lv_obj_t * par)
{
    lv_obj_t * list = lv_list_create(par, NULL);
    lv_obj_set_style_default(list);
    lv_obj_set_style_local_text_font(list, LV_LIST_PART_BG, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_obj_set_size(list, 170, 170);
    lv_obj_align(list, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(list, true);
    list_history = list;
}

static void btn_grp_create(lv_obj_t * par, btn_sw_t * btn_sw, int len)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, LV_STATE_DEFAULT, 5);
    lv_style_set_border_color(&style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_border_width(&style, LV_STATE_DEFAULT, 2);

    lv_style_set_bg_color(&style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&style, LV_STATE_DISABLED, LV_COLOR_GRAY);

    for(int i = 0; i < len; i++) {
        lv_obj_t * btn = lv_btn_create(par, NULL);
        lv_obj_set_size(btn, BTN_WIDTH, BTN_HEIGHT);

        lv_coord_t x_ofs = BTN_WIDTH / 2 + 3;
        lv_obj_align(btn, NULL, LV_ALIGN_IN_TOP_MID, i == 0 ? -x_ofs : x_ofs, 300);
        lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style);
        lv_obj_set_event_cb(btn, btn_grp_event_handler);
        lv_obj_set_style_local_bg_color(
            btn,
            LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_COLOR_MAKE(0x87, 0xFF, 0xCE));
        lv_obj_set_style_local_radius(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 10);

        lv_obj_t * img = lv_img_create(btn, NULL);
        lv_img_set_src(img, btn_sw[i].img_src);
        lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

        btn_sw[i].btn = btn;
        btn_sw[i].img = img;
    }
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
        lv_timer_del(task_sw_update);
        page_return_menu(false);
    }
    else if(event == LV_EVENT_DELETE) {
    }
}

lv_obj_t * page_stop_watch_create(void)
{
    AUTO_EVENT_CREATE(ae_grp);

    task_sw_update = lv_timer_create(
                         sw_update,
                         51,
                         (sw_is_pause ? lv_timer_PRIO_OFF : lv_timer_PRIO_HIGH),
                         NULL);

    if(screen != NULL) {
        return screen;
    }

    lv_obj_t * scr = page_screen_create();
    screen = scr;
    lv_obj_set_event_cb(scr, page_event_handler);

    list_history_create(scr);
    label_time_create(scr);
    btn_grp_create(scr, btn_grp, ARRAY_SIZE(btn_grp));
    line_sw_create(scr);

    return scr;
}

#endif
