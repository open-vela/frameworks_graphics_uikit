/**
 * @file lvx_camera_controller.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_camera_controller.h"
#include "media_recorder.h"
#include <ext/video/lvx_video.h>
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_camera_controller_class

#define PICSINK "PictureSink"
#define VIDEOSINK "VideoSink"
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_camera_controller_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_camera_controller_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void camera_take_picture_event_cb(lv_event_t* e);
static void camera_picture_start_event_cb(lv_event_t* e);
static void camera_recorder_event_cb(lv_event_t* e);
static void anim_opa_cb(lv_timer_t* t);
static void camera_start(const char* params, const char* src);
static void camera_stop(void);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_camera_controller_class = {
    .constructor_cb = lvx_camera_controller_constructor,
    .destructor_cb = lvx_camera_controller_destructor,
    .instance_size = sizeof(lvx_camera_controller_t),
    .base_class = &lv_obj_class
};

static void* handle;
static bool is_started = false;
static lv_timer_t* timer_anim;
static lv_obj_t* red_dot;
static bool is_opa = true;
static int count = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t* lvx_camera_controller_create(lv_obj_t* parent)
{
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    LV_ASSERT_MALLOC(obj);
    if (obj == NULL)
        return NULL;
    lv_obj_class_init_obj(obj);

    lvx_camera_controller_t* camera_controller = (lvx_camera_controller_t*)obj;

    /* init */
    lv_obj_t* camera_obj = lv_obj_create(obj);
    lv_obj_remove_style_all(camera_obj);
    lv_obj_set_size(camera_obj, LV_PCT(100), LV_PCT(90));
    lv_obj_clear_flag(camera_obj, LV_OBJ_FLAG_SCROLLABLE);

    camera_controller->video = lvx_video_create(camera_obj);
    lvx_video_set_src(camera_controller->video, "Camera:");
    lvx_video_start(camera_controller->video);
    lv_obj_set_size(camera_controller->video, LV_PCT(100), LV_PCT(100));
    lv_obj_align(camera_controller->video, LV_ALIGN_CENTER, 0, 0);

    camera_controller->take_picture_btn = lv_btn_create(obj);
    lv_obj_t* take_picture_label = lv_label_create(camera_controller->take_picture_btn);
    lv_label_set_text(take_picture_label, "Photo Mode");
    lv_obj_set_size(camera_controller->take_picture_btn, LV_PCT(17), LV_PCT(6));
    lv_obj_align(camera_controller->take_picture_btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_event_cb(camera_controller->take_picture_btn, camera_take_picture_event_cb, LV_EVENT_CLICKED, camera_controller);
    lv_obj_add_flag(camera_controller->take_picture_btn, LV_OBJ_FLAG_CLICKABLE);

    camera_controller->picture_btn = lv_btn_create(obj);
    lv_obj_t* picture_label = lv_label_create(camera_controller->picture_btn);
    lv_label_set_text(picture_label, "Burst Mode");
    lv_obj_set_size(camera_controller->picture_btn, LV_PCT(17), LV_PCT(6));
    lv_obj_align(camera_controller->picture_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(camera_controller->picture_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(camera_controller->picture_btn, camera_picture_start_event_cb, LV_EVENT_CLICKED, camera_controller);
    lv_obj_add_flag(camera_controller->picture_btn, LV_OBJ_FLAG_CLICKABLE);

    camera_controller->recorder_btn = lv_btn_create(obj);
    lv_obj_t* recorder_label = lv_label_create(camera_controller->recorder_btn);
    lv_label_set_text(recorder_label, "Video Mode");
    lv_obj_set_size(camera_controller->recorder_btn, LV_PCT(17), LV_PCT(6));
    lv_obj_align(camera_controller->recorder_btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_add_flag(camera_controller->recorder_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(camera_controller->recorder_btn, camera_recorder_event_cb, LV_EVENT_CLICKED, camera_controller);
    lv_obj_add_flag(camera_controller->recorder_btn, LV_OBJ_FLAG_CLICKABLE);

    return obj;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_camera_controller_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
}

static void lvx_camera_controller_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
}

static void take_picture_event_callback(void* cookie, int event, int ret, const char* data)
{
    char* str;

    switch (event) {
    case MEDIA_EVENT_STARTED:
        str = "MEDIA_EVENT_STARTED";
        break;
    case MEDIA_EVENT_STOPPED:
        str = "MEDIA_EVENT_STOPPED";
        break;
    case MEDIA_EVENT_COMPLETED:
        str = "MEDIA_EVENT_COMPLETED";
        break;
    case MEDIA_EVENT_PREPARED:
        str = "MEDIA_EVENT_PREPARED";
        break;
    case MEDIA_EVENT_PAUSED:
        str = "MEDIA_EVENT_PAUSED";
        break;
    case MEDIA_EVENT_PREVED:
        str = "MEDIA_EVENT_PREVED";
        break;
    case MEDIA_EVENT_NEXTED:
        str = "MEDIA_EVENT_NEXTED";
        break;
    case MEDIA_EVENT_START:
        str = "MEDIA_EVENT_START";
        break;
    case MEDIA_EVENT_PAUSE:
        str = "MEDIA_EVENT_PAUSE";
        break;
    case MEDIA_EVENT_STOP:
        str = "MEDIA_EVENT_STOP";
        break;
    default:
        str = "NORMAL EVENT";
        break;
    }
    LV_LOG_INFO("event %s, event %d, ret %d\n", str, event, ret);
}

static void camera_take_picture_event_cb(lv_event_t* e)
{
    count++;
    char src[64];
    snprintf(src, sizeof(src), "/gallery/pic_%d.jpg", count);
    media_recorder_take_picture(PICSINK, src, 1, take_picture_event_callback, e);
}

static void camera_picture_start_event_cb(lv_event_t* e)
{
    lv_obj_t* obj = e->user_data;
    lvx_camera_controller_t* camera_controller = (lvx_camera_controller_t*)obj;
    if (!is_started) {
        camera_start(PICSINK, "/gallery/img_%d.jpg");
        is_started = true;
        lv_obj_clear_flag(camera_controller->take_picture_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->recorder_btn, LV_OBJ_FLAG_CLICKABLE);
    } else {
        camera_stop();
        is_started = false;
        lv_obj_add_flag(camera_controller->take_picture_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->recorder_btn, LV_OBJ_FLAG_CLICKABLE);
    }
}

static void camera_recorder_event_cb(lv_event_t* e)
{
    lv_obj_t* obj = e->user_data;
    lvx_camera_controller_t* camera_controller = (lvx_camera_controller_t*)obj;

    if (!is_started) {
        red_dot = lv_obj_create(lv_scr_act());
        lv_obj_set_size(red_dot, 15, 15);
        lv_obj_set_style_bg_color(red_dot, lv_color_hex(0xff0000), 0);
        timer_anim = lv_timer_create(anim_opa_cb, 1000, red_dot);
        lv_timer_set_repeat_count(timer_anim, -1);
        lv_timer_resume(timer_anim);

        camera_start(VIDEOSINK, "/gallery/video.mp4");
        is_started = true;
        lv_obj_clear_flag(camera_controller->take_picture_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->picture_btn, LV_OBJ_FLAG_CLICKABLE);
    } else {
        if (timer_anim) {
            camera_stop();
            lv_timer_pause(timer_anim);
            lv_timer_del(timer_anim);
            timer_anim = NULL;
            lv_obj_del(red_dot);
        }
        is_started = false;
        lv_obj_add_flag(camera_controller->take_picture_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->picture_btn, LV_OBJ_FLAG_CLICKABLE);
    }
}

static void anim_opa_cb(lv_timer_t* t)
{
    lv_obj_t* obj = t->user_data;
    if (obj) {
        is_opa ? lv_obj_set_style_bg_opa(obj, 0, 0) : lv_obj_set_style_bg_opa(obj, 255, 0);
        is_opa = !is_opa;
    }
}

static void camera_start(const char* params, const char* src)
{
    if (handle) {
        LV_LOG_ERROR("media recorder is opened!");
        return;
    }

    handle = media_recorder_open(params);

    if (!handle) {
        LV_LOG_ERROR("media recorder open failed!");
        return;
    }

    if (media_recorder_prepare(handle, src, NULL) < 0) {
        LV_LOG_ERROR("media recorder prepare failed!");
        return;
    }

    if (media_recorder_start(handle) < 0) {
        LV_LOG_ERROR("media recorder start failed!");
        return;
    }
}

static void camera_stop(void)
{
    if (!handle)
        return;
    if (media_recorder_stop(handle) < 0) {
        LV_LOG_ERROR("media recorder stop failed!");
        return;
    }
    if (media_recorder_close(handle) < 0) {
        LV_LOG_ERROR("media recorder close failed!");
        return;
    }
    handle = NULL;
}
