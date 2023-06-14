/**
 * @file font_manager.h
 *
 */
#ifndef FONT_MANAGER_MANAGER_H
#define FONT_MANAGER_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <lvgl/lvgl.h>
#include "font_config.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lv_freetype_info_t lv_freetype_info_t;
typedef struct _font_manager_t font_manager_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create main font manager.
 * @return pointer to main font manager.
 */
font_manager_t* font_manager_create(void);

/**
 * Delete main font manager.
 * @param manager pointer to main font manager.
 * @return return true if the deletion was successful.
 */
bool font_manager_delete(font_manager_t* manager);

/**
 * Set the font directory file path.
 * @param manager pointer to main font manager.
 * @param base_path file path.
 */
void font_manager_set_base_path(font_manager_t* manager, const char* base_path);

/**
 * Create font.
 * @param manager pointer to main font manager.
 * @param ft_info font info.
 * @return point to the created font
 */
lv_font_t* font_manager_create_font(font_manager_t* manager, const lv_freetype_info_t* ft_info);

/**
 * Delete font.
 * @param manager pointer to main font manager.
 * @param font point to the font.
 * @return return true if the deletion was successful.
 */
bool font_manager_delete_font(font_manager_t* manager, lv_font_t* font);

#if FONT_USE_FONT_FAMILY

/**
 * Create font-family.
 * @param manager pointer to main font manager.
 * @param ft_info fallback font information.
 * @return point to the first font of font-family
 */
lv_font_t* font_manager_create_font_family(font_manager_t* manager, const lv_freetype_info_t* ft_info);

/**
 * Delete font-family.
 * @param manager pointer to main font manager.
 * @param font point to the first font of font-family.
 */
void font_manager_delete_font_family(font_manager_t* manager, lv_font_t* font);

#endif /* FONT_USE_FONT_FAMILY */

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FONT_MANAGER_MANAGER_H */
