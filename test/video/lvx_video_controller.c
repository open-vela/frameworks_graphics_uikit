/**
 * @file lvx_video_controller.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_video_controller.h"
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
    lv_obj_set_size(video_obj, LV_PCT(100), LV_PCT(90));
    lv_obj_clear_flag(video_obj, LV_OBJ_FLAG_SCROLLABLE);

    video_controller->video = lvx_video_create(video_obj);
    lv_obj_center(video_controller->video);
    lv_obj_add_event_cb(video_controller->video, video_event_cb, LV_EVENT_VALUE_CHANGED, video_controller);

    /* init controller widgets */
    lv_obj_t* controller_obj = lv_obj_create(obj);
    lv_obj_remove_style_all(controller_obj);
    lv_obj_set_size(controller_obj, LV_PCT(100), LV_PCT(10));
    lv_obj_clear_flag(controller_obj, LV_OBJ_FLAG_SCROLLABLE);

    /* init play btn */
    video_controller->play_imgbtn = lv_imgbtn_create(controller_obj);
    lv_obj_set_size(video_controller->play_imgbtn, LV_PCT(12), LV_PCT(100));

    /* init click event callback for play button */
    lv_obj_add_flag(video_controller->play_imgbtn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(video_controller->play_imgbtn, play_pause_event_cb, LV_EVENT_CLICKED, video_controller->video);
    lv_obj_add_flag(video_controller->play_imgbtn, LV_OBJ_FLAG_CLICKABLE);

    /* init progress slider */
    video_controller->progress_slider = lv_slider_create(controller_obj);
    lv_obj_set_size(video_controller->progress_slider, LV_PCT(76), 3);
    lv_obj_align_to(video_controller->progress_slider, video_controller->play_imgbtn, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    lv_obj_add_event_cb(video_controller->progress_slider, progress_slider_event_cb, LV_EVENT_RELEASED,
        video_controller->video);

    /* init duration label */
    video_controller->dur_label = lv_label_create(controller_obj);
    lv_label_set_text(video_controller->dur_label, "00:00/00:00");
    lv_obj_set_size(video_controller->play_imgbtn, LV_PCT(12), LV_PCT(100));
    lv_label_set_long_mode(video_controller->dur_label, LV_LABEL_LONG_CLIP);
    lv_obj_align_to(video_controller->dur_label, video_controller->progress_slider, LV_ALIGN_OUT_RIGHT_MID, 2, 0);

    return obj;
}

void lvx_video_controller_set_imgbtn(lv_obj_t* obj, const void* play_img, const void* pause_img)
{
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;

    lv_obj_t* play_imgbtn = video_controller->play_imgbtn;

    lv_imgbtn_set_src(play_imgbtn, LV_IMGBTN_STATE_RELEASED, NULL, play_img, NULL);
    lv_imgbtn_set_src(play_imgbtn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, pause_img, NULL);

    lv_img_header_t header;
    lv_img_decoder_get_info(play_img, &header);
    lv_obj_set_width(play_imgbtn, header.w);

    lv_obj_align_to(video_controller->progress_slider, video_controller->play_imgbtn, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_align_to(video_controller->dur_label, video_controller->progress_slider, LV_ALIGN_OUT_RIGHT_MID, 3, 0);
}

void lvx_video_controller_set_src(lv_obj_t* obj, const char* src)
{
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)obj;
    lvx_video_t* video = (lvx_video_t*)video_controller->video;
    lvx_video_set_src(video_controller->video, src);

    lv_slider_set_range(video_controller->progress_slider, 0, video->duration);
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

static void video_event_cb(lv_event_t* e)
{
    char dur_str[32];
    lvx_video_t* video = (lvx_video_t*)lv_event_get_target(e);
    lvx_video_controller_t* video_controller = (lvx_video_controller_t*)lv_event_get_user_data(e);

    snprintf(dur_str, sizeof(dur_str), "%02d:%02d/%02d:%02d", video->cur_time / 60, video->cur_time % 60,
        video->duration / 60, video->duration % 60);

    lv_label_set_text(video_controller->dur_label, dur_str);
    lv_slider_set_value(video_controller->progress_slider, video->cur_time, LV_ANIM_OFF);
}

static void play_pause_event_cb(lv_event_t* e)
{
    lv_obj_t* imgbtn = lv_event_get_target(e);
    lv_obj_t* video = lv_event_get_user_data(e);
    if (lv_obj_has_state(imgbtn, LV_STATE_CHECKED)) {
        lvx_video_resume(video);
    } else {
        lvx_video_pause(video);
    }
}

static void progress_slider_event_cb(lv_event_t* e)
{
    lv_obj_t* slider = lv_event_get_target(e);
    lv_obj_t* video = lv_event_get_user_data(e);

    int32_t pos = lv_slider_get_value(slider);

    lvx_video_seek(video, pos);
}
