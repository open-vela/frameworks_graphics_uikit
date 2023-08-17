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

#include "../hal/hal.h"

#if 0

static lv_obj_t * screen;

bool page_last_screen_is_dialplate = false;

LV_IMG_DECLARE(img_src_bluetooth);
LV_IMG_DECLARE(img_src_battery);
LV_IMG_DECLARE(img_src_num_shadow);
LV_IMG_DECLARE(img_src_weather);
LV_IMG_DECLARE(img_src_centigrade);
LV_IMG_DECLARE(img_src_step);
LV_IMG_DECLARE(img_src_heart);

static struct clock_value_s clock_value;
static lv_obj_t * label_date;
static lv_obj_t * label_time_hour;
static lv_obj_t * label_time_min;

static lv_obj_t * obj_batt;

static lv_timer_t * task_label_time_update;
static lv_timer_t * task_color_bar_update;

static lv_auto_event_data_t ae_grp[] = {
    {&screen, LV_EVENT_LEAVE, 5000},
};

typedef struct {
    lv_color_t color;
    lv_coord_t y;
    lv_coord_t bar_width;
    lv_coord_t arc_radius;
    const void * img_src;
    lv_obj_t * label;
    lv_obj_t * bar;
    lv_obj_t * arc;
} color_bar_t;

static color_bar_t color_bar_grp[] = {
    {
        .color = _LV_COLOR_MAKE(0xF7, 0x93, 0x1E),
        .y = 245,
        .bar_width = 130,
        .arc_radius = 120,
        .img_src = &img_src_weather
    },

    {
        .color = _LV_COLOR_MAKE(0x3F, 0xA9, 0xF5),
        .y = 285,
        .bar_width = 130,
        .arc_radius = 80,
        .img_src = &img_src_step
    },

    {
        .color = _LV_COLOR_MAKE(0xED, 0x1C, 0x24),
        .y = 325,
        .bar_width = 130,
        .arc_radius = 40,
        .img_src = &img_src_heart
    },
};

enum color_bar_index {
    COLOR_BAR_WEATHER,
    COLOR_BAR_STEP,
    COLOR_BAR_HEART
};

static void obj_batt_create(lv_obj_t * par)
{
    lv_obj_t * obj = lv_obj_create(par, NULL);
    lv_obj_set_style_default(obj);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_height(obj, lv_obj_get_height(par));
    lv_obj_set_width(obj, lv_obj_get_height(par) / 2);
    lv_obj_align(obj, NULL, LV_ALIGN_IN_LEFT_MID, 2, 0);

    obj_batt = obj;
}

static void topbar_create(lv_obj_t * par)
{
    lv_obj_t * topbar = lv_obj_create(par, NULL);
    lv_obj_set_size(topbar, lv_obj_get_width(par), 25);
    lv_obj_align(topbar, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_style_default(topbar);

    lv_obj_t * img1 = lv_img_create(topbar, NULL);
    lv_img_set_src(img1, &img_src_bluetooth);
    lv_obj_align(img1, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

    lv_obj_t * img2 = lv_img_create(topbar, NULL);
    lv_img_set_src(img2, &img_src_battery);
    lv_obj_align(img2, NULL, LV_ALIGN_IN_RIGHT_MID, -20, 0);

    obj_batt_create(img2);
}

static void label_date_create(lv_obj_t * par)
{
    lv_obj_t * label = lv_label_create(par, NULL);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_erasbd_23);
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 25);
    lv_obj_set_auto_realign(label, true);

    label_date = label;
}

static void label_time_update(lv_timer_t * task)
{
    clock_get_value(&clock_value);

    const char * week_str[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    int week_index = clock_value.week % ARRAY_SIZE(week_str);
    lv_label_set_text_fmt(
        label_date,
        "%02d-#F15A24 %02d# %s",
        clock_value.month,
        clock_value.date,
        week_str[week_index]);
    lv_label_set_text_fmt(label_time_hour, "%02d", clock_value.hour);
    lv_label_set_text_fmt(label_time_min, "#F15A24 %02d#", clock_value.min);
}

static void label_time_create(lv_obj_t * par)
{
    lv_obj_t * label;

    label = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_erasbd_128);
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 33);
    lv_obj_set_auto_realign(label, true);
    label_time_hour = label;

    lv_obj_t * img = lv_img_create(par, NULL);
    lv_img_set_src(img, &img_src_num_shadow);
    lv_obj_set_pos(img, 16, 136);

    img = lv_img_create(par, img);
    lv_obj_set_pos(img, 99, 136);

    label = lv_label_create(par, label);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 115);
    label_time_min = label;
}

