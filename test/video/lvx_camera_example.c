/**
 * @file lvx_camera_example.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lvx_camera_example.h"
#include "media_recorder.h"
#include "media_utils.h"
#ifdef CONFIG_LVX_USE_QRSCAN
#include <ext/qrscan/lv_qrscan.h>
#endif

#include <ext/video/lvx_video.h>
#include <stdio.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

#define PICSINK "PictureSink"
#define VIDEOSINK "VideoSink"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t* camera_ctl;
    lv_obj_t* video;
    lv_obj_t* photo_btn;
    lv_obj_t* burst_btn;
    lv_obj_t* record_btn;
    lv_obj_t* scan_btn;
    lv_obj_t* red_dot;

    lv_timer_t* timer_anim;
    const char* saved_path;
    const char* option;
    void* ui_loop;
    void* handle;

    bool is_started;
    bool is_opa;
    int count;
} camera_ctx_t;

typedef enum {
    CAMERA_PHOTO = 0,
    CAMERA_BURST = 1,
    CAMERA_VIDEO = 2,
    CAMERA_SCAN = 3
} camera_button_e;

extern char* optarg;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void camera_parse_cmd(camera_ctx_t* ctx, char* info[], int size);
static lv_obj_t* camera_create_btn(camera_ctx_t* ctx, char* label_text,
    lv_align_t align, lv_coord_t align_x_ofs,
    lv_event_cb_t event_cb, bool checkable);
static void camera_take_picture_event_cb(lv_event_t* e);
static void camera_picture_start_event_cb(lv_event_t* e);
static void camera_recorder_event_cb(lv_event_t* e);
static void camera_scan_event_cb(lv_event_t* e);
static void anim_opa_cb(lv_timer_t* t);
static void camera_start(camera_ctx_t* ctx, const char* params, const char* src);
static void camera_stop(camera_ctx_t* ctx);
static void disable_buttons_exclude(camera_ctx_t* ctx, camera_button_e button_mode);
static void enable_buttons_exclude(camera_ctx_t* ctx, camera_button_e button_mode);
static void take_picture_completed_cb(void* cookie, int ret);
static void show_scan_result(char* msg_buff);
#ifdef CONFIG_LVX_USE_QRSCAN
static int camera_scan(lv_image_dsc_t* img_dsc);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvx_example_camera(char* info[], int size, void* param)
{
    if (param == NULL) {
        LV_LOG_ERROR("the param to init ctx->ui_loop is null pointer!");
        return;
    }

    camera_ctx_t* ctx = (camera_ctx_t*)lv_malloc(sizeof(camera_ctx_t));
    if (!ctx) {
        LV_LOG_ERROR("camera ctx malloc failed");
        return;
    }

    lv_memzero(ctx, sizeof(camera_ctx_t));

    ctx->saved_path = "/gallery";
    ctx->is_started = false;
    ctx->is_opa = true;
    ctx->ui_loop = param;

    camera_parse_cmd(ctx, info, size);

    ctx->camera_ctl = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(ctx->camera_ctl);
    lv_obj_set_size(ctx->camera_ctl, LV_PCT(70), LV_PCT(70));
    lv_obj_align(ctx->camera_ctl, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* camera_obj = lv_obj_create(ctx->camera_ctl);
    lv_obj_remove_style_all(camera_obj);
    lv_obj_set_size(camera_obj, LV_PCT(100), LV_PCT(90));
    lv_obj_clear_flag(camera_obj, LV_OBJ_FLAG_SCROLLABLE);

    ctx->video = lvx_video_create(camera_obj);
    lvx_video_set_src(ctx->video, "Camera:");
    lvx_video_start(ctx->video);
    lv_obj_set_size(ctx->video, LV_PCT(100), LV_PCT(100));
    lv_obj_align(ctx->video, LV_ALIGN_CENTER, 0, 0);

    ctx->photo_btn = camera_create_btn(ctx, "Photo", LV_ALIGN_BOTTOM_LEFT,
        0, camera_take_picture_event_cb, false);

    ctx->burst_btn = camera_create_btn(ctx, "Burst", LV_ALIGN_BOTTOM_LEFT,
        LV_PCT(26), camera_picture_start_event_cb, true);

    ctx->record_btn = camera_create_btn(ctx, "Video", LV_ALIGN_BOTTOM_LEFT,
        LV_PCT(52), camera_recorder_event_cb, true);

    ctx->scan_btn = camera_create_btn(ctx, "Scan", LV_ALIGN_BOTTOM_RIGHT,
        0, camera_scan_event_cb, false);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline void camera_parse_cmd(camera_ctx_t* ctx, char* info[], int size)
{
    char ch;
    while ((ch = getopt(size, info, "hs:o:")) != -1) {
        switch (ch) {
        case 's':
            ctx->saved_path = optarg;
            break;
        case 'o':
            ctx->option = optarg;
            break;
        case 'h':
            LV_LOG("\nUsage:  lvxdemo %s [-h] -s <dir> -o <option>\n", info[0]);
            LV_LOG("-h             help\n");
            LV_LOG("-s <dir>       the directory that you want to save\n");
            LV_LOG("-o <option>    the option for preparing recorder\n");
            break;
        }
    }
}

static inline lv_obj_t* camera_create_btn(camera_ctx_t* ctx, char* label_text,
    lv_align_t align, lv_coord_t align_x_ofs,
    lv_event_cb_t event_cb, bool checkable)
{
    lv_obj_t* btn = lv_button_create(ctx->camera_ctl);
    lv_obj_t* label = lv_label_create(btn);
    lv_obj_center(label);
    lv_label_set_text(label, label_text);
    lv_obj_set_size(btn, LV_PCT(22), LV_PCT(10));
    lv_obj_align(btn, align, align_x_ofs, 0);
    lv_obj_add_event(btn, event_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    if (checkable) {
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    }

    return btn;
}

static void camera_take_picture_event_cb(lv_event_t* e)
{
    camera_ctx_t* ctx = (camera_ctx_t*)e->user_data;
    if (ctx == NULL) {
        return;
    }

    ctx->count++;
    char src[128];

    if (ctx->ui_loop == NULL) {
        LV_LOG_ERROR("uv ui_loop is null pointer!");
        return;
    }

    disable_buttons_exclude(ctx, CAMERA_PHOTO);
    snprintf(src, sizeof(src), "%s/pic_%d.jpg", ctx->saved_path, ctx->count);
    media_uv_recorder_take_picture(ctx->ui_loop, PICSINK, src, 1,
        take_picture_completed_cb, ctx);
}

static void camera_picture_start_event_cb(lv_event_t* e)
{
    camera_ctx_t* ctx = (camera_ctx_t*)e->user_data;
    if (ctx == NULL) {
        return;
    }

    char src[128];

    if (!ctx->is_started) {
        snprintf(src, sizeof(src), "%s/img_%%d.jpg", ctx->saved_path);
        camera_start(ctx, PICSINK, src);
        ctx->is_started = true;
        disable_buttons_exclude(ctx, CAMERA_BURST);
    } else {
        camera_stop(ctx);
        ctx->is_started = false;
        enable_buttons_exclude(ctx, CAMERA_BURST);
    }
}

static void camera_recorder_event_cb(lv_event_t* e)
{
    camera_ctx_t* ctx = (camera_ctx_t*)e->user_data;
    if (ctx == NULL) {
        return;
    }

    char src[128];

    if (!ctx->is_started) {
        ctx->red_dot = lv_obj_create(ctx->camera_ctl);
        lv_obj_set_size(ctx->red_dot, 15, 15);
        lv_obj_set_style_bg_color(ctx->red_dot, lv_color_hex(0xff0000), 0);
        ctx->timer_anim = lv_timer_create(anim_opa_cb, 1000, ctx);
        lv_timer_set_repeat_count(ctx->timer_anim, -1);
        lv_timer_resume(ctx->timer_anim);

        snprintf(src, sizeof(src), "%s/video.mp4", ctx->saved_path);
        camera_start(ctx, VIDEOSINK, src);
        ctx->is_started = true;
        disable_buttons_exclude(ctx, CAMERA_VIDEO);
    } else {
        if (ctx->timer_anim) {
            camera_stop(ctx);
            lv_timer_pause(ctx->timer_anim);
            lv_timer_del(ctx->timer_anim);
            ctx->timer_anim = NULL;
            lv_obj_del(ctx->red_dot);
        }
        ctx->is_started = false;
        enable_buttons_exclude(ctx, CAMERA_VIDEO);
    }
}

static void camera_scan_event_cb(lv_event_t* e)
{
    camera_ctx_t* ctx = (camera_ctx_t*)e->user_data;
    if (ctx == NULL) {
        return;
    }

#ifdef CONFIG_LVX_USE_QRSCAN

    lv_image_dsc_t* img_dsc;

    img_dsc = lvx_video_get_img_dsc(ctx->video);
    if (img_dsc == NULL) {
        LV_LOG_ERROR("camera get_img_dsc failed!");
        return;
    }

    disable_buttons_exclude(ctx, CAMERA_SCAN);

    camera_scan(img_dsc);

    enable_buttons_exclude(ctx, CAMERA_SCAN);
#else
    disable_buttons_exclude(ctx, CAMERA_SCAN);

    show_scan_result("Please Config LVX_USE_QRSCAN.");

    enable_buttons_exclude(ctx, CAMERA_SCAN);
#endif
}

static void anim_opa_cb(lv_timer_t* t)
{
    camera_ctx_t* ctx = (camera_ctx_t*)t->user_data;
    if (ctx == NULL) {
        return;
    }

    if (ctx->red_dot) {
        ctx->is_opa ? lv_obj_set_style_bg_opa(ctx->red_dot, 0, 0) : lv_obj_set_style_bg_opa(ctx->red_dot, 255, 0);
        ctx->is_opa = !ctx->is_opa;
    }
}

static void disable_buttons_exclude(camera_ctx_t* ctx, camera_button_e button_mode)
{
    if (ctx == NULL) {
        return;
    }

    switch (button_mode) {
    case CAMERA_PHOTO:
        lv_obj_clear_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->record_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_BURST:
        lv_obj_clear_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->record_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_VIDEO:
        lv_obj_clear_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_SCAN:
        lv_obj_clear_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ctx->record_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    default:
        break;
    }
}

static void enable_buttons_exclude(camera_ctx_t* ctx, camera_button_e button_mode)
{
    if (ctx == NULL) {
        LV_LOG_ERROR("camera ctx is a null pointer!");
        return;
    }

    switch (button_mode) {
    case CAMERA_PHOTO:
        lv_obj_add_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->record_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_BURST:
        lv_obj_add_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->record_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_VIDEO:
        lv_obj_add_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->scan_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    case CAMERA_SCAN:
        lv_obj_add_flag(ctx->photo_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->burst_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ctx->record_btn, LV_OBJ_FLAG_CLICKABLE);
        break;

    default:
        break;
    }
}

static void take_picture_completed_cb(void* cookie, int ret)
{
    enable_buttons_exclude((camera_ctx_t*)cookie, CAMERA_PHOTO);
    if (ret < 0) {
        LV_LOG_ERROR("take_picture error:%d", ret);
    }
}

static void camera_start(camera_ctx_t* ctx, const char* params, const char* src)
{
    if (ctx == NULL) {
        return;
    }

    const char* option = NULL;
    if (strncmp(params, VIDEOSINK, 9) == 0 && ctx->option != NULL) {
        option = ctx->option;
    }

    if (ctx->handle) {
        LV_LOG_ERROR("media recorder is opened!");
        return;
    }

    if (ctx->ui_loop == NULL) {
        LV_LOG_ERROR("uv ui_loop is null pointer!");
        return;
    }

    ctx->handle = media_uv_recorder_open(ctx->ui_loop, params, NULL, NULL);

    if (!ctx->handle) {
        LV_LOG_ERROR("media recorder open failed!");
        return;
    }

    if (media_uv_recorder_prepare(ctx->handle, src, option, NULL, NULL, NULL) < 0) {
        LV_LOG_ERROR("media recorder prepare failed!");
        return;
    }

    if (media_uv_recorder_start(ctx->handle, NULL, NULL) < 0) {
        LV_LOG_ERROR("media recorder start failed!");
        return;
    }
}

static void camera_stop(camera_ctx_t* ctx)
{
    if (ctx == NULL || !ctx->handle) {
        return;
    }

    if (media_uv_recorder_stop(ctx->handle, NULL, NULL) < 0) {
        LV_LOG_ERROR("media recorder stop failed!");
        return;
    }
    if (media_uv_recorder_close(ctx->handle, NULL) < 0) {
        LV_LOG_ERROR("media recorder close failed!");
        return;
    }
    ctx->handle = NULL;
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

    lv_obj_t* msg_btn = lv_button_create(msg_obj);
    lv_obj_align(msg_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_t* cancel_label = lv_label_create(msg_btn);
    lv_label_set_text(cancel_label, "Quit");
    lv_obj_center(cancel_label);
    lv_obj_set_size(msg_btn, LV_PCT(20), LV_PCT(10));
    lv_obj_add_event(msg_btn, show_scan_result_cb, LV_EVENT_CLICKED, msg_obj);
    lv_obj_add_flag(msg_btn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* title_label = lv_label_create(msg_obj);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text(title_label, "RESULT");

    lv_obj_t* msg_label = lv_label_create(msg_obj);
    lv_obj_align_to(msg_label, msg_obj, LV_ALIGN_TOP_LEFT, LV_PCT(10), LV_PCT(10));

    if (msg_buff == NULL) {
        lv_label_set_text(msg_label, "Please put the QR code towards the camera.");
    } else {
        lv_label_set_text(msg_label, msg_buff);
    }
}

#ifdef CONFIG_LVX_USE_QRSCAN
static int camera_scan(lv_image_dsc_t* img_dsc)
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

    if (img_dsc->header.cf == LV_COLOR_FORMAT_NATIVE_WITH_ALPHA) {

        gray_buff = (uint8_t*)lv_malloc(img_dsc->data_size / (LV_COLOR_DEPTH >> 3));
        if (!gray_buff) {
            LV_LOG_ERROR("malloc failed");
            goto ERR;
        }
        bgra8888_to_gray(img_dsc->data, img_dsc->header.w * 4, img_dsc->header.w, img_dsc->header.h, gray_buff, img_dsc->header.w);

    } else if (img_dsc->header.cf == LV_COLOR_FORMAT_NATIVE_REVERSED) {

        gray_buff = (uint8_t*)lv_malloc(img_dsc->data_size);
        if (!gray_buff) {
            LV_LOG_ERROR("malloc failed");
            goto ERR;
        }
        nv12_to_gray(img_dsc->data, img_dsc->header.w, img_dsc->header.h, gray_buff);

    } else if (img_dsc->header.cf == LV_COLOR_FORMAT_NATIVE) {

        gray_buff = (uint8_t*)lv_malloc(img_dsc->data_size / (LV_COLOR_DEPTH >> 3));

        if (!gray_buff) {
            LV_LOG_ERROR("malloc failed");
            goto ERR;
        }
        rgb565_to_gray(img_dsc->data, img_dsc->header.w, img_dsc->header.h, gray_buff);

    } else {
        show_scan_result("Can not support this color format");
        goto ERR;
    }

    ret = lv_qrscan_scan(qr, img_dsc->header.w, img_dsc->header.h, gray_buff, &msg_buff);
    LV_LOG_INFO("camera scan result: [%s]\n", msg_buff ? msg_buff : "null");
    show_scan_result(msg_buff);

    if (msg_buff) {
        lv_free(msg_buff);
        msg_buff = NULL;
    }

    if (gray_buff) {
        lv_free(gray_buff);
        gray_buff = NULL;
    }

ERR:
    lv_qrscan_destory(qr);

    return ret;
}
#endif
