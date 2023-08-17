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

LV_IMG_DECLARE(img_src_arrow_up);
LV_IMG_DECLARE(img_src_arrow_down);
LV_IMG_DECLARE(img_src_sleep);

static lv_obj_t * img_down;
static lv_obj_t * img_up;

static lv_obj_t * cont_sleep;
#define CONT_SLEEP_MOVE_DOWN(down) \
    LV_OBJ_ADD_ANIM(               \
                                   cont_sleep,                \
                                   y,                         \
                                   (down) ? -lv_obj_get_height(cont_sleep) / 2 : 0, LV_ANIM_TIME_DEFAULT)

typedef enum {
    SLEEP_TYPE_SHALLOW,
    SLEEP_TYPE_DEEP,
    SLEEP_TYPE_REM,
    SLEEP_TYPE_AWAKE,
    SLEEP_TYPE_MAX
} sleep_type;

static const lv_color_t sleep_type_color[SLEEP_TYPE_MAX] = {
    _LV_COLOR_MAKE(0x3f, 0x71, 0xf5),
    _LV_COLOR_MAKE(0x02, 0x31, 0x92),
    _LV_COLOR_MAKE(0x7a, 0xc9, 0x43),
    _LV_COLOR_MAKE(0xff, 0x93, 0x1e),
};

typedef struct {
    sleep_type type;
    uint32_t start_min;
    uint32_t end_min;
} sleep_time_t;

static sleep_time_t sleep_time_grp[] = {
    {SLEEP_TYPE_SHALLOW, 29, 588},
    {SLEEP_TYPE_DEEP, 61, 80},
    {SLEEP_TYPE_DEEP, 120, 200},
    {SLEEP_TYPE_REM, 80, 95},
    {SLEEP_TYPE_REM, 250, 310},
    {SLEEP_TYPE_AWAKE, 400, 410},
};

typedef struct {
    const char * text;
    sleep_type type;
} sleep_info_t;

static sleep_info_t sleep_info_grp[] = {
    {"Shallow sleep", SLEEP_TYPE_SHALLOW},
    {"Deep sleep", SLEEP_TYPE_DEEP},
    {"REM", SLEEP_TYPE_REM},
    {"Awake", SLEEP_TYPE_AWAKE},
};

static lv_auto_event_data_t ae_grp[] = {
    {&img_down, LV_EVENT_CLICKED, 2000},
    {&img_up, LV_EVENT_CLICKED, 2000},
    {&screen, LV_EVENT_LEAVE, 2000},
};

static void img_sleep_create(lv_obj_t * par)
{
    lv_obj_t * img = lv_img_create(par, NULL);
    lv_img_set_src(img, &img_src_sleep);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 0, 32);
}

static void label_score_create(lv_obj_t * par)
{
    lv_obj_t * label1 = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(
        label1,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        &font_bahnschrift_20);
    lv_obj_set_style_local_text_color(
        label1,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x80, 0x80, 0x80));
    lv_label_set_text(label1, "Score");
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 123);

    lv_obj_t * label2 = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(
        label2,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        &font_bahnschrift_72);
    lv_obj_set_style_local_text_color(
        label2,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_WHITE);
    lv_label_set_text(label2, "78");
    lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 0, 148);
}

static void label_total_time_create(lv_obj_t * par)
{
    lv_obj_t * label1 = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(
        label1,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        &font_bahnschrift_20);
    lv_obj_set_style_local_text_color(
        label1,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x80, 0x80, 0x80));
    lv_label_set_text(label1, "Total time");
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 224);

    lv_obj_t * label2 = lv_label_create(par, label1);
    lv_obj_set_style_local_text_color(
        label2,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x3F, 0xA9, 0xF5));
    lv_label_set_recolor(label2, true);
    lv_label_set_align(label2, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(
        label2,
        "8 h 52 min\n"
        "00:29#808080 -#09:48");
    lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 0, 249);
}

