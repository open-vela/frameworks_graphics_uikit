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

LV_IMG_DECLARE(img_src_heart_color);
LV_IMG_DECLARE(img_src_icon_reset);
LV_IMG_DECLARE(img_src_arrow_up);
LV_IMG_DECLARE(img_src_arrow_down);

static lv_obj_t * label_hr_current;
static lv_obj_t * img_heart;
static lv_obj_t * label_hr_record_range;
static lv_obj_t * label_hr_record_time;

static lv_obj_t * img_down;
static lv_obj_t * img_up;
static lv_obj_t * btn_rst;

static lv_timer_t * task_label_hr_update;

static lv_obj_t * chart_hr;
static lv_chart_series_t * chart_hr_ser1;

static lv_obj_t * cont_hr;
#define CONT_HR_MOVE_DOWN(down)                                                  \
    do                                                                           \
    {                                                                            \
        LV_OBJ_ADD_ANIM(                                                         \
                                                                                 cont_hr,                                                             \
                                                                                 y,                                                                   \
                                                                                 (down) ? -lv_obj_get_height(cont_hr) / 2 : 0, LV_ANIM_TIME_DEFAULT); \
        img_heart_anim_enable(!down);                                            \
    } while (0)

typedef struct {
    const char * text;
    int value;
    lv_color_t color;
    lv_coord_t y;
} color_bar_t;

static color_bar_t color_bar_grp[] = {
    {"Relaxation", 45, _LV_COLOR_MAKE(0x3F, 0xA9, 0xF5), 158},
    {"Sports", 30, _LV_COLOR_MAKE(0x7A, 0xC9, 0x43), 212},
    {"Competition", 20, _LV_COLOR_MAKE(0xF1, 0x5A, 0x24), 266},
    {"Endurance", 5, _LV_COLOR_MAKE(0xC1, 0x27, 0x2D), 320},
};

static lv_auto_event_data_t ae_grp[] = {
    {&btn_rst, LV_EVENT_CLICKED, 2000},
    {&img_down, LV_EVENT_CLICKED, 1000},
    {&img_up, LV_EVENT_CLICKED, 1000},
    {&screen, LV_EVENT_LEAVE, 1000},
};

static void label_hr_current_update(lv_timer_t * task)
{
    float hr_min, hr_max;
    int hr_beats = (int)particle_sensor_get_beats();
    if(!particle_sensor_get_beats_range(&hr_min, &hr_max)) {
        return;
    }

    lv_label_set_text_fmt(label_hr_current, "%d", hr_beats);
    lv_label_set_text_fmt(label_hr_record_range, "%d-%d", (int)hr_min, (int)hr_max);
    lv_chart_set_next(chart_hr, chart_hr_ser1, hr_beats);
}

static void img_heart_anim_enable(bool en)
{
    if(en) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img_heart);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_zoom);
        lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, (int)(LV_IMG_ZOOM_NONE * 0.7f));
        lv_anim_set_time(&a, 300);
        lv_anim_set_playback_time(&a, 300);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_repeat_delay(&a, 800);
        lv_anim_path_t path;
        lv_anim_path_init(&path);
        lv_anim_path_set_cb(&path, lv_anim_path_overshoot);
        lv_anim_set_path(&a, &path);
        lv_anim_start(&a);
    }
    else {
        lv_anim_del(img_heart, (lv_anim_exec_xcb_t)lv_img_set_zoom);
    }
}

static void label_hr_current_create(lv_obj_t * par)
{
    lv_obj_t * img = lv_img_create(par, NULL);
    lv_img_set_src(img, &img_src_heart_color);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 0, 28);

    lv_obj_t * label1 = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(
        label1,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        &font_bahnschrift_72);
    lv_label_set_text(label1, "--");
    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 95);
    lv_obj_set_auto_realign(label1, true);

    lv_obj_t * label2 = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(
        label2,
        LV_LABEL_PART_MAIN,
        LV_STATE_DEFAULT,
        &font_bahnschrift_20);
    lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_label_set_text(label2, "bpm");
    lv_obj_align(label2, label1, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, -20);

    label_hr_current = label1;
    img_heart = img;
}

