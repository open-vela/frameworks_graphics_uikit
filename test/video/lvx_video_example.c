/**
 * @file lvx_video_example.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lvx_video_example.h"
#include "lvx_camera_controller.h"
#include "lvx_video_controller.h"
#include <ext/video/lvx_video.h>
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/

#define DURATION 12 * 1000

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void video_obj_free_cb(lv_timer_t* t);

static void* video_parse_cmd(char* info[], int size);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/****************************************************************************
 * lvx_example_video
 ****************************************************************************/

void lvx_example_video(char* info[], int size)
{
    char* src = video_parse_cmd(info, size);

    lv_obj_t* video = lvx_video_create(lv_scr_act());
    lv_timer_t* timer = lv_timer_create(video_obj_free_cb, DURATION, video);
    lv_timer_set_repeat_count(timer, 1);
    lv_timer_pause(timer);
    lvx_video_set_src(video, src);
    lv_timer_resume(timer);

    lv_obj_align(video, LV_ALIGN_CENTER, 0, 0);

    lvx_video_start(video);
}

/****************************************************************************
 * lvx_example_video_controller
 ****************************************************************************/

void lvx_example_video_controller(char* info[], int size)
{
    char* src = video_parse_cmd(info, size);

    lv_obj_t* obj = lvx_video_controller_create(lv_scr_act());

    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lvx_video_controller_set_src(obj, src);

    LV_IMG_DECLARE(img_play);
    LV_IMG_DECLARE(img_pause);
    lvx_video_controller_set_imgbtn(obj, &img_play, &img_pause);

    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}

/****************************************************************************
 * lvx_example_video_call
 ****************************************************************************/

void lvx_example_video_call(char* info[], int size)
{
    char* src = video_parse_cmd(info, size);
    lv_obj_t* camera_obj = lvx_video_create(lv_scr_act());
    lvx_video_set_src(camera_obj, "Camera:");
    lv_obj_set_size(camera_obj, LV_PCT(100), LV_PCT(100));

    lv_timer_t* timer1 = lv_timer_create(video_obj_free_cb, DURATION, camera_obj);
    lv_timer_set_repeat_count(timer1, 1);
    lv_timer_pause(timer1);

    lv_obj_t* video_obj = lvx_video_create(lv_scr_act());
    lv_obj_set_size(video_obj, 240, 240);
    lv_obj_set_style_border_width(video_obj, 2, 0);
    lv_obj_set_style_border_color(video_obj, lv_color_hex(0xffffff), 0);

    lvx_video_set_src(video_obj, src);

    lv_timer_t* timer = lv_timer_create(video_obj_free_cb, DURATION, video_obj);
    lv_timer_set_repeat_count(timer, 1);
    lv_timer_pause(timer);

    lvx_video_start(camera_obj);
    lvx_video_start(video_obj);

    lv_obj_align(video_obj, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_align(camera_obj, LV_ALIGN_CENTER, 0, 0);
    lv_timer_resume(timer);
    lv_timer_resume(timer1);
}

/****************************************************************************
 * lvx_example_camera
 ****************************************************************************/
void lvx_example_camera(char* info[], int size)
{
    lv_obj_t* obj = lvx_camera_controller_create(lv_scr_act());
    if (size > 0 && info) {
        lvx_camera_controller_set_url(obj, info[0]);
    }
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void video_obj_free_cb(lv_timer_t* t)
{
    lv_obj_t* obj = t->user_data;
    lvx_video_stop(obj);
    lv_obj_del(obj);
}

static void* video_parse_cmd(char* info[], int size)
{
    if (size > 0 && info) {
        return info[0];
    } else {
        return CONFIG_LVX_DEFAULT_VIDEO_PATH;
    }
}