/**
 * @file video_demo.h
 *
 */

#ifndef UIKIT_VIDEO_DEMO_H
#define UIKIT_VIDEO_DEMO_H

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

void uikit_demo_video(char* info[], int size, void* param);

void uikit_demo_video_controller(char* info[], int size, void* param);

void uikit_demo_video_call(char* info[], int size, void* param);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
