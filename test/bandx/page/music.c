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

#define MC_COLOR_GRAY LV_COLOR_MAKE(0x33, 0x33, 0x33)
#define MC_COLOR_PINK LV_COLOR_MAKE(0xB9, 0x00, 0x5E)
#define MC_COLOR_PINK_BG LV_COLOR_MAKE(0x17, 0x00, 0x0B)

LV_IMG_DECLARE(img_src_icon_volume_add);
LV_IMG_DECLARE(img_src_icon_volume_reduce);
LV_IMG_DECLARE(img_src_icon_start);
LV_IMG_DECLARE(img_src_icon_pause);
LV_IMG_DECLARE(img_src_icon_next);
LV_IMG_DECLARE(img_src_icon_prev);

typedef struct {
    const char * name;
    uint32_t time_ms;
} music_info_t;

#define MUSIC_TIME_TO_MS(min, sec) ((min)*60 * 1000 + (sec)*1000)

static music_info_t music_info_grp[] = {
    {.name = "Wannabe (Instrumental)", MUSIC_TIME_TO_MS(3, 37)},
    {.name = "River", MUSIC_TIME_TO_MS(3, 40)},
    {.name = "I Feel Tired", MUSIC_TIME_TO_MS(3, 25)},
};

static lv_obj_t * bar_volume;
static lv_obj_t * arc_play;
static lv_obj_t * label_time;
static lv_obj_t * label_music_info;
static lv_obj_t * btn_volume_add;
static lv_obj_t * btn_volume_reduce;
static lv_obj_t * btn_music_ctrl_next;
static lv_obj_t * btn_music_ctrl_prev;
static lv_obj_t * obj_play;
static lv_timer_t * task_music_update;

static uint32_t music_current_time = 0;
static uint32_t music_end_time = 0;
static uint16_t music_current_index = 0;
static const uint16_t music_current_index_max = ARRAY_SIZE(music_info_grp);
static bool music_is_playing = false;

static lv_auto_event_data_t ae_grp[] = {
    {&obj_play, LV_EVENT_CLICKED, 1000},
    {&btn_volume_add, LV_EVENT_CLICKED, 500},
    {&btn_volume_add, LV_EVENT_CLICKED, 500},
    {&btn_volume_reduce, LV_EVENT_CLICKED, 500},
    {&btn_volume_reduce, LV_EVENT_CLICKED, 500},
    {&btn_music_ctrl_next, LV_EVENT_CLICKED, 2000},
    {&btn_music_ctrl_next, LV_EVENT_CLICKED, 2000},
    {&obj_play, LV_EVENT_CLICKED, 1000},
    {&screen, LV_EVENT_LEAVE, 1000},
};

static void bar_volume_set_value(int16_t value)
{
    lv_bar_set_value(bar_volume, value, LV_ANIM_ON);
}

static void bar_volume_create(lv_obj_t * par)
{
    lv_obj_t * bar = lv_bar_create(par, NULL);
    lv_obj_set_size(bar, 160, 3);
    lv_obj_align(bar, NULL, LV_ALIGN_IN_TOP_MID, 0, 70);
    lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, MC_COLOR_PINK);
    lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, MC_COLOR_PINK_BG);

    bar_volume = bar;
}

static void btn_add_style(lv_obj_t * btn)
{
    static lv_style_t style = {NULL};

    if(style.map == NULL) {
        lv_style_init(&style);
        lv_style_set_bg_color(&style, LV_OBJ_PART_MAIN, MC_COLOR_GRAY);
        lv_style_set_border_width(&style, LV_OBJ_PART_MAIN, 0);
        lv_style_set_outline_width(&style, LV_OBJ_PART_MAIN, 0);
        lv_style_set_radius(&style, LV_OBJ_PART_MAIN, 10);

        lv_style_set_bg_color(&style, LV_STATE_PRESSED, LV_COLOR_WHITE);

        static lv_anim_path_t path_ease_in_out;
        lv_anim_path_init(&path_ease_in_out);
        lv_anim_path_set_cb(&path_ease_in_out, lv_anim_path_ease_in_out);

        lv_style_set_transition_path(&style, LV_STATE_PRESSED, &path_ease_in_out);
        lv_style_set_transition_time(&style, LV_STATE_DEFAULT, 50);
        lv_style_set_transition_prop_1(&style, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);
    }

    lv_obj_add_style(btn, LV_OBJ_PART_MAIN, &style);
}

