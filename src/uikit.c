/**
 * @file lv_ext.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "uikit/uikit.h"
#include "video/video_adapter.h"

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

void vg_init(void)
{
    if (VG_GLOBAL_DEFAULT()) {
        LV_LOG_WARN("lv_ext has been initialized!");
        return;
    }

    vg_global_t* data = lv_malloc(sizeof(vg_global_t));
    if (data == NULL) {
        LV_LOG_WARN("lv_ext init failure!");
        return;
    }

    lv_memset(data, 0x00, sizeof(vg_global_t));
    _update_ext_global(data);

#if (UIKIT_FONT_MANAGER != 0)
    vg_font_init();
#endif

#ifdef CONFIG_UIKIT_VIDEO_ADAPTER
    vg_video_adapter_init();
#endif
}

void vg_deinit(void)
{
    void* data = VG_GLOBAL_DEFAULT();

    if (data == NULL) {
        LV_LOG_WARN("lv_ext hasn't been initialized!");
        return;
    }

    vg_async_refr_deinit();

#if (UIKIT_FONT_MANAGER != 0)
    vg_font_deinit();
#endif

#ifdef CONFIG_UIKIT_VIDEO_ADAPTER
    vg_video_adapter_uninit();
#endif

    _update_ext_global(NULL);
    lv_free(data);
}

void* vg_get_user_data(void)
{
    return VG_GLOBAL_DEFAULT()->user_data;
}

void vg_set_user_data(void* data)
{
    VG_GLOBAL_DEFAULT()->user_data = data;
}

void vg_uv_deinit(void)
{
#ifdef CONFIG_UIKIT_VIDEO_ADAPTER
    vg_video_adapter_loop_deinit();
#endif
}

void vg_uv_init(void* loop)
{
#ifdef CONFIG_UIKIT_VIDEO_ADAPTER
    vg_video_adapter_loop_init(loop);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
