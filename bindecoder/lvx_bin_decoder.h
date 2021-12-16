/**
 * @file lvx_bin_decoder.h
 *
 */

#ifndef LV_BIN_DECODER_H
#define LV_BIN_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "lv_ext_conf.h"

#if LVX_USE_BIN_DECODER

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
 * Register the BIN decoder functions in LittlevGL
 */
void lvx_bin_decoder_init(void);

/**********************
 *      MACROS
 **********************/

#endif /*LVX_USE_BIN*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_BIN_DECODER_H*/
