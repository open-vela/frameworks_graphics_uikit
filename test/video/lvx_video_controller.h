/**
 * @file lvx_video_controller.h
 *
 */

#ifndef LVX_VIDEO_CONTROLLER_H
#define LVX_VIDEO_CONTROLLER_H

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
} lvx_video_controller_t;

extern const lv_obj_class_t lvx_video_controller_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t* lvx_video_controller_create(lv_obj_t* parent);

void lvx_video_controller_set_imgbtn(lv_obj_t* obj, const void* play_img, const void* pause_img);

void lvx_video_controller_set_src_opt(lv_obj_t* obj, const char* src, const char* option);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVX_VIDEO_CONTROLLER_H*/