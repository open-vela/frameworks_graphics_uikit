/*
 * Copyright (C) 2024 Xiaomi Corporation
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

#ifndef __UIKIT_MARKDOWN_H__
#define __UIKIT_MARKDOWN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>
#include <nuttx/config.h>

#ifdef CONFIG_UIKIT_MARKDOWN

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    VG_MARKDOWN_DECOR_NONE = 0,
    VG_MARKDOWN_DECOR_STRIKETHROUGH = 1,
    VG_MARKDOWN_DECOR_EM = 2,
    VG_MARKDOWN_DECOR_STRONG = 3,
} vg_markdown_decor_t;

typedef void (*vg_markdown_heading_style_cb_t)(lv_style_t* style, int32_t level);
typedef void (*vg_markdown_list_marker_cb_t)(char* buff, uint32_t size, int32_t level, int32_t index, bool ordered);
typedef void (*vg_markdown_thematic_break_style_cb_t)(lv_style_t* style);
typedef void (*vg_markdown_paragraph_style_cb_t)(lv_style_t* style);
typedef void (*vg_markdown_text_deco_style_cb_t)(lv_style_t* style, vg_markdown_decor_t decor);
typedef void (*vg_markdown_image_url_process_cb_t)(const char* path, char* out_path, int32_t len);
typedef void (*vg_markdown_url_create_cb_t)(lv_span_t* span, const char* src, const char* title, const char* alt);
typedef lv_obj_t* (*vg_markdown_image_create_cb_t)(lv_obj_t* parent, const char* src, const char* title,
    const char* alt, int32_t width_hint);
typedef void (*vg_markdown_unsupported_cb_t)(char* buff, uint32_t size, lv_style_t* style, int32_t type);

typedef struct {
    lv_obj_t obj;
    void* doc;
    void* ctx;

    int32_t list_marker_width;

    lv_style_t thematic_break_style;
    lv_style_t paragraph_style;
    lv_style_t url_style;

    /* Callbacks */
    vg_markdown_heading_style_cb_t heading_style_cb;
    vg_markdown_list_marker_cb_t list_marker_cb;
    vg_markdown_thematic_break_style_cb_t thematic_break_style_cb;
    vg_markdown_paragraph_style_cb_t paragraph_style_cb;
    vg_markdown_text_deco_style_cb_t text_deco_style_cb;
    vg_markdown_url_create_cb_t url_create_cb;
    vg_markdown_image_url_process_cb_t image_url_process_cb;
    vg_markdown_image_create_cb_t image_create_cb;
    vg_markdown_unsupported_cb_t unsupported_cb;
} vg_markdown_t;

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t vg_markdown_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_obj_t* vg_markdown_create(lv_obj_t* parent);

/*======================
 * Add/remove functions
 *=====================*/

/*=====================
 * Setter functions
 *====================*/

void vg_markdown_set_data(lv_obj_t* obj, const char* data, uint32_t data_len);

void vg_markdown_set_heading_style_cb(lv_obj_t* obj, vg_markdown_heading_style_cb_t cb);
void vg_markdown_set_list_marker_cb(lv_obj_t* obj, vg_markdown_list_marker_cb_t cb);
void vg_markdown_set_list_marker_width(lv_obj_t* obj, int32_t width);
void vg_markdown_set_thematic_break_style_cb(lv_obj_t* obj, vg_markdown_thematic_break_style_cb_t cb);
void vg_markdown_set_paragraph_style_cb(lv_obj_t* obj, vg_markdown_paragraph_style_cb_t cb);
void vg_markdown_set_text_deco_style_cb(lv_obj_t* obj, vg_markdown_text_deco_style_cb_t cb);
void vg_markdown_set_url_create_cb(lv_obj_t* obj, vg_markdown_url_create_cb_t cb);
void vg_markdown_set_image_url_process_cb(lv_obj_t* obj, vg_markdown_image_url_process_cb_t cb);
void vg_markdown_set_image_create_cb(lv_obj_t* obj, vg_markdown_image_create_cb_t cb);
void vg_markdown_set_unsupported_cb(lv_obj_t* obj, vg_markdown_unsupported_cb_t cb);

/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*CONFIG_UIKIT_MARKDOWN*/

#ifdef __cplusplus
}
#endif

#endif /*__UIKIT_MARKDOWN_H__*/