static void btn_volume_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        lv_obj_t * img = lv_obj_get_child(obj, NULL);
        int16_t vol_value = lv_bar_get_value(bar_volume);

        if(lv_img_get_src(img) == &img_src_icon_volume_add) {
            bar_volume_set_value(vol_value + 10);
        }
        else {
            bar_volume_set_value(vol_value - 10);
        }
    }
}

static void btn_volume_create(lv_obj_t * par)
{
    lv_obj_t * btn1 = lv_btn_create(par, NULL);
    lv_obj_set_size(btn1, 85, 48);
    btn_add_style(btn1);
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_TOP_MID, -lv_obj_get_width(btn1) / 2 - 3, 10);
    lv_obj_set_event_cb(btn1, btn_volume_event_handler);

    lv_obj_t * btn2 = lv_btn_create(par, btn1);
    lv_obj_align(btn2, NULL, LV_ALIGN_IN_TOP_MID, lv_obj_get_width(btn2) / 2 + 3, 10);

    lv_obj_t * img1 = lv_img_create(btn1, NULL);
    lv_img_set_src(img1, &img_src_icon_volume_reduce);
    lv_obj_set_style_local_image_recolor_opa(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_image_recolor(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * img2 = lv_img_create(btn2, img1);
    lv_img_set_src(img2, &img_src_icon_volume_add);
    lv_obj_align(img2, NULL, LV_ALIGN_CENTER, 0, 0);

    btn_volume_reduce = btn1;
    btn_volume_add = btn2;
}

static void obj_play_anim_ready_callback(lv_anim_t * a_p)
{
    if(a_p->act_time == 0)
        return;

    lv_obj_t * img = (lv_obj_t *)a_p->var;
    const lv_img_dsc_t * img_src_next;
    img_src_next = (lv_img_get_src(img) == &img_src_icon_start) ? &img_src_icon_pause : &img_src_icon_start;
    lv_img_set_src(img, img_src_next);

    music_is_playing = (lv_img_get_src(img) == &img_src_icon_pause);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
    lv_anim_set_values(&a, 1800, 3600);
    lv_anim_set_time(&a, 300);

    lv_anim_path_t path;
    lv_anim_path_init(&path);
    lv_anim_path_set_cb(&path, lv_anim_path_ease_out);
    lv_anim_set_path(&a, &path);
    lv_anim_start(&a);
}

static void obj_play_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        lv_obj_t * img = lv_obj_get_child(obj, NULL);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
        lv_anim_set_ready_cb(&a, obj_play_anim_ready_callback);
        lv_anim_set_values(&a, 0, 1800);
        lv_anim_set_time(&a, 300);

        lv_anim_path_t path;
        lv_anim_path_init(&path);
        lv_anim_path_set_cb(&path, lv_anim_path_ease_in);
        lv_anim_set_path(&a, &path);
        lv_anim_start(&a);
    }
}

static void obj_play_create(lv_obj_t * par)
{
    lv_obj_t * obj = lv_obj_create(par, NULL);
    lv_obj_set_size(obj, 104, 104);
    btn_add_style(obj);
    lv_obj_set_event_cb(obj, obj_play_event_handler);
    lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, MC_COLOR_GRAY);
    lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * img = lv_img_create(obj, NULL);
    lv_img_set_src(img, &img_src_icon_start);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);

    obj_play = obj;
}

