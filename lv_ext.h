#ifndef LV_EXT_H
#define LV_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "bindecoder/lvx_bin_decoder.h"
#include "lv_ext_conf.h"
#include "lvx_async.h"
#include "lvx_font_manager.h"
#include "rle/lvx_rle_decoder.h"
#include "video/lvx_video.h"
#include "video/lvx_video_adapter.h"

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
 * Lvgl extern init.
 */
void lv_ext_init(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_EXT_H */
