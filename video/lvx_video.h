/**
 * @file lvx_video.h
 *
 */

#ifndef LVX_VIDEO_H
#define LVX_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <lvgl/lvgl.h>
#include <nuttx/config.h>

#if defined(CONFIG_LVX_USE_VIDEO_ADAPTER)

#include "media_defs.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lvx_video_vtable_t lvx_video_vtable_t;

typedef void (*video_event_callback)(void* obj);

typedef struct {
    lv_image_t img;
    lv_display_t* disp;
    lv_image_dsc_t img_dsc;
    unsigned duration;
    unsigned cur_time;
    lv_area_t crop_coords;
    void* video_ctx;
    lvx_video_vtable_t* vtable;
    lv_event_code_t custom_event_id;
} lvx_video_t;

struct _lvx_video_vtable_t {

    void* (*video_adapter_open)(struct _lvx_video_vtable_t* vtable, const char* src, const char* option);

    int (*video_adapter_get_frame)(struct _lvx_video_vtable_t* vtable, void* ctx, lvx_video_t* video);

    int (*video_adapter_get_dur)(struct _lvx_video_vtable_t* vtable, void* ctx, media_uv_unsigned_callback callback, void* cookie);

    int (*video_adapter_start)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_seek)(struct _lvx_video_vtable_t* vtable, void* ctx, int pos);

    int (*video_adapter_pause)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_resume)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_stop)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_close)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_loop)(struct _lvx_video_vtable_t* vtable, void* ctx, int loop);

    int (*video_adapter_get_playing)(struct _lvx_video_vtable_t* vtable, void* ctx, media_uv_int_callback cb, void* cookie);

    int (*video_adapter_set_callback)(struct _lvx_video_vtable_t* vtable, void* ctx, int event, void* obj, video_event_callback callback);
};

extern const lv_obj_class_t lvx_video_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lvx_video_vtable_set_default(lvx_video_vtable_t* vtable);
lvx_video_vtable_t* lvx_video_vtable_get_default(void);

lv_obj_t* lvx_video_create(lv_obj_t* parent);
void lvx_video_set_src(lv_obj_t* obj, const char* src);
void lvx_video_set_src_opt(lv_obj_t* obj, const char* src, const char* option);
void lvx_video_set_vtable(lv_obj_t* obj, lvx_video_vtable_t* vtable);
int lvx_video_start(lv_obj_t* obj);
int lvx_video_stop(lv_obj_t* obj);
int lvx_video_seek(lv_obj_t* obj, int pos);
int lvx_video_pause(lv_obj_t* obj);
int lvx_video_resume(lv_obj_t* obj);
int lvx_video_get_dur(lv_obj_t* obj, media_uv_unsigned_callback callback, void* cookie);
int lvx_video_set_loop(lv_obj_t* obj, int loop);
void lvx_video_set_align(lv_obj_t* obj, lv_image_align_t align);
void lvx_video_set_poster(lv_obj_t* obj, const char* poster_path);
void lvx_video_set_colorkey(lv_obj_t* obj, lv_color_t low, lv_color_t high);
int lvx_video_get_playing(lv_obj_t* obj, media_uv_int_callback cb, void* cookie);
int lvx_video_set_callback(lv_obj_t* obj, int event, void* ctx_obj, video_event_callback callback);
lv_image_dsc_t* lvx_video_get_img_dsc(lv_obj_t* obj);
lv_event_code_t lvx_video_get_custom_event_id(lv_obj_t* obj);

/**********************
 *      MACROS
 **********************/

#endif /* CONFIG_LVX_USE_VIDEO_ADAPTER */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVX_VIDEO_H*/
