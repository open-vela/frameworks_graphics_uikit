/**
 * @file lv_png.h
 *
 */

#ifndef LV_RLE_DECODER_H
#define LV_RLE_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "lv_ext_conf.h"

#if LVX_USE_RLE

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
 * Register the RLE decoder functions in LittlevGL
 */
void lvx_rle_decoder_init(void);

void lvx_rle_decoder_custom_init(lv_img_decoder_t * dec);

/**********************
 *      MACROS
 **********************/

#endif /*LVX_USE_RLE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_RLE_DECODER_H*/
