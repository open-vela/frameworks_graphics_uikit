/**
 * @file lvx_theme_extended.h
 *
 */

#ifndef LVX_THEME_EXTENDED_H
#define LVX_THEME_EXTENDED_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_ext_conf.h"

#if LVX_USE_THEME_EXTENDED != 0

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

/**
 * Initialize the theme
 * @return a pointer to reference this theme later
 */
lv_theme_t* lvx_theme_extended_init(lv_disp_t* disp);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVX_THEME_EXTENDED_H*/
