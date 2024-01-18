/**
 * @file lvx_video_controller.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_video_controller.h"
#include "media_defs.h"
#include <ext/video/lvx_video.h>
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_video_controller_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_video_controller_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_video_controller_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void video_event_cb(lv_event_t* e);
static void play_pause_event_cb(lv_event_t* e);
static void progress_slider_event_cb(lv_event_t* e);
static void duration_received_cb(void* obj, int ret, unsigned duration);
static void prepared_cb(void* obj);
static void completed_cb(void* obj);
static void stopped_cb(void* obj);
static void video_controller_set_callback(lvx_video_controller_t* video_controller);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_video_controller_class = {
    .constructor_cb = lvx_video_controller_constructor,
    .destructor_cb = lvx_video_controller_destructor,
    .instance_size = sizeof(lvx_video_controller_t),
    .base_class = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t* lvx_video_controller_create(lv_obj_t* parent)
{
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    LV_ASSERT_MALLOC(obj);
    if (obj == NULL)
        return NULL;
    lv_obj_class_init_obj(obj);

    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);

    /* init video */
    lv_obj_t* video_obj = lv_obj_create(obj);
    lv_obj_remove_style_all(video_obj);
    lv_obj_set_size(video_obj, LV_PCT(100), LV_PCT(85));
    lv_obj_clear_flag(video_obj, LV_OBJ_FLAG_SCROLLABLE);

    video_controller->video = lvx_video_create(video_obj);
    lv_obj_set_size(video_controller->video, LV_PCT(100), LV_PCT(100));
    lv_obj_center(video_controller->video);
    lv_obj_add_event(video_controller->video, video_event_cb, LV_EVENT_VALUE_CHANGED, video_controller);

    /* init controller widgets */
    lv_obj_t* controller_obj = lv_obj_create(obj);
    lv_obj_remove_style_all(controller_obj);
    lv_obj_set_size(controller_obj, LV_PCT(100), LV_PCT(15));
    lv_obj_clear_flag(controller_obj, LV_OBJ_FLAG_SCROLLABLE);

    /* init play btn */
    video_controller->play_imgbtn = lv_imgbtn_create(controller_obj);
    lv_obj_set_size(video_controller->play_imgbtn, LV_PCT(12), LV_PCT(100));

    /* init click event callback for play button */
    lv_obj_add_flag(video_controller->play_imgbtn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event(video_controller->play_imgbtn, play_pause_event_cb, LV_EVENT_ALL, video_controller);
    lv_obj_add_flag(video_controller->play_imgbtn, LV_OBJ_FLAG_CLICKABLE);

    /* init progress slider */
    video_controller->progress_slider = lv_slider_create(controller_obj);

    lv_obj_set_size(video_controller->progress_slider, LV_PCT(60), 6);
    lv_obj_set_style_pad_all(video_controller->progress_slider, 8, LV_PART_KNOB);

    lv_obj_add_event(video_controller->progress_slider, progress_slider_event_cb, LV_EVENT_RELEASED,
        video_controller->video);

    /* init duration label */
    video_controller->dur_label = lv_label_create(controller_obj);
    lv_label_set_text(video_controller->dur_label, "00:00/00:00");
    lv_label_set_long_mode(video_controller->dur_label, LV_LABEL_LONG_CLIP);

    return obj;
}

