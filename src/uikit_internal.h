#ifndef UIKIT_INTERNAL_H
#define UIKIT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uikit/uikit.h"

typedef struct _vg_global_t {
    vg_async_t async_info;

#if UIKIT_FONT_MANAGER
    struct vg_font_manager_t* font_manager;
#endif

#if UIKIT_VIDEO_ADAPTER
    vg_video_vtable_t* video_vtable;
#endif

    void* user_data;
} vg_global_t;

static inline vg_global_t* VG_GLOBAL_DEFAULT(void)
{
    return (vg_global_t*)(LV_GLOBAL_DEFAULT()->user_data);
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* UIKIT_INTERNAL_H */
