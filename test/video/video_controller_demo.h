/**
 * @file video_controller_demo.h
 *
 */

#ifndef UIKIT_VIDEO_CONTROLLER_H
#define UIKIT_VIDEO_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <lvgl/lvgl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t obj;
    lv_obj_t* video;
    lv_obj_t* play_imgbtn;
    lv_obj_t* progress_slider;
    lv_obj_t* dur_label;
    const char* src;
    const char* option;
    uint8_t is_stop;
} video_controller_demo_t;

extern const lv_obj_class_t video_controller_demo_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t* uikit_demo_video_controller_create(lv_obj_t* parent);

void video_controller_demo_set_imgbtn(lv_obj_t* obj, const void* play_img, const void* pause_img);

void video_controller_demo_set_src_opt(lv_obj_t* obj, const char* src, const char* option);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UIKIT_VIDEO_CONTROLLER_H*/