static void label_hr_record_time_update(void)
{
    struct clock_value_s clock_value;
    clock_get_value(&clock_value);
    lv_label_set_text_fmt(label_hr_record_time, "%02d:%02d", clock_value.hour, clock_value.min);
}

static void btn_rst_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        particle_sensor_reset_beats_range();
        label_hr_record_time_update();

        lv_obj_t * img = lv_obj_get_child(obj, NULL);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
        lv_anim_set_values(&a, 0, 3600);
        lv_anim_set_time(&a, 1000);
        lv_anim_path_t path;
        lv_anim_path_init(&path);
        lv_anim_path_set_cb(&path, lv_anim_path_ease_out);
        lv_anim_set_path(&a, &path);
        lv_anim_start(&a);
    }
}

static void btn_rst_create(lv_obj_t * par)
{
    lv_obj_t * btn = lv_btn_create(par, NULL);
    lv_obj_set_size(btn, 149, 44);
    lv_obj_align(btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 279);
    lv_obj_set_event_cb(btn, btn_rst_event_handler);
    lv_obj_set_style_local_bg_color(
        btn,
        LV_BTN_PART_MAIN,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0xFF, 0x54, 0x2A));
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_obj_set_style_local_radius(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_click_anim_default(btn);

    lv_obj_t * img = lv_img_create(btn, NULL);
    lv_img_set_src(img, &img_src_icon_reset);
    lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

    btn_rst = btn;
}

static void label_hr_record_create(lv_obj_t * par)
{
    static lv_style_t style1;
    lv_style_init(&style1);
    lv_style_set_text_font(&style1, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_style_set_text_color(&style1, LV_STATE_DEFAULT, LV_COLOR_GRAY);

    static lv_style_t style2;
    lv_style_init(&style2);
    lv_style_set_text_font(&style2, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_style_set_text_color(&style2, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xF1, 0x5A, 0x24));

    lv_obj_t * label1 = lv_label_create(par, NULL);
    lv_obj_add_style(label1, LV_LABEL_PART_MAIN, &style1);
    lv_label_set_text(label1, "Recent updates");
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);

    lv_obj_t * label2 = lv_label_create(par, label1);
    lv_obj_add_style(label2, LV_LABEL_PART_MAIN, &style2);
    lv_label_set_text(label2, "--:--");
    lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 0, 192);
    lv_obj_set_auto_realign(label2, true);

    lv_obj_t * label3 = lv_label_create(par, NULL);
    lv_obj_add_style(label3, LV_LABEL_PART_MAIN, &style1);
    lv_label_set_text(label3, "Range");
    lv_obj_align(label3, NULL, LV_ALIGN_IN_TOP_MID, 0, 222);

    lv_obj_t * label4 = lv_label_create(par, NULL);
    lv_obj_add_style(label4, LV_LABEL_PART_MAIN, &style2);
    lv_label_set_text(label4, "");
    lv_obj_align(label4, NULL, LV_ALIGN_IN_TOP_MID, 0, 246);
    lv_obj_set_auto_realign(label4, true);

    label_hr_record_time = label2;
    label_hr_record_range = label4;
}

static void chart_hr_create(lv_obj_t * par)
{
    lv_obj_t * chart = lv_chart_create(par, NULL);
    lv_obj_set_size(chart, 156, 87);
    lv_obj_set_click(chart, false);
    lv_obj_align(chart, NULL, LV_ALIGN_IN_TOP_MID, 0, PAGE_VER_RES + 36);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);

    lv_obj_set_style_local_bg_color(
        chart,
        LV_CHART_PART_BG,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x1A, 0x1A, 0x1A));
    lv_obj_set_style_local_border_width(chart, LV_CHART_PART_BG, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_radius(chart, LV_CHART_PART_BG, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_border_color(
        chart,
        LV_CHART_PART_BG,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x66, 0x66, 0x66));

    lv_obj_set_style_local_bg_opa(chart, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, LV_OPA_50);
    lv_obj_set_style_local_bg_grad_dir(chart, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_obj_set_style_local_bg_main_stop(chart, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 255);
    lv_obj_set_style_local_bg_grad_stop(chart, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_line_width(chart, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 1);

    lv_chart_set_div_line_count(chart, 0, 4);
    lv_obj_set_style_local_line_color(
        chart,
        LV_CHART_PART_SERIES_BG,
        LV_STATE_DEFAULT,
        LV_COLOR_MAKE(0x66, 0x66, 0x66));

    lv_chart_set_y_range(chart, LV_CHART_AXIS_PRIMARY_Y, 50, 180);
    lv_chart_set_point_count(chart, 100);

    chart_hr_ser1 = lv_chart_add_series(chart, LV_COLOR_MAKE(0xfc, 0x5c, 0x00));

    chart_hr = chart;
}

static void img_arrow_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        bool is_down = (lv_img_get_src(obj) == &img_src_arrow_down);
        CONT_HR_MOVE_DOWN(is_down);
    }
}