void lvx_video_controller_set_imgbtn(lv_obj_t* obj, const void* play_img, const void* pause_img)
{
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;

    lv_obj_t* play_imgbtn = video_controller->play_imgbtn;

    lv_imgbtn_set_src(play_imgbtn, LV_IMGBTN_STATE_RELEASED, NULL, play_img, NULL);
    lv_imgbtn_set_src(play_imgbtn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, pause_img, NULL);

    lv_image_header_t header;
    lv_image_decoder_get_info(play_img, &header);
    lv_obj_set_width(play_imgbtn, header.w);

    lv_obj_align(video_controller->play_imgbtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_align_to(video_controller->progress_slider, video_controller->play_imgbtn, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    lv_obj_align_to(video_controller->dur_label, video_controller->progress_slider, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
}

void lvx_video_controller_set_src_opt(lv_obj_t* obj, const char* src, const char* option)
{
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;
    video_controller->src = src;
    video_controller->option = option;
    video_controller->is_stop = true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_video_controller_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
}

static void lvx_video_controller_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
}

static inline void video_controller_set_callback(lvx_video_controller_t* video_controller)
{
    lvx_video_set_callback(video_controller->video, MEDIA_EVENT_PREPARED, video_controller, prepared_cb);
    lvx_video_set_callback(video_controller->video, MEDIA_EVENT_COMPLETED, video_controller, completed_cb);
    lvx_video_set_callback(video_controller->video, MEDIA_EVENT_STOPPED, video_controller, stopped_cb);
}

static void video_event_cb(lv_event_t* e)
{
    char dur_str[32];
    lvx_video_t* video = (lvx_video_t*)lv_event_get_target(e);
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)lv_event_get_user_data(e);

    snprintf(dur_str, sizeof(dur_str), "%02u:%02u/%02u:%02u", video->cur_time / 60, video->cur_time % 60,
        video->duration / 60, video->duration % 60);

    lv_label_set_text(video_controller->dur_label, dur_str);
    lv_slider_set_value(video_controller->progress_slider, video->cur_time, LV_ANIM_OFF);
}

static void play_pause_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lvx_video_controller_t* video_controller = lv_event_get_user_data(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        if (lv_obj_has_state(video_controller->play_imgbtn, LV_STATE_CHECKED)) {
            if (video_controller->is_stop) {
                lvx_video_set_src_opt(video_controller->video, video_controller->src, video_controller->option);
                video_controller_set_callback(video_controller);
                video_controller->is_stop = false;
            }
            lvx_video_resume(video_controller->video);
        } else {
            lvx_video_pause(video_controller->video);
        }
    } else if (code == LV_EVENT_LONG_PRESSED) {
        if (!video_controller->is_stop) {
            lvx_video_stop(video_controller->video);
            video_controller->is_stop = true;
        } else {
            lv_imgbtn_set_state(video_controller->play_imgbtn, LV_IMGBTN_STATE_CHECKED_RELEASED);
        }
    }
}

static void progress_slider_event_cb(lv_event_t* e)
{
    lv_obj_t* slider = lv_event_get_target(e);
    lv_obj_t* video = lv_event_get_user_data(e);

    int32_t pos = lv_slider_get_value(slider);

    lvx_video_seek(video, pos);
}

static void duration_received_cb(void* obj, int ret, unsigned duration)
{
    LV_UNUSED(ret);

    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;
    lvx_video_t* video = (lvx_video_t*)video_controller->video;

    video->duration = duration / 1000;
    lv_slider_set_range(video_controller->progress_slider, 0, video->duration);
}

static void prepared_cb(void* obj)
{
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;
    lvx_video_get_dur(video_controller->video, duration_received_cb, video_controller);
}

static void completed_cb(void* obj)
{
    char dur_str[32];
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;
    lvx_video_t* video = (lvx_video_t*)video_controller->video;

    snprintf(dur_str, sizeof(dur_str), "%02u:%02u/%02u:%02u", video->duration / 60, video->duration % 60,
        video->duration / 60, video->duration % 60);
    lv_label_set_text(video_controller->dur_label, dur_str);
    lv_slider_set_value(video_controller->progress_slider, video->duration, LV_ANIM_OFF);

    lv_imgbtn_set_state(video_controller->play_imgbtn, LV_IMGBTN_STATE_RELEASED);
    lvx_video_pause(video_controller->video);
    lvx_video_seek(video_controller->video, 0);
}

static void stopped_cb(void* obj)
{
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;

    lv_label_set_text(video_controller->dur_label, "00:00/00:00");
    lv_slider_set_value(video_controller->progress_slider, 0, LV_ANIM_OFF);
    lv_imgbtn_set_state(video_controller->play_imgbtn, LV_IMGBTN_STATE_CHECKED_RELEASED);
}