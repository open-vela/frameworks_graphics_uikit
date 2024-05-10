/**
 * @file lvx_video.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_video.h"

#ifdef CONFIG_LVX_USE_VIDEO_ADAPTER
#include "lv_ext.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_video_class
#define LVX_VIDEO_DEFAULT_PERIOD (30)
#define g_video_default_vtable LV_EXT_GLOBAL()->video_vtable

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_video_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_video_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_video_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void video_frame_task_cb(lv_event_t* e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_video_class = {
    .constructor_cb = lvx_video_constructor,
    .destructor_cb = lvx_video_destructor,
    .event_cb = lvx_video_event,
    .instance_size = sizeof(lvx_video_t),
    .base_class = &lv_image_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lvx_video_vtable_set_default(lvx_video_vtable_t* vtable)
{
    LV_ASSERT_NULL(vtable);

    g_video_default_vtable = vtable;
}

lvx_video_vtable_t* lvx_video_vtable_get_default(void)
{
    return g_video_default_vtable;
}

lv_obj_t* lvx_video_create(lv_obj_t* parent)
{
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);

    return obj;
}

void lvx_video_set_src(lv_obj_t* obj, const char* src)
{
    lvx_video_set_src_opt(obj, src, NULL);
}

void lvx_video_set_src_opt(lv_obj_t* obj, const char* src, const char* option)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    video_obj->video_ctx = video_obj->vtable->video_adapter_open(video_obj->vtable, src, option);
}

void lvx_video_set_vtable(lv_obj_t* obj, lvx_video_vtable_t* vtable)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(vtable);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    video_obj->vtable = vtable;
}

int lvx_video_start(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    if ((ret = video_obj->vtable->video_adapter_start(video_obj->vtable, video_obj->video_ctx)) == 0) {
        lv_display_register_vsync_event(video_obj->disp, video_frame_task_cb, obj);
    }

    return ret;
}

int lvx_video_stop(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_image_cache_drop(&video_obj->img_dsc);

    if ((ret = video_obj->vtable->video_adapter_stop(video_obj->vtable, video_obj->video_ctx)) == 0) {

        video_obj->vtable->video_adapter_close(video_obj->vtable, video_obj->video_ctx);

        lv_display_unregister_vsync_event(video_obj->disp, video_frame_task_cb, obj);

        lv_memset(&video_obj->img_dsc, 0, sizeof(video_obj->img_dsc));
    }

    return ret;
}

int lvx_video_seek(lv_obj_t* obj, int pos)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_seek(video_obj->vtable, video_obj->video_ctx, pos);
}

int lvx_video_pause(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    if ((ret = video_obj->vtable->video_adapter_pause(video_obj->vtable, video_obj->video_ctx)) == 0) {
        lv_display_unregister_vsync_event(video_obj->disp, video_frame_task_cb, obj);
    }

    return ret;
}

int lvx_video_resume(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    if ((ret = video_obj->vtable->video_adapter_resume(video_obj->vtable, video_obj->video_ctx)) == 0) {
        lv_display_register_vsync_event(video_obj->disp, video_frame_task_cb, obj);
    }

    return ret;
}

int lvx_video_get_dur(lv_obj_t* obj, media_uv_unsigned_callback callback, void* cookie)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_get_dur(video_obj->vtable, video_obj->video_ctx, callback, cookie);
}

int lvx_video_set_loop(lv_obj_t* obj, int loop)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_loop(video_obj->vtable, video_obj->video_ctx, loop);
}

int lvx_video_set_callback(lv_obj_t* obj, int event, void* ctx_obj, video_event_callback callback)
{

    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_set_callback(video_obj->vtable, video_obj->video_ctx, event, ctx_obj, callback);
}

void lvx_video_set_align(lv_obj_t* obj, lv_image_align_t align)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_image_set_align(&video_obj->img.obj, align);
}

void lvx_video_set_poster(lv_obj_t* obj, const char* poster_path)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_image_set_src(&video_obj->img.obj, poster_path);
}

void lvx_video_set_colorkey(lv_obj_t* obj, lv_color_t low, lv_color_t high)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    static lv_image_colorkey_t colorkey;
    colorkey.low = low;
    colorkey.high = high;
    lv_obj_set_style_image_colorkey(&video_obj->img.obj, &colorkey, 0);
}

int lvx_video_get_playing(lv_obj_t* obj, media_uv_int_callback cb, void* cookie)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_get_playing(video_obj->vtable, video_obj->video_ctx, cb, cookie);
}

lv_image_dsc_t* lvx_video_get_img_dsc(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return &video_obj->img_dsc;
}

lv_event_code_t lvx_video_get_custom_event_id(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->custom_event_id;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_video_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    video_obj->vtable = g_video_default_vtable;

    LV_ASSERT_NULL(video_obj->vtable);

    video_obj->disp = lv_display_get_default();
    video_obj->custom_event_id = lv_event_register_id();
}

static void lvx_video_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_image_cache_drop(&video_obj->img_dsc);

    video_obj->vtable->video_adapter_close(video_obj->vtable, video_obj->video_ctx);

    lv_display_unregister_vsync_event(video_obj->disp, video_frame_task_cb, obj);
}

static void lvx_video_set_crop(lvx_video_t* video_obj)
{
    if (video_obj->crop_coords.x1 || video_obj->crop_coords.x2 || video_obj->crop_coords.y1 || video_obj->crop_coords.y2) {
        int crop_width = video_obj->img_dsc.header.w - (video_obj->crop_coords.x1 + video_obj->crop_coords.x2);
        int crop_height = video_obj->img_dsc.header.h - (video_obj->crop_coords.y1 + video_obj->crop_coords.y2);

        video_obj->img.w = crop_width;
        video_obj->img.h = crop_height;

        if (video_obj->crop_coords.x1) {
            lv_img_set_offset_x(&video_obj->img.obj, -video_obj->crop_coords.x1);
            lv_obj_set_style_translate_x(&video_obj->img.obj, video_obj->crop_coords.x1, 0);
        }

        if (video_obj->crop_coords.y1) {
            lv_img_set_offset_y(&video_obj->img.obj, -video_obj->crop_coords.y1);
            lv_obj_set_style_translate_y(&video_obj->img.obj, video_obj->crop_coords.y1, 0);
        }
    }
}

static void lvx_video_frame_scale(lvx_video_t* video_obj)
{
    int32_t scale_y = LV_SCALE_NONE, scale_x = LV_SCALE_NONE, scale = LV_SCALE_NONE;

    int32_t width = lv_obj_get_width(&video_obj->img.obj);
    int32_t height = lv_obj_get_height(&video_obj->img.obj);

    if (width <= 0 || height <= 0) {
        return;
    }

    if (height > video_obj->img.h && width > video_obj->img.w) {
        lv_obj_set_size(&video_obj->img.obj, video_obj->img.w, video_obj->img.h);
    }

    if (width < video_obj->img.w) {
        scale_x = LV_SCALE_NONE * width / video_obj->img.w;
    }

    if (height < video_obj->img.h) {
        scale_y = LV_SCALE_NONE * height / video_obj->img.h;
    }

    scale = LV_MIN(scale_x, scale_y);
    if (scale != LV_SCALE_NONE) {
        lv_image_set_scale(&video_obj->img.obj, scale);
    }
}

static void video_frame_task_cb(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_user_data(e);
    lvx_video_t* video_obj = (lvx_video_t*)obj;
    int32_t last_frame_time = video_obj->cur_time;
    bool first_frame = video_obj->img_dsc.data == NULL ? true : false;

    if (video_obj->vtable->video_adapter_get_frame(video_obj->vtable, video_obj->video_ctx, video_obj) < 0) {
        return;
    }

    if (first_frame) {
        lv_image_set_src(&video_obj->img.obj, &video_obj->img_dsc);
        lvx_video_set_crop(video_obj);
        lvx_video_frame_scale(video_obj);
        lv_obj_send_event(obj, video_obj->custom_event_id, NULL);
    } else {
        lv_image_cache_drop(&video_obj->img_dsc);
        lv_obj_invalidate(obj);
    }

    if (video_obj->cur_time != last_frame_time) {
        lv_obj_send_event((lv_obj_t*)video_obj, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void lvx_video_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);

    lv_result_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RESULT_OK)
        return;
}

#endif /* CONFIG_LVX_USE_VIDEO_ADAPTER */
