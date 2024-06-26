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

#include "draw/shapes/lvx_vector_shapes.h"
#include "draw/clipper/lvx_vector_clipper.h"

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
    void* user_data;
} lv_ext_global_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
static inline lv_ext_global_t* LV_EXT_GLOBAL(void)
{
    return (lv_ext_global_t*)(LV_GLOBAL_DEFAULT()->user_data);
}

void* lv_ext_get_user_data(void);

void lv_ext_set_user_data(void* data);

/**
 * Lvgl extern init.
 */
void lv_ext_init(void);

void lv_ext_deinit(void);

/**
 * Lvgl extern uv init.
 * @param loop          pointer to a uv loop from application layer
 */
void lv_ext_uv_init(void* loop);

/**
 * Lvgl extern uv deinit.
 */
void lv_ext_uv_deinit(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_EXT_H */
