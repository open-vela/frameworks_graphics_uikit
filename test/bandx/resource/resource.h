/**
 * @file resource.h
 *
 */

#ifndef RESOURCE_H
#define RESOURCE_H

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

void resource_init(void);

const lv_font_t * resource_get_font(const char * key);

const void * resource_get_img(const char * key);

lv_style_t * resource_get_style(const char * key);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*RESOURCE_H*/
