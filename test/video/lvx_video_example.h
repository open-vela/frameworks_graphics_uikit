/**
 * @file lvx_video_example.h
 *
 */

#ifndef LVX_VIDEO_EXAMPLES_H
#define LVX_VIDEO_EXAMPLES_H

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lvx_example_video(char* info[], int size, void* param);

void lvx_example_video_controller(char* info[], int size, void* param);

void lvx_example_video_call(char* info[], int size, void* param);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