static void obj_sleep_time_create(lv_obj_t * par, sleep_time_t * sleep_time, int len)
{
    lv_obj_t * obj_base = lv_obj_create(par, NULL);
    lv_obj_set_style_default(obj_base);
    lv_obj_set_size(obj_base, 165, 74);
    lv_obj_set_style_local_radius(obj_base, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_clip_corner(obj_base, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_align(obj_base, NULL, LV_ALIGN_IN_TOP_MID, 0, PAGE_VER_RES + 42);

    uint32_t time_len = sleep_time[0].end_min - sleep_time[0].start_min;
    uint32_t time_start = sleep_time[0].start_min;
    lv_coord_t width_base = lv_obj_get_width(obj_base);

    for(int i = 0; i < len; i++) {
        uint8_t type_index = sleep_time[i].type;
        lv_color_t color = sleep_type_color[type_index];

        lv_obj_t * obj = lv_obj_create(obj_base, NULL);
        lv_obj_set_style_default(obj);
        lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);

        lv_coord_t width = width_base * (sleep_time[i].end_min - sleep_time[i].start_min) / time_len;
        lv_coord_t x_ofs = width_base * (sleep_time[i].start_min - time_start) / time_len;

        lv_obj_set_size(obj, width, lv_obj_get_height(obj_base));
        lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_LEFT, x_ofs, 0);
    }

    lv_obj_t * label1 = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_bahnschrift_15);
    lv_obj_set_style_local_text_color(
        label1,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x80, 0x80, 0x80));

    uint32_t start_min = sleep_time[0].start_min;
    lv_label_set_text_fmt(label1, "%02d:%02d", start_min / 60, start_min % 60);
    lv_obj_align(label1, obj_base, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t * label2 = lv_label_create(par, label1);
    lv_obj_set_style_local_text_font(
        label2,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        &font_bahnschrift_15);
    lv_obj_set_style_local_text_color(
        label2,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x80, 0x80, 0x80));

    uint32_t end_min = sleep_time[0].end_min;
    lv_label_set_text_fmt(label2, "%02d:%02d", end_min / 60, end_min % 60);
    lv_obj_align(label2, obj_base, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);
}

static uint32_t sleep_time_get_sum(sleep_time_t * sleep_time, int len, sleep_type check_type)
{
    uint32_t sum = 0;
    if(check_type == SLEEP_TYPE_SHALLOW) {
        sum = sleep_time[0].end_min - sleep_time[0].start_min;
        sum -= sleep_time_get_sum(sleep_time, len, SLEEP_TYPE_DEEP);
        sum -= sleep_time_get_sum(sleep_time, len, SLEEP_TYPE_REM);
        sum -= sleep_time_get_sum(sleep_time, len, SLEEP_TYPE_AWAKE);
    }
    else {
        for(int i = 0; i < len; i++) {
            if(sleep_time[i].type == check_type) {
                uint32_t time = sleep_time[i].end_min - sleep_time[i].start_min;
                sum += time;
            }
        }
    }
    return sum;
}

