/**
 * @file lvx_camera_controller.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_camera_controller.h"
#include "media_recorder.h"
#ifdef CONFIG_LV_USE_QRSCAN
#include "lv_qrscan.h"
#endif

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

typedef enum {
    CAMERA_PHOTO = 0,
    CAMERA_BURST = 1,
    CAMERA_VIDEO = 2,
    CAMERA_SCAN = 3
} camera_button_e;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_camera_controller_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_camera_controller_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static inline lv_obj_t* camera_create_btn(lv_obj_t* parent, char* label_text,
    lv_align_t align, lv_coord_t align_x_ofs,
    lv_event_cb_t event_cb, bool checkable);
static void camera_take_picture_event_cb(lv_event_t* e);
static void camera_picture_start_event_cb(lv_event_t* e);
static void camera_recorder_event_cb(lv_event_t* e);
static void camera_scan_event_cb(lv_event_t* e);
static void anim_opa_cb(lv_timer_t* t);
static void camera_start(const char* params, const char* src);
static void camera_stop(void);
static void disable_buttons_exclude(camera_button_e button_mode, lv_obj_t* obj);
static void enable_buttons_exclude(camera_button_e button_mode, lv_obj_t* obj);
static void show_scan_result(char* msg_buff);
#ifdef CONFIG_LV_USE_QRSCAN
static int camera_scan(lv_img_dsc_t* img_dsc);
#endif

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

    camera_controller->photo_btn = camera_create_btn(obj, "Photo Mode", LV_ALIGN_BOTTOM_LEFT,
        0, camera_take_picture_event_cb, false);

    camera_controller->burst_btn = camera_create_btn(obj, "Burst Mode", LV_ALIGN_BOTTOM_LEFT,
        LV_PCT(28), camera_picture_start_event_cb, true);

    camera_controller->record_btn = camera_create_btn(obj, "Video Mode", LV_ALIGN_BOTTOM_LEFT,
        LV_PCT(54), camera_recorder_event_cb, true);

    camera_controller->scan_btn = camera_create_btn(obj, "Scan Mode", LV_ALIGN_BOTTOM_RIGHT,
        0, camera_scan_event_cb, false);

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

static inline lv_obj_t* camera_create_btn(lv_obj_t* parent, char* label_text,
    lv_align_t align, lv_coord_t align_x_ofs,
    lv_event_cb_t event_cb, bool checkable)
{
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_t* label = lv_label_create(btn);
    lv_obj_center(label);
    lv_label_set_text(label, label_text);
    lv_obj_set_size(btn, LV_PCT(17), LV_PCT(6));
    lv_obj_align(btn, align, align_x_ofs, 0);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, parent);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    if (checkable) {
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    }

    return btn;
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
    LV_UNUSED(str);
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
    if (!is_started) {
        camera_start(PICSINK, "/gallery/img_%d.jpg");
        is_started = true;
        disable_buttons_exclude(CAMERA_BURST, obj);
    } else {
        camera_stop();
        is_started = false;
        enable_buttons_exclude(CAMERA_BURST, obj);
    }
}

static void camera_recorder_event_cb(lv_event_t* e)
{
    lv_obj_t* obj = e->user_data;

    if (!is_started) {
        red_dot = lv_obj_create(lv_scr_act());
        lv_obj_set_size(red_dot, 15, 15);
        lv_obj_set_style_bg_color(red_dot, lv_color_hex(0xff0000), 0);
        timer_anim = lv_timer_create(anim_opa_cb, 1000, red_dot);
        lv_timer_set_repeat_count(timer_anim, -1);
        lv_timer_resume(timer_anim);

        camera_start(VIDEOSINK, "/gallery/video.mp4");
        is_started = true;
        disable_buttons_exclude(CAMERA_VIDEO, obj);
    } else {
        if (timer_anim) {
            camera_stop();
            lv_timer_pause(timer_anim);
            lv_timer_del(timer_anim);
            timer_anim = NULL;
            lv_obj_del(red_dot);
        }
        is_started = false;
        enable_buttons_exclude(CAMERA_VIDEO, obj);
    }
}

static void camera_scan_event_cb(lv_event_t* e)
{
    lv_obj_t* obj = e->user_data;
#ifdef CONFIG_LV_USE_QRSCAN
    lvx_camera_controller_t* camera_controller = (lvx_camera_controller_t*)obj;

    lv_img_dsc_t* img_dsc;

    img_dsc = lvx_video_get_img_dsc(camera_controller->video);
    if (img_dsc == NULL) {
        LV_LOG_ERROR("camera get_img_dsc failed!");
        return;
    }

    disable_buttons_exclude(CAMERA_SCAN, obj);

    camera_scan(img_dsc);

    enable_buttons_exclude(CAMERA_SCAN, obj);
#else
    disable_buttons_exclude(CAMERA_SCAN, obj);

    show_scan_result("Please Config LV_USE_QRSCAN.");

    enable_buttons_exclude(CAMERA_SCAN, obj);
#endif
}

static void anim_opa_cb(lv_timer_t* t)
{
    lv_obj_t* obj = t->user_data;
    if (obj) {
        is_opa ? lv_obj_set_style_bg_opa(obj, 0, 0) : lv_obj_set_style_bg_opa(obj, 255, 0);
        is_opa = !is_opa;
    }
}

static void disable_buttons_exclude(camera_button_e button_mode, lv_obj_t* obj)
{
    lvx_camera_controller_t* camera_controller = (lvx_camera_controller_t*)obj;

    switch (button_mode) {
    case CAMERA_BURST:
        lv_obj_clear_flag(camera_controller->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->record_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_VIDEO:
        lv_obj_clear_flag(camera_controller->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_SCAN:
        lv_obj_clear_flag(camera_controller->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(camera_controller->record_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    default:
        break;
    }
}

static void enable_buttons_exclude(camera_button_e button_mode, lv_obj_t* obj)
{
    lvx_camera_controller_t* camera_controller = (lvx_camera_controller_t*)obj;

    switch (button_mode) {
    case CAMERA_BURST:
        lv_obj_add_flag(camera_controller->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->record_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_VIDEO:
        lv_obj_add_flag(camera_controller->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_SCAN:
        lv_obj_add_flag(camera_controller->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(camera_controller->record_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    default:
        break;
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

void show_scan_result_cb(lv_event_t* e)
{
    lv_obj_t* obj = e->user_data;
    lv_obj_del(obj);
}

void show_scan_result(char* msg_buff)
{
    lv_obj_t* msg_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(msg_obj, LV_PCT(100), LV_PCT(100));
    lv_obj_align(msg_obj, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* msg_btn = lv_btn_create(msg_obj);
    lv_obj_align_to(msg_btn, msg_obj, LV_ALIGN_TOP_LEFT, LV_PCT(90), 0);
    lv_obj_t* cancel_label = lv_label_create(msg_btn);
    lv_label_set_text(cancel_label, "Quit");
    lv_obj_center(cancel_label);
    lv_obj_set_size(msg_btn, LV_PCT(10), LV_PCT(6));
    lv_obj_add_event_cb(msg_btn, show_scan_result_cb, LV_EVENT_CLICKED, msg_obj);
    lv_obj_add_flag(msg_btn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* title_label = lv_label_create(msg_obj);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text(title_label, "RESULT");

    lv_obj_t* msg_label = lv_label_create(msg_obj);
    lv_obj_align_to(msg_label, msg_obj, LV_ALIGN_TOP_LEFT, 0, LV_PCT(10));

    if (msg_buff == NULL) {
        lv_label_set_text(msg_label, "[null]");
    } else {
        lv_label_set_text(msg_label, msg_buff);
    }
}

#ifdef CONFIG_LV_USE_QRSCAN
static int camera_scan(lv_img_dsc_t* img_dsc)
{

    char* msg_buff = NULL;
    uint8_t* gray_buff;
    struct quirc* qr;
    int ret = -1;

    qr = lv_qrscan_create();
    if (!qr) {
        LV_LOG_ERROR("lv_qrscan_create error");
        return ret;
    }

    LV_LOG_INFO("\n===============================\n");

    if (img_dsc->header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {

        gray_buff = (uint8_t*)lv_mem_alloc(img_dsc->data_size / 4);
        if (!gray_buff) {
            LV_LOG_ERROR("malloc failed");
            goto ERR;
        }
        bgra8888_to_gray(img_dsc->data, img_dsc->header.w * 4, img_dsc->header.w, img_dsc->header.h, gray_buff, img_dsc->header.w);

    } else if (img_dsc->header.cf == LV_IMG_CF_RESERVED_17) {

        gray_buff = (uint8_t*)lv_mem_alloc(img_dsc->data_size);
        if (!gray_buff) {
            LV_LOG_ERROR("malloc failed");
            goto ERR;
        }
        nv12_to_gray(img_dsc->data, img_dsc->header.w, img_dsc->header.h, gray_buff);

    } else {
        show_scan_result("Can not support this color format");
        goto ERR;
    }

    ret = lv_qrscan_scan(qr, img_dsc->header.w, img_dsc->header.h, gray_buff, &msg_buff);
    LV_LOG_INFO("camera scan result: [%s]\n", msg_buff ? msg_buff : "null");
    show_scan_result(msg_buff);

    if (msg_buff) {
        lv_mem_free(msg_buff);
        msg_buff = NULL;
    }

    if (gray_buff) {
        lv_mem_free(gray_buff);
        gray_buff = NULL;
    }

ERR:
    lv_qrscan_destory(qr);

    return ret;
}
#endif