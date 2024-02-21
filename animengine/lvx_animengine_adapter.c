/*
 * ext/animengine/lvx_animengine_adapter.h
 *
 * Copyright (C) 2023 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "lvx_animengine_adapter.h"
#include <errno.h>

#if LVX_USE_ANIMENGINE_ADAPTER

#include "lvx_property_callback_macro.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#ifdef CONFIG_LVX_ANIM_DEFAULT_PERIOD
#define LVX_ANIM_DEFAULT_PERIOD CONFIG_LVX_ANIM_DEFAULT_PERIOD
#else
#define LVX_ANIM_DEFAULT_PERIOD 33
#endif

#define LVX_OPACITY_ZOOM_NONE 0xFF
#define LVX_ANGLE_RATIO 10

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void anim_timer_cb(lv_timer_t* param)
{
    LV_UNUSED(param);
    anim_engine_handle_t handle = (anim_engine_handle_t)param->user_data;
    if (handle)
        anim_on_frame(handle, lv_tick_get());
}

static inline lv_coord_t anim_value_to_lvgl_value(const anim_value_t* v)
{
    return (lv_coord_t)v->v.fv;
}

static inline lv_color_t anim_value_to_lvgl_color(const anim_value_t* v)
{
    if (!v || v->type != ANIM_VT_COLOR) {
        return lv_color_make(255, 0, 0);
    }
    return lv_color_make(v->v.color.ch.red, v->v.color.ch.green,
        v->v.color.ch.blue);
}

static inline void lvgl_value_to_anim_value(lv_coord_t v, anim_value_t* pv)
{
    pv->type = ANIM_VT_FLOAT;
    pv->v.fv = v;
}

static inline void lvgl_color_to_anim_value(lv_color_t v, anim_value_t* pv)
{
    pv->type = ANIM_VT_COLOR;
    pv->v.color.ch.red = v.red;
    pv->v.color.ch.green = v.green;
    pv->v.color.ch.blue = v.blue;
}

static inline int anim_get_property(anim_layer_t* layer_obj, anim_value_t* v)
{
    if (!layer_obj || !(layer_obj->layer_object)) {
        return EINVAL;
    }

    lv_obj_t* obj = (lv_obj_t*)(layer_obj->layer_object);
    switch (layer_obj->property_type) {

    case ANIM_PT_WIDTH: {
        lvgl_value_to_anim_value(lv_obj_get_style_width(obj, LV_PART_MAIN), v);
        break;
    }

    case ANIM_PT_HEIGHT: {
        lvgl_value_to_anim_value(lv_obj_get_style_height(obj, LV_PART_MAIN), v);
        break;
    }

    case ANIM_PT_OPACITY: {
        switch (layer_obj->layer_type) {
        case ANIM_LT_IMAGE:
            lvgl_value_to_anim_value(lv_obj_get_style_image_opa(obj, LV_PART_MAIN), v);
            break;
        default:
            lvgl_value_to_anim_value(lv_obj_get_style_opa(obj, LV_PART_MAIN), v);
            break;
        }
        v->v.fv = v->v.fv / LVX_OPACITY_ZOOM_NONE * 1.0;
        break;
    }

    case ANIM_PT_VISIBILITY:
        // TODO
        break;

    case ANIM_PT_BACKGROUND_COLOR: {
        lvgl_color_to_anim_value(lv_obj_get_style_bg_color(obj, LV_PART_MAIN), v);
        break;
    }

    case ANIM_PT_TRANSFORM_ANCHOR_X: {
        lvgl_value_to_anim_value(
            lv_obj_get_style_transform_pivot_x(obj, LV_PART_MAIN), v);
        break;
    }

    case ANIM_PT_TRANSFORM_ANCHOR_Y: {
        lvgl_value_to_anim_value(
            lv_obj_get_style_transform_pivot_y(obj, LV_PART_MAIN), v);
        break;
    }

    case ANIM_PT_X: {
        lvgl_value_to_anim_value(lv_obj_get_style_x(obj, LV_PART_MAIN),
            v);
        break;
    }

    case ANIM_PT_Y: {
        lvgl_value_to_anim_value(lv_obj_get_style_y(obj, LV_PART_MAIN),
            v);
        break;
    }

    case ANIM_PT_TRANSFORM_SCALE: {
        if (layer_obj->layer_type == ANIM_LT_IMAGE) {
            lvgl_value_to_anim_value(lv_image_get_scale(obj), v);
        } else {
            // TODO:lvgl V9 update
            lvgl_value_to_anim_value(
                lv_obj_get_style_transform_scale_x(obj, LV_PART_MAIN), v);
        }
        v->v.fv = v->v.fv / LV_SCALE_NONE * 1.0;
        break;
    }

    case ANIM_PT_TRANSFORM_ROTATE: {
        if (layer_obj->layer_type == ANIM_LT_IMAGE) {
            lvgl_value_to_anim_value(lv_image_get_rotation(obj) / LVX_ANGLE_RATIO, v);
        } else {
            lvgl_value_to_anim_value(
                lv_obj_get_style_transform_rotation(obj, LV_PART_MAIN) / LVX_ANGLE_RATIO, v);
        }
        break;
    }
    default:
        break;
    }

    return 0;
}

static inline int anim_set_property(anim_layer_t* layer_obj,
    const anim_value_t* v)
{

    if (!layer_obj || !(layer_obj->layer_object)) {
        return EINVAL;
    }

    lv_obj_t* obj = (lv_obj_t*)(layer_obj->layer_object);
    switch (layer_obj->property_type) {

    case ANIM_PT_WIDTH: {
        lv_obj_set_style_width(obj, anim_value_to_lvgl_value(v), LV_PART_MAIN);
        break;
    }

    case ANIM_PT_HEIGHT: {
        lv_obj_set_style_height(obj, anim_value_to_lvgl_value(v), LV_PART_MAIN);
        break;
    }

    case ANIM_PT_OPACITY: {
        lv_opa_t opa = v->v.fv * LVX_OPACITY_ZOOM_NONE;
        switch (layer_obj->layer_type) {
        case ANIM_LT_IMAGE:
            lv_obj_set_style_image_opa(obj, opa, LV_PART_MAIN);
            break;
        default:
            lv_obj_set_style_opa(obj, opa, LV_PART_MAIN);
            break;
        }
        break;
    }

    case ANIM_PT_VISIBILITY:
        // TODO
        break;

    case ANIM_PT_BACKGROUND_COLOR:
        lv_obj_set_style_bg_color(obj, anim_value_to_lvgl_color(v), LV_PART_MAIN);
        break;

    case ANIM_PT_TRANSFORM_ANCHOR_X: {
        lv_obj_set_style_transform_pivot_x(obj, anim_value_to_lvgl_value(v),
            LV_PART_MAIN);
        break;
    }

    case ANIM_PT_TRANSFORM_ANCHOR_Y: {
        lv_obj_set_style_transform_pivot_y(obj, anim_value_to_lvgl_value(v),
            LV_PART_MAIN);
        break;
    }

    case ANIM_PT_X: {
        lv_obj_set_style_x(obj, anim_value_to_lvgl_value(v),
            LV_PART_MAIN);
        break;
    }

    case ANIM_PT_Y: {
        lv_obj_set_style_y(obj, anim_value_to_lvgl_value(v),
            LV_PART_MAIN);
        break;
    }

    case ANIM_PT_TRANSFORM_SCALE: {
        lv_coord_t scale = v->v.fv * LV_SCALE_NONE;
        if (scale != 0) {
            if (layer_obj->layer_type == ANIM_LT_IMAGE) {
                lv_image_set_scale(obj, scale);
            } else {
                lv_obj_set_style_transform_scale(obj, scale, LV_PART_MAIN);
            }
        }
        break;
    }

    case ANIM_PT_TRANSFORM_ROTATE: {
        if (layer_obj->layer_type == ANIM_LT_IMAGE) {
            lv_image_set_rotation(obj, anim_value_to_lvgl_value(v) * LVX_ANGLE_RATIO);
        } else {
            lv_obj_set_style_transform_rotation(obj, anim_value_to_lvgl_value(v) * LVX_ANGLE_RATIO,
                LV_PART_MAIN);
        }
        break;
    }
    default:
        break;
    }

    return 0;
}

static inline void anim_destroy(void* context)
{
    anim_context_t* ctx = (anim_context_t*)context;
    if (!ctx || ctx->timer_handle == NULL)
        return;
    lv_timer_del(ctx->timer_handle);
}

static inline void on_obj_delete_cb(lv_event_t* e)
{
    void* user_data = lv_event_get_user_data(e);
    anim_target_delete(user_data);
}

static inline bool lvx_find_event(lv_obj_t* obj, lv_event_cb_t event_cb, void* user_data, uint32_t* index)
{
    uint32_t event_cnt = lv_obj_get_event_count(obj);
    uint32_t i;
    for (i = 0; i < event_cnt; i++) {
        lv_event_dsc_t* dsc = lv_obj_get_event_dsc(obj, i);
        if (dsc->cb == event_cb && dsc->user_data == user_data && dsc->filter == LV_EVENT_DELETE) {
            *index = i;
            return true;
        }
    }

    return false;
}

static inline void anim_set_event(anim_layer_t* layer_obj, void* user_data, anim_event_type_t filter_event)
{
    if (!layer_obj || !(layer_obj->layer_object)) {
        return;
    }

    lv_obj_t* obj = (lv_obj_t*)(layer_obj->layer_object);
    uint32_t index = 0;
    LV_UNUSED(index);
    switch (filter_event) {
    case ANIM_EVENT_TARGET_DELETE:
        if (!lvx_find_event(obj, on_obj_delete_cb, user_data, &index)) {
            lv_obj_add_event(obj, on_obj_delete_cb, LV_EVENT_DELETE, user_data);
        }
        break;
    case ANIM_EVENT_ANIM_DELETE:
        if (lvx_find_event(obj, on_obj_delete_cb, user_data, &index)) {
            lv_obj_remove_event(obj, index);
        }
        break;
    default:
        break;
    }
}

_DEFINE_ANIM_PROPERTY_CB_SELECTOR_(
    style,
    img_opa,
    lv_obj_set_style_image_opa,
    lv_obj_get_style_image_opa,
    lv_opa_t,
    LVX_OPACITY_ZOOM_NONE,
    LV_PART_MAIN);

_DEFINE_ANIM_PROPERTY_CB_(
    image,
    img_angle,
    lv_image_set_rotation,
    lv_image_get_rotation,
    int16_t,
    LVX_ANGLE_RATIO);

_DEFINE_ANIM_PROPERTY_CB_(
    image,
    img_zoom,
    lv_image_set_scale,
    lv_image_get_scale,
    uint16_t,
    LV_SCALE_NONE);

_DEFINE_ANIM_PROPERTY_CB_(
    object,
    obj_x,
    lv_obj_set_x,
    lv_obj_get_x,
    lv_coord_t,
    1);

_DEFINE_ANIM_PROPERTY_CB_(
    object,
    obj_y,
    lv_obj_set_y,
    lv_obj_get_y,
    lv_coord_t,
    1);

_DEFINE_ANIM_PROPERTY_CB_SELECTOR_(
    style,
    transform_width,
    lv_obj_set_style_transform_width,
    lv_obj_get_style_transform_width,
    lv_coord_t,
    1,
    LV_PART_MAIN);

_DEFINE_ANIM_PROPERTY_CB_SELECTOR_(
    style,
    transform_height,
    lv_obj_set_style_transform_height,
    lv_obj_get_style_transform_height,
    lv_coord_t,
    1,
    LV_PART_MAIN);

_DEFINE_ANIM_PROPERTY_CB_SELECTOR_(
    style,
    translate_x,
    lv_obj_set_style_translate_x,
    lv_obj_get_style_translate_x,
    lv_coord_t,
    1,
    LV_PART_MAIN);

_DEFINE_ANIM_PROPERTY_CB_SELECTOR_(
    style,
    translate_y,
    lv_obj_set_style_translate_y,
    lv_obj_get_style_translate_y,
    lv_coord_t,
    1,
    LV_PART_MAIN);

static inline bool lvx_anim_register_property(anim_engine_handle_t handle)
{
    anim_property_callback_t property_cb;

#define _REGISTER_ANIM_PROPERTY_ENTRY_(module, style_name, check_value_cb) \
    property_cb.get_property_cb = _##module##_get_##style_name##_;         \
    property_cb.set_property_cb = _##module##_set_##style_name##_;         \
    property_cb.check_legality_cb = check_value_cb;                        \
    anim_register_property(handle, #style_name, property_cb)

    _REGISTER_ANIM_PROPERTY_ENTRY_(style, img_opa, checkOpacityLegality);
    _REGISTER_ANIM_PROPERTY_ENTRY_(image, img_angle, checkRotateLegality);
    _REGISTER_ANIM_PROPERTY_ENTRY_(image, img_zoom, checkScaleLegality);

    _REGISTER_ANIM_PROPERTY_ENTRY_(object, obj_x, checkPositionLegality);
    _REGISTER_ANIM_PROPERTY_ENTRY_(object, obj_y, checkPositionLegality);

    _REGISTER_ANIM_PROPERTY_ENTRY_(style, translate_y, checkPositionLegality);
    _REGISTER_ANIM_PROPERTY_ENTRY_(style, translate_x, checkPositionLegality);

    _REGISTER_ANIM_PROPERTY_ENTRY_(style, transform_height, checkWidthAndHeightLegality);
    _REGISTER_ANIM_PROPERTY_ENTRY_(style, transform_width, checkWidthAndHeightLegality);

#undef _REGISTER_ANIM_PROPERTY_ENTRY_
    return true;
}

void lvx_anim_unregister_property(anim_engine_handle_t handle)
{
#define _UNREGISTER_ANIM_PROPERTY_ENTRY_(module, style_name) \
    anim_unregister_property(handle, #style_name)

    _UNREGISTER_ANIM_PROPERTY_ENTRY_(style, img_opa);
    _UNREGISTER_ANIM_PROPERTY_ENTRY_(image, img_angle);
    _UNREGISTER_ANIM_PROPERTY_ENTRY_(image, img_zoom);

    _UNREGISTER_ANIM_PROPERTY_ENTRY_(object, obj_x);
    _UNREGISTER_ANIM_PROPERTY_ENTRY_(object, obj_y);

#undef _UNREGISTER_ANIM_PROPERTY_ENTRY_
}

static inline void anim_schedule_add_cb(void* user_data)
{
    anim_context_t* ctx = (anim_context_t*)user_data;
    if (!ctx || ctx->timer_handle == NULL)
        return;
    lv_timer_t* timer = (lv_timer_t*)(ctx->timer_handle);
    if (timer->paused)
        lv_timer_resume(timer);
}

static inline void anim_schedule_remove_cb(void* user_data)
{
    anim_context_t* ctx = (anim_context_t*)user_data;
    if (!ctx || ctx->timer_handle == NULL)
        return;
    lv_timer_t* timer = (lv_timer_t*)(ctx->timer_handle);
    lv_timer_pause(timer);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
anim_engine_handle_t lvx_anim_adapter_init(void)
{
    anim_context_t* anim_ctx = anim_engine_init();
    anim_ctx->get_property_cb = anim_get_property;
    anim_ctx->set_property_cb = anim_set_property;
    anim_ctx->get_tick_cb = lv_tick_get;
    anim_ctx->destroy_cb = anim_destroy;
    anim_ctx->frame_period = LVX_ANIM_DEFAULT_PERIOD;
    anim_ctx->set_event = anim_set_event;

    lv_timer_t* anim_timer = lv_timer_create(anim_timer_cb, LVX_ANIM_DEFAULT_PERIOD, anim_ctx->engine_handle);
    anim_ctx->timer_handle = anim_timer;

    lv_timer_pause(anim_timer);

    lvx_anim_register_property(anim_ctx->engine_handle);

    anim_register_event(
        anim_ctx->engine_handle,
        ANIM_EVENT_SCHEDULE_ADD,
        anim_schedule_add_cb,
        anim_ctx);

    anim_register_event(
        anim_ctx->engine_handle,
        ANIM_EVENT_SCHEDULE_REMOVE,
        anim_schedule_remove_cb,
        anim_ctx);

    return anim_ctx->engine_handle;
}

void lvx_anim_adapter_uninit(anim_engine_handle_t handle)
{
    anim_engine_destroy(handle);
}

#endif /*LVX_USE_ANIMENGINE_ADAPTER*/
