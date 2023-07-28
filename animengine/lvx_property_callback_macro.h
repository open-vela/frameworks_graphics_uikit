/*
 * ext/animengine/lvx_property_callback_macro.h
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

#ifndef __LVX_ANIMATION_ENGINE_ADAPTER_PROPERTY_CALLBACK_MACRO_H__
#define __LVX_ANIMATION_ENGINE_ADAPTER_PROPERTY_CALLBACK_MACRO_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "lv_ext_conf.h"
#include <lvgl/lvgl.h>

#if LVX_USE_ANIMENGINE_ADAPTER
#include <anim_api.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Macro Definitions
 ****************************************************************************/
#define _CHECK_CONDITION_VALIDITY                   \
    if (!layer_obj || !(layer_obj->layer_object)) { \
        return EINVAL;                              \
    }                                               \
    lv_obj_t* obj = (lv_obj_t*)(layer_obj->layer_object);

#define _DEFINE_ANIM_PROPERTY_CB_SELECTOR_(module, style_name, lvx_setter, lvx_getter, \
    data_type, ratio, selector)                                                        \
    static inline int _##module##_set_##style_name##_(anim_layer_t* layer_obj,         \
        const anim_value_t* v)                                                         \
    {                                                                                  \
        _CHECK_CONDITION_VALIDITY                                                      \
        lvx_setter(obj, (data_type)(v->v.fv * ratio), selector);                       \
        LV_LOG_INFO("obj|name|value|animV|time: %p|%s|%d|%f|%" PRId32,                 \
            obj, #style_name, (int)(v->v.fv * ratio + 0.5), v->v.fv, v->current_time); \
        return 0;                                                                      \
    }                                                                                  \
                                                                                       \
    static inline int _##module##_get_##style_name##_(anim_layer_t* layer_obj,         \
        anim_value_t* v)                                                               \
    {                                                                                  \
        _CHECK_CONDITION_VALIDITY                                                      \
        v->type = ANIM_VT_FLOAT;                                                       \
        v->v.fv = (float)lvx_getter(obj, selector) / (float)ratio;                     \
        return 0;                                                                      \
    }

#define _DEFINE_ANIM_PROPERTY_CB_(module, style_name, lvx_setter, lvx_getter,          \
    data_type, ratio)                                                                  \
    static inline int _##module##_set_##style_name##_(anim_layer_t* layer_obj,         \
        const anim_value_t* v)                                                         \
    {                                                                                  \
        _CHECK_CONDITION_VALIDITY                                                      \
        lvx_setter(obj, (data_type)(v->v.fv * ratio));                                 \
        LV_LOG_INFO("obj|name|value|animV|time: %p|%s|%d|%f|%" PRId32,                 \
            obj, #style_name, (int)(v->v.fv * ratio + 0.5), v->v.fv, v->current_time); \
        return 0;                                                                      \
    }                                                                                  \
                                                                                       \
    static inline int _##module##_get_##style_name##_(anim_layer_t* layer_obj,         \
        anim_value_t* v)                                                               \
    {                                                                                  \
        _CHECK_CONDITION_VALIDITY                                                      \
        v->type = ANIM_VT_FLOAT;                                                       \
        v->v.fv = (float)lvx_getter(obj) / (float)ratio;                               \
        return 0;                                                                      \
    }

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* LVX_USE_ANIMENGINE_ADAPTER */

#endif /* __LVX_ANIMATION_ENGINE_ADAPTER_PROPERTY_CALLBACK_MACRO_H__ */
