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

#ifndef __UIKIT_KEYBOARD_H__
#define __UIKIT_KEYBOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#ifdef CONFIG_UIKIT_KEYBOARD

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    COMMON_KEY = 0x01,
    CAPS_KEY1 = 0x02,
    BACK_KEY = 0x04,
    ENTER_KEY = 0x08,
    SPACE_KEY = 0x10,
    DELETE_KEY = 0x20,
} vg_keyboard_key_t;

typedef struct {
    lv_color_t bg_color;
    lv_color_t pressed_color;
    lv_state_t state;
    lv_color_t text_color;
    const char* text;
} vg_key_t;

typedef struct {
    lv_keyboard_t obj;
    vg_keyboard_key_t key_type;
    vg_key_t enter_key;
    const lv_font_t* font;
    void* delete_icon;
} vg_keyboard_t;

extern const lv_obj_class_t vg_keyboard_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a virtual keyboard object
 * @param parent pointer to a parent object
 * @return pointer to the created keyboard object
 */
lv_obj_t* vg_keyboard_create(lv_obj_t* parent);

/**
 * set keyboard backgroup color
 * @param kb pointer to the keyboard object
 * @param color the color value
 * @param opa the opacity value
 */
void vg_keyboard_set_bg_color(lv_obj_t* kb, lv_color_t color, lv_opa_t opa);

/**
 * Set the keyboard's key color
 * @param kb pointer to the keyboard object
 * @param color the color value
 * @param opa the opacity value
 * @param key the key type
 */
void vg_keyboard_set_key_color(lv_obj_t* kb, lv_color_t color, lv_opa_t opa, lv_state_t state, vg_keyboard_key_t key);

/**
 * Set the keyboard's key text
 * @param kb pointer to the keyboard object
 * @param text pointer to the text string
 * @param key the key type
 */
void vg_keyboard_set_key_text(lv_obj_t* kb, const char* text, vg_keyboard_key_t key);

/**
 * Set the keyboard's key text color
 * @param kb pointer to the keyboard object
 * @param color the color value
 * @param opa the opacity value
 * @param key the key type
 */
void vg_keyboard_set_key_text_color(lv_obj_t* kb, lv_color_t color, lv_opa_t opa, vg_keyboard_key_t key);

/**
 * Set the keyboard's font
 * @param kb pointer to the keyboard object
 * @param font_name the name pointer to the font
 */
void vg_keyboard_set_font(lv_obj_t* kb, const char* font_name);

/**
 * Set the keyboard's delete icon
 * @param kb pointer to the keyboard object
 * @param icon pointer to the delete icon
 */
void vg_keyboard_set_delete_icon(lv_obj_t* kb, void* icon);

#endif /* CONFIG_UIKIT_KEYBOARD */

#ifdef __cplusplus
}
#endif

#endif /* __UIKIT_KEYBOARD_H__ */
