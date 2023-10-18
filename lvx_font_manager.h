/**
 * @file lvx_font_manager.h
 *
 */
#ifndef LVX_FONT_MANAGER_H
#define LVX_FONT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef __NuttX__
#include "lv_ext_conf.h"
#endif
#include <lvgl/lvgl.h>

#if (LVX_USE_FONT_MANAGER != 0)

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef void* lvx_font_path_handle_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize font library.
 */
void lvx_font_init(void);

/**
 * destory font library and free resource.
 */
void lvx_font_deinit(void);

/**
 * create font.
 * @param name font name.eg:"simhei".
 * @param size font size.eg:16.
 * @param style font style. see LV_FREETYPE_FONT_STYLE for details.
 * @return lvgl font pointer.
 */
lv_font_t* lvx_font_create(const char* name, uint16_t size, uint16_t style);

/**
 * destroy font.
 * @param delfont lvgl font pointer.
 */
void lvx_font_destroy(lv_font_t* delfont);

/**
 * set font default path.
 * @param path font find path.eg:"./xxx/".note:must have '/' in end.
 */
void lvx_font_set_base_path(const char* path);

/**
 * set font find path.
 * @param name font name.eg:"simhei".
 * @param path font file path.eg:"/data/simhei.ttf".
 * @return font path handle.
 */
lvx_font_path_handle_t lvx_font_add_path(const char* name, const char* path);

/**
 * remove font find path.
 * @param handle font path handle.
 * @return true if success.
 */
bool lvx_font_remove_path(lvx_font_path_handle_t handle);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LVX_FONT_MANAGER_H */