static void color_bar_update(lv_timer_t * task)
{
    lv_label_set_text_fmt(color_bar_grp[COLOR_BAR_WEATHER].label, "15");
    lv_label_set_text_fmt(color_bar_grp[COLOR_BAR_STEP].label, "%d", imu_get_steps());
    lv_label_set_text_fmt(
        color_bar_grp[COLOR_BAR_HEART].label,
        "%d",
        (int)particle_sensor_get_beats());
}

static void color_bar_create(lv_obj_t * par, color_bar_t * color_bar, int len)
{
    for(int i = 0; i < len; i++) {
        lv_color_t color = color_bar[i].color;
        lv_coord_t y_pos = color_bar[i].y;
        lv_coord_t bar_width = color_bar[i].bar_width;
        lv_coord_t arc_size = color_bar[i].arc_radius * 2;

        lv_obj_t * bar = lv_bar_create(par, NULL);
        lv_bar_set_value(bar, 100, LV_ANIM_OFF);
        lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, color);
        lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, color);
        lv_obj_set_size(bar, bar_width, 32);
        lv_obj_set_pos(bar, -12, y_pos);

        lv_obj_t * arc = lv_arc_create(par, NULL);
        lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, color);
        lv_obj_set_style_local_line_width(
            arc,
            LV_ARC_PART_INDIC,
            LV_STATE_DEFAULT,
            lv_obj_get_height(bar));
        lv_obj_set_style_local_pad_all(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_bg_opa(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_style_local_border_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
        lv_arc_set_angles(arc, 270, 360);
        lv_arc_set_bg_angles(arc, 270, 360);
        lv_obj_set_size(arc, arc_size, arc_size);

        lv_obj_align(arc, bar, LV_ALIGN_OUT_RIGHT_TOP, -lv_obj_get_width(arc) / 2 - 10, 0);

        lv_obj_t * img = lv_img_create(bar, NULL);
        lv_img_set_src(img, color_bar[i].img_src);
        lv_obj_align(img, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

        if(i == COLOR_BAR_WEATHER) {
            lv_obj_t * img_deg = lv_img_create(bar, NULL);
            lv_img_set_src(img_deg, &img_src_centigrade);
            lv_obj_align(img_deg, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
        }

        lv_obj_t * label = lv_label_create(bar, NULL);
        lv_obj_set_style_local_text_font(
            label,
            LV_LABEL_PART_MAIN,
            LV_STATE_DEFAULT,
            &font_erasbd_28);
        lv_obj_set_style_local_text_color(
            label,
            LV_LABEL_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_COLOR_WHITE);
        lv_obj_align(label, img, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

        lv_obj_move_foreground(bar);

        color_bar[i].bar = bar;
        color_bar[i].label = label;
        color_bar[i].arc = arc;
    }
}

static void page_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_GESTURE) {
        lv_gesture_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_GESTURE_DIR_LEFT) {
            lv_obj_send_event(obj, LV_EVENT_LEAVE, NULL);
        }
    }
    else if(event == LV_EVENT_LEAVE) {
        lv_obj_t * scr = page_menu_create();
        lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, LV_ANIM_TIME_DEFAULT, 0, true);
        page_last_screen_is_dialplate = true;
    }
    else if(event == LV_EVENT_DELETE) {
        lv_timer_del(task_label_time_update);
        lv_timer_del(task_color_bar_update);
    }
}

lv_obj_t * page_dialplate_create(void)
{
    AUTO_EVENT_CREATE(ae_grp);

    lv_obj_t * scr = page_screen_create();
    screen = scr;
    lv_obj_set_event_cb(scr, page_event_handler);

    clock_get_value(&clock_value);

    topbar_create(scr);
    label_date_create(scr);
    label_time_create(scr);
    color_bar_create(scr, color_bar_grp, ARRAY_SIZE(color_bar_grp));

    task_label_time_update = lv_timer_create(label_time_update, 100, lv_timer_PRIO_MID, NULL);
    label_time_update(task_label_time_update);

    task_color_bar_update = lv_timer_create(color_bar_update, 1000, lv_timer_PRIO_MID, NULL);
    color_bar_update(task_color_bar_update);

    return scr;
}

#endif
