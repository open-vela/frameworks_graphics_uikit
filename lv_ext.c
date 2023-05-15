/**
 * @file lv_ext.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_ext.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_ext_init(void)
{

#if (LVX_USE_FONT_MANAGER != 0)
    lvx_font_init();
#endif

#ifdef CONFIG_LVX_USE_VIDEO_ADAPTER
    lvx_video_adapter_init();
#endif

#ifdef CONFIG_LVX_USE_RLE
    lvx_rle_decoder_init();
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
