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

static inline void _update_ext_global(void* data)
{
    LV_GLOBAL_DEFAULT()->user_data = data;
}

void lv_ext_init(void)
{
    if (LV_EXT_GLOBAL()) {
        LV_LOG_WARN("lv_ext has been initialized!");
        return;
    }

    lv_ext_global_t* data = lv_malloc(sizeof(lv_ext_global_t));
    if (data == NULL) {
        LV_LOG_WARN("lv_ext init failure!");
        return;
    }

    lv_memset(data, 0x00, sizeof(lv_ext_global_t));
    _update_ext_global(data);

#if (LVX_USE_FONT_MANAGER != 0)
    lvx_font_init();
#endif

#ifdef CONFIG_LVX_USE_VIDEO_ADAPTER
    lvx_video_adapter_init();
#endif
}

void lv_ext_deinit(void)
{
    void* data = LV_EXT_GLOBAL();

    if (data == NULL) {
        LV_LOG_WARN("lv_ext hasn't been initialized!");
        return;
    }

#if (LVX_USE_FONT_MANAGER != 0)
    lvx_font_deinit();
#endif

#ifdef CONFIG_LVX_USE_VIDEO_ADAPTER
    lvx_video_adapter_uninit();
#endif

    _update_ext_global(NULL);
    lv_free(data);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
