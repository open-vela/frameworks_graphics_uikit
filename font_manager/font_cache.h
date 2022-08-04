/**
 * @file font_cache.h
 *
 */

#ifndef FONT_MANAGER_FONT_CACHE_H
#define FONT_MANAGER_FONT_CACHE_H

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

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _font_cache_manager_t font_cache_manager_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create font cache manager.
 * @param max_size cache size.
 * @return pointer to font cache manager.
 */
font_cache_manager_t* font_cache_manager_create(uint32_t max_size);

/**
 * Delete font cache manager.
 * @param manager pointer to font cache manager.
 */
void font_cache_manager_delete(font_cache_manager_t* manager);

/**
 * Get a reusable font.
 * @param manager pointer to font cache manager.
 * @param ft_info font info.
 * @return returns true on success.
 */
bool font_cache_manager_get_reuse(font_cache_manager_t* manager, lv_ft_info_t* ft_info);

/**
 * Set fonts to be reused.
 * @param manager pointer to font cache manager.
 * @param ft_info font info.
 */
void font_cache_manager_set_reuse(font_cache_manager_t* manager, const lv_ft_info_t* ft_info);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* FONT_MANAGER_FONT_CACHE_H */