static void img_arrow_create(lv_obj_t * par)
{
    lv_obj_t * img1 = lv_img_create(par, NULL);
    lv_img_set_src(img1, &img_src_arrow_down);
    lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_MID, 0, 339);
    lv_obj_set_event_cb(img1, img_arrow_event_handler);
    lv_obj_set_click(img1, true);
    img_down = img1;

    lv_obj_t * img2 = lv_img_create(par, NULL);
    lv_img_set_src(img2, &img_src_arrow_up);
    lv_obj_align(img2, NULL, LV_ALIGN_IN_TOP_MID, 0, PAGE_VER_RES + 12);
    lv_obj_set_event_cb(img2, img_arrow_event_handler);
    lv_obj_set_click(img2, true);
    img_up = img2;
}

static void color_bar_create(lv_obj_t * par, color_bar_t * color_bar, int len)
{
    static lv_style_t style1;
    lv_style_init(&style1);
    lv_style_set_text_font(&style1, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_style_set_text_color(&style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    for(int i = 0; i < len; i++) {
        lv_obj_t * bar = lv_bar_create(par, NULL);
        lv_bar_set_value(bar, color_bar[i].value, LV_ANIM_OFF);
        lv_obj_set_size(bar, 156, 10);
        lv_obj_set_style_local_bg_color(
            bar,
            LV_BAR_PART_INDIC,
            LV_STATE_DEFAULT,
            color_bar[i].color);
        lv_obj_align(bar, NULL, LV_ALIGN_IN_TOP_MID, 0, PAGE_VER_RES + color_bar[i].y);

        lv_obj_t * label = lv_label_create(par, NULL);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style1);
        lv_label_set_text_fmt(label, "%s %d%%", color_bar[i].text, color_bar[i].value);
        lv_obj_align(label, bar, LV_ALIGN_OUT_TOP_LEFT, 0, -2);
    }
}

static lv_obj_t * cont_hr_create(lv_obj_t * par)
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
            lv_obj_send_event(obj, LV_EVENT_LEAVE, NULL);
        }
        else if(dir == LV_GESTURE_DIR_TOP) {
            CONT_HR_MOVE_DOWN(true);
        }
        else if(dir == LV_GESTURE_DIR_BOTTOM) {
            CONT_HR_MOVE_DOWN(false);
        }
    }
    else if(event == LV_EVENT_LEAVE) {
        img_heart_anim_enable(false);
        page_return_menu(true);
    }

    else if(event == LV_EVENT_DELETE) {
        lv_timer_del(task_label_hr_update);
    }
}

lv_obj_t * page_heart_rate_create(void)
{
    AUTO_EVENT_CREATE(ae_grp);

    lv_obj_t * scr = page_screen_create();
    screen = scr;
    lv_obj_set_event_cb(scr, page_event_handler);

    lv_obj_t * cont = cont_hr_create(scr);
    cont_hr = cont;

    label_hr_current_create(cont);
    label_hr_record_create(cont);
    label_hr_record_time_update();
    btn_rst_create(cont);
    img_arrow_create(cont);
    chart_hr_create(cont);
    color_bar_create(cont, color_bar_grp, ARRAY_SIZE(color_bar_grp));

    task_label_hr_update = lv_timer_create(label_hr_current_update, 1000, lv_timer_PRIO_MID, NULL);

    img_heart_anim_enable(true);

    return scr;
}

#endif
