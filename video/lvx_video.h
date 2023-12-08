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

    int (*video_adapter_set_event_callback)(struct _lvx_video_vtable_t* vtable, void* ctx, void* cookie, media_event_callback event_callback);

    int (*video_adapter_get_frame)(struct _lvx_video_vtable_t* vtable, void* ctx, lvx_video_t* video);

    int (*video_adapter_get_dur)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_start)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_seek)(struct _lvx_video_vtable_t* vtable, void* ctx, int pos);

    int (*video_adapter_pause)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_resume)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_stop)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_close)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_loop)(struct _lvx_video_vtable_t* vtable, void* ctx, int loop);

    int (*video_adapter_get_player_state)(struct _lvx_video_vtable_t* vtable, void* ctx);

    int (*video_adapter_write_data)(struct _lvx_video_vtable_t* vtable, void* ctx, void* data, size_t len);
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
int lvx_video_set_event_callback(lv_obj_t* obj, void* cookie, media_event_callback event_callback);
void lvx_video_set_vtable(lv_obj_t* obj, lvx_video_vtable_t* vtable);
int lvx_video_start(lv_obj_t* obj);
int lvx_video_stop(lv_obj_t* obj);
int lvx_video_seek(lv_obj_t* obj, int pos);
int lvx_video_pause(lv_obj_t* obj);
int lvx_video_resume(lv_obj_t* obj);
int lvx_video_get_dur(lv_obj_t* obj);
int lvx_video_set_loop(lv_obj_t* obj, int loop);
void lvx_video_set_poster(lv_obj_t* obj, const char* poster_path);
bool lvx_video_is_playing(lv_obj_t* obj);
int lvx_video_write_data(lv_obj_t* obj, void* data, size_t len);
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
