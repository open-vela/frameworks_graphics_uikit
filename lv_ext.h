#ifndef LV_EXT_H
#define LV_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_ext_conf.h"
#include "lvx_async.h"
#include "lvx_font_manager.h"
#include "video/lvx_video.h"
#include "video/lvx_video_adapter.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
struct _font_manager_t;

typedef struct _lv_ext_global_t {
    lv_async_t async_info;
#if (LVX_USE_FONT_MANAGER != 0)
    struct _font_manager_t* font_manager;
#endif
#ifdef CONFIG_LVX_USE_VIDEO_ADAPTER
    lvx_video_vtable_t* video_vtable;
#endif
} lv_ext_global_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
static inline lv_ext_global_t* LV_EXT_GLOBAL(void)
{
    return (lv_ext_global_t*)(LV_GLOBAL_DEFAULT()->user_data);
}

/**
 * Lvgl extern init.
 */
void lv_ext_init(void);

void lv_ext_deinit(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_EXT_H */
