/**
 * @file font_emoji.h
 *
 */

#ifndef FONT_MANAGER_FONT_EMOJI_H
#define FONT_MANAGER_FONT_EMOJI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "font_utils.h"
#include <lvgl/lvgl.h>

#if FONT_USE_EMOJI

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _font_emoji_manager_t font_emoji_manager_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create emoji font manager.
 * @param json_obj points to a json parsing object.
 * @return pointer to font emoji manager.
 */
font_emoji_manager_t* font_emoji_manager_create(font_utils_json_obj_t* json_obj);

/**
 * Delete emoji font manager.
 * @param manager pointer to font emoji manager.
 */
void font_emoji_manager_delete(font_emoji_manager_t* manager);

/**
 * Create an emoji font.
 * @param manager pointer to font emoji manager.
 * @param name emoji font name.
 * @param height emoji font height.
 * @return pointer to emoji font.
 */
lv_font_t* font_emoji_manager_create_font(font_emoji_manager_t* manager, const char* name, uint16_t height);

/**
 * Delete an emoji font.
 * @param manager pointer to font emoji manager.
 * @param font pointer to emoji font.
 */
void font_emoji_manager_delete_font(font_emoji_manager_t* manager, lv_font_t* font);

/**********************
 *      MACROS
 **********************/

#endif /* FONT_USE_EMOJI */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* FONT_MANAGER_FONT_EMOJI_H */