static void arc_play_create(lv_obj_t * par)
{
    lv_obj_t * arc = lv_arc_create(par, NULL);
    lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, MC_COLOR_PINK);
    lv_obj_set_style_local_line_width(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 6);

    lv_obj_set_style_local_line_color(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, MC_COLOR_PINK_BG);
    lv_obj_set_style_local_line_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 6);

    lv_obj_set_style_local_pad_all(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_opa(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_size(arc, 121, 121);
    lv_obj_align(arc, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_arc_set_start_angle(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_range(arc, 0, 1000);
    lv_arc_set_rotation(arc, 270);

    arc_play = arc;
}

static void label_time_update(uint32_t cur_ms, uint32_t end_ms)
{
    uint32_t cur_min = cur_ms / 60 / 1000;
    uint32_t cur_sec = (cur_ms / 1000) % 60;

    uint32_t end_min = end_ms / 60 / 1000;
    uint32_t end_sec = (end_ms / 1000) % 60;

    lv_label_set_text_fmt(
        label_time,
        "#B9005E %d:%02d# #666666 -- %d:%02d#",
        cur_min,
        cur_sec,
        end_min,
        end_sec);
}

static void label_time_create(lv_obj_t * par)
{
    lv_obj_t * label = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "--");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 87);
    lv_obj_set_auto_realign(label, true);

    label_time = label;
}

static void label_music_info_create(lv_obj_t * par)
{
    lv_obj_t * label = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font_bahnschrift_20);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_label_set_text(label, "");
    lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL);
    lv_label_set_anim_speed(label, 20);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(label, 166);
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 259);

    label_music_info = label;
}

static void music_change_current(int index)
{
    index %= ARRAY_SIZE(music_info_grp);

    music_current_time = 0;
    music_end_time = music_info_grp[index].time_ms;

    lv_label_set_text(label_music_info, music_info_grp[index].name);
    label_time_update(music_current_time, music_end_time);
}

static void music_change_next(bool next)
{
    if(next) {
        if(music_current_index >= music_current_index_max - 1) {
            music_current_index = 0;
        }
        else {
            music_current_index++;
        }
    }
    else {
        if(music_current_index == 0) {
            music_current_index = music_current_index_max;
        }
        else {
            music_current_index--;
        }
    }

    music_change_current(music_current_index);
}

static void btn_music_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        lv_obj_t * img = lv_obj_get_child(obj, NULL);

        bool is_next = (lv_img_get_src(img) == &img_src_icon_next);
        music_change_next(is_next);
    }
}

static void btn_music_ctrl_create(lv_obj_t * par)
{
    lv_obj_t * btn1 = lv_btn_create(par, NULL);
    lv_obj_set_size(btn1, 85, 64);
    btn_add_style(btn1);
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_BOTTOM_MID, -lv_obj_get_width(btn1) / 2 - 3, -10);
    lv_obj_set_event_cb(btn1, btn_music_event_handler);

    lv_obj_t * btn2 = lv_btn_create(par, btn1);
    lv_obj_align(btn2, NULL, LV_ALIGN_IN_BOTTOM_MID, lv_obj_get_width(btn2) / 2 + 3, -10);

    lv_obj_t * img1 = lv_img_create(btn1, NULL);
    lv_img_set_src(img1, &img_src_icon_prev);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * img2 = lv_img_create(btn2, NULL);
    lv_img_set_src(img2, &img_src_icon_next);
    lv_obj_align(img2, NULL, LV_ALIGN_CENTER, 0, 0);

    btn_music_ctrl_prev = btn1;
    btn_music_ctrl_next = btn2;
}

static void music_update(lv_timer_t * task)
{
    if(!music_is_playing) {
        return;
    }
    music_current_time += task->period;

    if(music_current_time > music_end_time) {
        music_change_next(true);
    }

    lv_arc_set_value(arc_play, music_current_time * 1000 / music_end_time);
    label_time_update(music_current_time, music_end_time);
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
        lv_timer_del(task_music_update);
    }
}

lv_obj_t * page_music_create(void)
{
    AUTO_EVENT_CREATE(ae_grp);

    lv_obj_t * scr = page_screen_create();
    screen = scr;
    lv_obj_set_event_cb(scr, page_event_handler);

    btn_volume_create(scr);
    bar_volume_create(scr);
    bar_volume_set_value(50);

    arc_play_create(scr);
    obj_play_create(arc_play);
    label_time_create(scr);
    label_music_info_create(scr);
    btn_music_ctrl_create(scr);

    music_is_playing = false;
    music_change_current(music_current_index);

    task_music_update = lv_timer_create(music_update, 100, lv_timer_PRIO_MID, NULL);

    return scr;
}

#endif