static void obj_sleep_info_create(lv_obj_t * par, sleep_info_t * sleep_info, int len)
{
    for(int i = 0; i < len; i++) {
        lv_obj_t * obj_base = lv_obj_create(par, NULL);
        lv_obj_set_style_default(obj_base);
        lv_obj_set_size(obj_base, lv_obj_get_width(par) - 40, 50);
        //lv_obj_set_drag_parent(obj_base, true);
        //lv_obj_set_gesture_parent(obj_base, true);

        lv_coord_t y_ofs = PAGE_VER_RES + 150 + (lv_obj_get_height(obj_base) + 5) * i;
        lv_obj_align(obj_base, NULL, LV_ALIGN_IN_TOP_MID, 0, y_ofs);

        lv_obj_t * label1 = lv_label_create(obj_base, NULL);
        lv_obj_set_style_local_text_font(
            label1,
            LV_LABEL_PART_MAIN,
            LV_STATE_DEFAULT,
            &font_bahnschrift_20);
        lv_obj_set_style_local_text_color(
            label1,
            LV_LABEL_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_COLOR_MAKE(0x80, 0x80, 0x80));
        lv_label_set_text(label1, sleep_info[i].text);
        lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

        lv_obj_t * label2 = lv_label_create(obj_base, label1);
        lv_obj_set_style_local_text_color(
            label2,
            LV_LABEL_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_COLOR_WHITE);
        uint32_t time = sleep_time_get_sum(
                            sleep_time_grp,
                            ARRAY_SIZE(sleep_time_grp),
                            sleep_info[i].type);
        lv_label_set_text_fmt(label2, "%dh %dmin", time / 60, time % 60);
        lv_obj_align(label2, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

        lv_obj_t * label3 = lv_label_create(obj_base, label2);
        lv_obj_set_style_local_text_color(
            label3,
            LV_LABEL_PART_MAIN,
            LV_STATE_DEFAULT,
            sleep_type_color[sleep_info[i].type]);
        uint32_t time_len = sleep_time_grp[0].end_min - sleep_time_grp[0].start_min;
        lv_label_set_text_fmt(label3, "%d%%", time * 100 / time_len);
        lv_obj_align(label3, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    }
}

static void img_arrow_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        bool is_down = (lv_img_get_src(obj) == &img_src_arrow_down);
        CONT_SLEEP_MOVE_DOWN(is_down);
    }
}

static void img_arrow_create(lv_obj_t * par)
{
    lv_obj_t * img1 = lv_img_create(par, NULL);
    lv_img_set_src(img1, &img_src_arrow_down);
    lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_MID, 0, 339);
    lv_obj_set_event_cb(img1, img_arrow_event_handler);
    lv_obj_set_click(img1, true);

    lv_obj_t * img2 = lv_img_create(par, NULL);
    lv_img_set_src(img2, &img_src_arrow_up);
    lv_obj_align(img2, NULL, LV_ALIGN_IN_TOP_MID, 0, PAGE_VER_RES + 12);
    lv_obj_set_event_cb(img2, img_arrow_event_handler);
    lv_obj_set_click(img2, true);

    img_down = img1;
    img_up = img2;
}

static lv_obj_t * cont_sleep_create(lv_obj_t * par)
{
    lv_obj_t * obj = lv_cont_create(par, NULL);
    lv_obj_set_size(obj, lv_obj_get_width(par), lv_obj_get_height(par) * 2);
    lv_obj_align(obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_style_default(obj);

    return obj;
}

static void page_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_GESTURE) {
        lv_gesture_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_GESTURE_DIR_RIGHT) {
            if(dir == LV_GESTURE_DIR_RIGHT) {
                lv_obj_send_event(obj, LV_EVENT_LEAVE, NULL);
            }
        }
        else if(dir == LV_GESTURE_DIR_TOP) {
            CONT_SLEEP_MOVE_DOWN(true);
        }
        else if(dir == LV_GESTURE_DIR_BOTTOM) {
            CONT_SLEEP_MOVE_DOWN(false);
        }
    }
    else if(event == LV_EVENT_LEAVE) {
        page_return_menu(true);
    }
    else if(event == LV_EVENT_DELETE) {
    }
}

lv_obj_t * page_sleep_create(void)
{
    AUTO_EVENT_CREATE(ae_grp);

    lv_obj_t * scr = page_screen_create();
    screen = scr;
    lv_obj_set_event_cb(scr, page_event_handler);

    lv_obj_t * cont = cont_sleep_create(scr);
    cont_sleep = cont;

    img_arrow_create(cont);
    img_sleep_create(cont);
    label_score_create(cont);
    label_total_time_create(cont);

    obj_sleep_time_create(cont, sleep_time_grp, ARRAY_SIZE(sleep_time_grp));
    obj_sleep_info_create(cont, sleep_info_grp, ARRAY_SIZE(sleep_info_grp));

    return scr;
}

#endif
