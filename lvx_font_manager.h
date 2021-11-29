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
 * @param style font style. see LV_FT_FONT_STYLE for details.
 * @return lvgl font pointer.
 */
lv_font_t * lvx_font_create(const char * name, uint16_t size, LV_FT_FONT_STYLE style);

/**
 * create font.similar font_create().
 * @param newfont font info.see lv_ft_info_t for details.
 * @return true on success, otherwise false.
 */
bool lvx_font_create_core(lv_ft_info_t * newfont);

/**
 * destroy font.
 * @param delfont lvgl font pointer.
 */
void lvx_font_destroy(lv_font_t * delfont);

/**
 * set font find path.
 * @param path font find path.eg:"./xxx/".note:must have '/' in end.
 */
void lvx_font_base_path_set(const char * path);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LVX_FONT_MANAGER_H */
