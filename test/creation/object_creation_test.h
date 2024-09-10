/**
 * @file object_creation_test.h
 *
 */

#ifndef UIKIT_DEMO_OBJCREATION_H
#define UIKIT_DEMO_OBJCREATION_H

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
void uikit_demo_time_object_creation(char* info[], int size, void* param);

void uikit_demo_time_render_label(char* info[], int size, void* param);

void uikit_demo_time_render_spangroup(char* info[], int size, void* param);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
