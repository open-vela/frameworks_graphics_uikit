#ifndef UI_KIT_H
#define UI_KIT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "uikit/uikit_conf.h"

#ifdef CONFIG_UIKIT
#include "uikit/uikit_async.h"
#include "uikit/uikit_font_manager.h"
#include "uikit/uikit_qrscan.h"
#include "uikit/uikit_vector_shapes.h"
#include "uikit/video/uikit_video.h"
#include "uikit/video/uikit_vtun.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
struct vg_font_manager_t;

typedef struct _vg_global_t {
    vg_async_t async_info;

#if (UIKIT_FONT_MANAGER != 0)
    struct vg_font_manager_t* font_manager;
#endif

#if (UIKIT_VIDEO_ADAPTER != 0)
    vg_video_vtable_t* video_vtable;
#endif

    void* user_data;
} vg_global_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
static inline vg_global_t* VG_GLOBAL_DEFAULT(void)
{
    return (vg_global_t*)(LV_GLOBAL_DEFAULT()->user_data);
}

void* vg_get_user_data(void);

void vg_set_user_data(void* data);

/**
 * uikit vg init.
 */
void vg_init(void);

void vg_deinit(void);

/**
 * uikit vg uv init.
 * @param loop          pointer to a uv loop from application layer
 */
void vg_uv_init(void* loop);

/**
 * uikit vg uv deinit.
 */
void vg_uv_deinit(void);

/**********************
 *      MACROS
 **********************/
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* UI_KIT_H */
