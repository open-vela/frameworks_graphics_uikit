/**
 * @file font_log.h
 *
 */

#ifndef FONT_MANAGER_FONT_LOG_H
#define FONT_MANAGER_FONT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "font_config.h"
#include <lvgl/lvgl.h>

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

#if FONT_USE_LOG_USER
#  define FONT_LOG_INFO LV_LOG_USER
#else
#  define FONT_LOG_INFO LV_LOG_INFO
#endif

#define FONT_LOG_WARN LV_LOG_WARN
#define FONT_LOG_ERROR LV_LOG_ERROR

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* FONT_MANAGER_FONT_LOG_H */
