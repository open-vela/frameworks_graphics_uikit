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

#if defined(CONFIG_UIKIT_FONT_LIB_PATH)
#define UIKIT_FONT_LIB_PATH CONFIG_UIKIT_FONT_LIB_PATH
#else
#define UIKIT_FONT_LIB_PATH "/"
#endif

/* FONT_NAME */

#if defined(CONFIG_UIKIT_FONT_NAME_MAX)
#define UIKIT_FONT_NAME_MAX CONFIG_UIKIT_FONT_NAME_MAX
#else
#define UIKIT_FONT_NAME_MAX 32
#endif

/* FONT_EXT */

#if defined(CONFIG_UIKIT_FONT_EXT_NAME)
#define UIKIT_FONT_EXT_NAME CONFIG_UIKIT_FONT_EXT_NAME
#else
#define UIKIT_FONT_EXT_NAME "ttf"
#endif

#if defined(CONFIG_UIKIT_FONT_EXT_MAX)
#define UIKIT_FONT_EXT_MAX CONFIG_UIKIT_FONT_EXT_MAX
#else
#define UIKIT_FONT_EXT_MAX 8
#endif

/* FONT_CACHE */

#if defined(CONFIG_UIKIT_FONT_CACHE_SIZE)
#define UIKIT_FONT_CACHE_SIZE CONFIG_UIKIT_FONT_CACHE_SIZE
#else
#define UIKIT_FONT_CACHE_SIZE 8
#endif

/* FONT_FAMILY */

#if defined(CONFIG_UIKIT_FONT_USE_FONT_FAMILY)
#define UIKIT_FONT_USE_FONT_FAMILY CONFIG_UIKIT_FONT_USE_FONT_FAMILY
#else
#define UIKIT_FONT_USE_FONT_FAMILY 0
#endif

#if defined(CONFIG_UIKIT_FONT_CONFIG_FILE_PATH)
#define UIKIT_FONT_CONFIG_FILE_PATH CONFIG_UIKIT_FONT_CONFIG_FILE_PATH
#else
#define UIKIT_FONT_CONFIG_FILE_PATH "/etc/font_config.json"
#endif

/* FONT_EMOJI */

#if defined(CONFIG_UIKIT_FONT_USE_EMOJI)
#define UIKIT_FONT_USE_EMOJI CONFIG_UIKIT_FONT_USE_EMOJI
#else
#define UIKIT_FONT_USE_EMOJI 0
#endif

#if defined(CONFIG_UIKIT_FONT_EMOJI_HEADER)
#define UIKIT_FONT_EMOJI_HEADER CONFIG_UIKIT_FONT_EMOJI_HEADER
#else
#define UIKIT_FONT_EMOJI_HEADER "Emoji"
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
