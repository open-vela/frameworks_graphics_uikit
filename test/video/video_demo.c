/**
 * @file video_demo.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "video_demo.h"
#include "media_defs.h"
#include "video_controller_demo.h"

#include <stdio.h>
#include <uikit/uikit.h>
#include <unistd.h>

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
static void set_screen_active_style(void);
static void completed_cb(void* obj);

static void video_obj_free_cb(lv_timer_t* t);

static void video_parse_cmd(char* info[], int size, char** url, char** option);

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
 * uikit_demo_video
 ****************************************************************************/

void uikit_demo_video(char* info[], int size, void* param)
{
    LV_UNUSED(param);

    char* src = NULL;
    char* option = NULL;
    video_parse_cmd(info, size, &src, &option);

    set_screen_active_style();

    lv_obj_t* video = vg_video_create(lv_scr_act());
    lv_obj_set_size(video, LV_PCT(100), LV_PCT(100));
    vg_video_set_src_opt(video, src, option);
    vg_video_set_callback(video, MEDIA_EVENT_COMPLETED, video, completed_cb);

    lv_obj_align(video, LV_ALIGN_CENTER, 0, 0);

    vg_video_start(video);
}

/****************************************************************************
 * uikit_demo_video_controller
 ****************************************************************************/

void uikit_demo_video_controller(char* info[], int size, void* param)
{
    LV_UNUSED(param);

    char* src = NULL;
    char* option = NULL;
    video_parse_cmd(info, size, &src, &option);

    set_screen_active_style();

    lv_obj_t* obj = uikit_demo_video_controller_create(lv_scr_act());

    lv_obj_set_size(obj, LV_PCT(70), LV_PCT(70));
    video_controller_demo_set_src_opt(obj, src, option);

    LV_IMG_DECLARE(img_play);
    LV_IMG_DECLARE(img_pause);
    video_controller_demo_set_imgbtn(obj, &img_play, &img_pause);

    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}

/****************************************************************************
 * uikit_demo_video_call
 ****************************************************************************/

void uikit_demo_video_call(char* info[], int size, void* param)
{
    LV_UNUSED(param);

    char* src = NULL;
    char* option = NULL;
    video_parse_cmd(info, size, &src, &option);

    set_screen_active_style();

    lv_obj_t* camera_obj = vg_video_create(lv_scr_act());
    vg_video_set_src(camera_obj, "Camera:");
    lv_obj_set_size(camera_obj, LV_PCT(100), LV_PCT(100));

    lv_timer_t* timer1 = lv_timer_create(video_obj_free_cb, DURATION, camera_obj);
    lv_timer_set_repeat_count(timer1, 1);
    lv_timer_pause(timer1);

    lv_obj_t* video_obj = vg_video_create(lv_scr_act());
    lv_obj_set_size(video_obj, LV_PCT(30), LV_PCT(30));
    lv_obj_set_style_border_width(video_obj, 2, 0);
    lv_obj_set_style_border_color(video_obj, lv_color_hex(0xffffff), 0);

    vg_video_set_src_opt(video_obj, src, option);

    lv_timer_t* timer = lv_timer_create(video_obj_free_cb, DURATION, video_obj);
    lv_timer_set_repeat_count(timer, 1);
    lv_timer_pause(timer);

    vg_video_start(camera_obj);
    vg_video_start(video_obj);

    lv_obj_align(video_obj, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_align(camera_obj, LV_ALIGN_CENTER, 0, 0);
    lv_timer_resume(timer);
    lv_timer_resume(timer1);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline void set_screen_active_style(void)
{
    lv_obj_t* scr = lv_scr_act();
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(scr, lv_palette_lighten(LV_PALETTE_GREY, 4), 0);
    lv_obj_set_style_text_color(scr, lv_color_black(), 0);
}

static void completed_cb(void* obj)
{
    vg_video_stop(obj);
    lv_obj_del(obj);
}

static void video_obj_free_cb(lv_timer_t* t)
{
    lv_obj_t* obj = t->user_data;
    vg_video_stop(obj);
    lv_obj_del(obj);
}

static inline void video_parse_cmd(char* info[], int size, char** url, char** option)
{
    *url = CONFIG_UIKIT_DEFAULT_VIDEO_PATH;

    char ch;
    while ((ch = getopt(size, info, "hs:o:")) != -1) {
        switch (ch) {
        case 's':
            if (optarg) {
                *url = optarg;
            }
            break;
        case 'o':
            if (optarg) {
                *option = optarg;
            }
            break;
        case 'h':
            LV_LOG("\nUsage:  lvxdemo %s [-h] -s <file> -o <option>\n", info[0]);
            LV_LOG("-h             help\n");
            LV_LOG("-s <file>      the file that you want to play\n");
            LV_LOG("-o <option>    the option for preparing player\n");
            break;
        default:
            return;
        }
    }
}
