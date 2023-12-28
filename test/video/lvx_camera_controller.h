/**
 * @file lvx_camera_controller.h
 *
 */

#ifndef LVX_CAMERA_CONTROLLER_H
#define LVX_CAMERA_CONTROLLER_H

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
    lv_obj_t* photo_btn;
    lv_obj_t* burst_btn;
    lv_obj_t* record_btn;
    lv_obj_t* scan_btn;
} lvx_camera_controller_t;

extern const lv_obj_class_t lvx_camera_controller_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t* lvx_camera_controller_create(lv_obj_t* parent);

void lvx_camera_controller_set_url_with_option(lv_obj_t* obj, const char* url, const char* opt);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVX_CAMERA_CONTROLLER_H*/
