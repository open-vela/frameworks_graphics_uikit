/**
 * @file font_config.h
 *
 */

#ifndef FONT_MANAGER_FONT_CONFIG_H
#define FONT_MANAGER_FONT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <lvgl/lvgl.h>

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/* FONT_PATH */

#if defined(CONFIG_FONT_LIB_PATH)
#define FONT_LIB_PATH CONFIG_FONT_LIB_PATH
#else
#define FONT_LIB_PATH "/"
#endif

/* FONT_NAME */

#if defined(CONFIG_FONT_NAME_MAX)
#define FONT_NAME_MAX CONFIG_FONT_NAME_MAX
#else
#define FONT_NAME_MAX 32
#endif

/* FONT_EXT */

#if defined(CONFIG_FONT_EXT_NAME)
#define FONT_EXT_NAME CONFIG_FONT_EXT_NAME
#else
#define FONT_EXT_NAME "ttf"
#endif

#if defined(CONFIG_FONT_EXT_MAX)
#define FONT_EXT_MAX CONFIG_FONT_EXT_MAX
#else
#define FONT_EXT_MAX 8
#endif

/* FONT_CACHE */

#if defined(CONFIG_FONT_CACHE_SIZE)
#define FONT_CACHE_SIZE CONFIG_FONT_CACHE_SIZE
#else
#define FONT_CACHE_SIZE 8
#endif

/* FONT_FAMILY */

#if defined(CONFIG_FONT_USE_FONT_FAMILY)
#define FONT_USE_FONT_FAMILY CONFIG_FONT_USE_FONT_FAMILY
#else
#define FONT_USE_FONT_FAMILY 0
#endif

#if defined(CONFIG_FONT_CONFIG_FILE_PATH)
#define FONT_CONFIG_FILE_PATH CONFIG_FONT_CONFIG_FILE_PATH
#else
#define FONT_CONFIG_FILE_PATH "/etc/font_config.json"
#endif

/* FONT_EMOJI */

#if defined(CONFIG_FONT_USE_EMOJI)
#define FONT_USE_EMOJI CONFIG_FONT_USE_EMOJI
#else
#define FONT_USE_EMOJI 0
#endif

#if defined(CONFIG_FONT_EMOJI_HEADER)
#define FONT_EMOJI_HEADER CONFIG_FONT_EMOJI_HEADER
#else
#define FONT_EMOJI_HEADER "Emoji"
#endif

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

#endif /* FONT_MANAGER_FONT_CONFIG_H */
