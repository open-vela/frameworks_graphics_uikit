/**
 * @file uikit_video.h
 *
 */

#ifndef UIKIT_VIDEO_H
#define UIKIT_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <lvgl/lvgl.h>
#include <nuttx/config.h>

#if defined(CONFIG_UIKIT_VIDEO_ADAPTER)

#include "media_defs.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _vg_video_vtable_t vg_video_vtable_t;

typedef void (*video_event_callback)(void* obj);

typedef struct {
    lv_image_t img;
    lv_display_t* disp;
    lv_image_dsc_t img_dsc;
    unsigned duration;
    unsigned cur_time;
    lv_area_t crop_coords;
    void* video_ctx;
    vg_video_vtable_t* vtable;
    lv_event_code_t custom_event_id;
} vg_video_t;

struct _vg_video_vtable_t {

    void* (*video_adapter_open)(struct _vg_video_vtable_t* vtable, const char* src, const char* option);

    int (*video_adapter_get_frame)(struct _vg_video_vtable_t* vtable, void* ctx, vg_video_t* video);

    int (*video_adapter_get_dur)(struct _vg_video_vtable_t* vtable, void* ctx, media_uv_unsigned_callback callback, void* cookie);

    int (*video_adapter_start)(struct _vg_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_seek)(struct _vg_video_vtable_t* vtable, void* ctx, int pos);

    int (*video_adapter_pause)(struct _vg_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_resume)(struct _vg_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_stop)(struct _vg_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_close)(struct _vg_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_loop)(struct _vg_video_vtable_t* vtable, void* ctx, int loop);

    int (*video_adapter_get_playing)(struct _vg_video_vtable_t* vtable, void* ctx, media_uv_int_callback cb, void* cookie);

    int (*video_adapter_set_callback)(struct _vg_video_vtable_t* vtable, void* ctx, int event, void* obj, video_event_callback callback);
};

extern const lv_obj_class_t vg_video_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void vg_video_vtable_set_default(vg_video_vtable_t* vtable);
vg_video_vtable_t* vg_video_vtable_get_default(void);

lv_obj_t* vg_video_create(lv_obj_t* parent);
void vg_video_set_src(lv_obj_t* obj, const char* src);
void vg_video_set_src_opt(lv_obj_t* obj, const char* src, const char* option);
void vg_video_set_vtable(lv_obj_t* obj, vg_video_vtable_t* vtable);
int vg_video_start(lv_obj_t* obj);
int vg_video_stop(lv_obj_t* obj);
int vg_video_seek(lv_obj_t* obj, int pos);
int vg_video_pause(lv_obj_t* obj);
int vg_video_resume(lv_obj_t* obj);
int vg_video_get_dur(lv_obj_t* obj, media_uv_unsigned_callback callback, void* cookie);
int vg_video_set_loop(lv_obj_t* obj, int loop);
void vg_video_set_align(lv_obj_t* obj, lv_image_align_t align);
void vg_video_set_poster(lv_obj_t* obj, const char* poster_path);
void vg_video_set_colorkey(lv_obj_t* obj, lv_color_t low, lv_color_t high);
int vg_video_get_playing(lv_obj_t* obj, media_uv_int_callback cb, void* cookie);
int vg_video_set_callback(lv_obj_t* obj, int event, void* ctx_obj, video_event_callback callback);
lv_image_dsc_t* vg_video_get_img_dsc(lv_obj_t* obj);
lv_event_code_t vg_video_get_custom_event_id(lv_obj_t* obj);

/**********************
 *      MACROS
 **********************/

#endif /* CONFIG_UIKIT_VIDEO_ADAPTER */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UIKIT_VIDEO_H*/
