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

/*********************
 *      INCLUDES
 *********************/
#include "uikit/uikit_keyboard.h"
#include "uikit/uikit_font_manager.h"
#include <math.h>
#include <string.h>

#ifdef CONFIG_UIKIT_KEYBOARD

LV_IMG_DECLARE(image_delete_icon);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#define MY_CLASS &vg_keyboard_class

/**********************
 *   MACROS TYPEDEFS
 **********************/
#ifdef CONFIG_INPUT_KEYBOARD_HEIGHT
#define KEYBAORD_DEFAULT_HEIGHT CONFIG_INPUT_KEYBOARD_HEIGHT
#else
#define KEYBAORD_DEFAULT_HEIGHT 260
#endif
#ifdef CONFIG_INPUT_KEYBOARD_FONT_SIZE
#define KEYBAORD_DEFAULT_FONT_SIZE CONFIG_INPUT_KEYBOARD_FONT_SIZE
#else
#define KEYBAORD_DEFAULT_FONT_SIZE 20
#endif
#define KEYBAORD_DEFAULT_TEXT_COLOR 0xAAAAAA
#define KEYBAORD_DEFAULT_ITEM_BACKGROUND_COLOR 0x333436
#define KEYBAORD_DEFAULT_ITEM_PRESSED_BACKGROUND_COLOR 0x262729
#define KEYBAORD_DEFAULT_ENETR_KEY_BACKGROUND_COLOR 0x3482FF
#define KEYBAORD_DEFAULT_ENETR_KEY_PRESSED_BACKGROUND_COLOR 0x3482FF
#define KEYBAORD_DEFAULT_BACKGROUND_COLOR 0x19191B
#define KEYBAORD_DEFAULT_ENTER_KEY_TEXT "连接"

/**********************
 *  STATIC PROTOTYPES
 **********************/

static const char* kb_map_lc[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    " ", "a", "s", "d", "f", "g", "h", "j", "k", "l", " ", "\n",
    "ABC", " ", "z", "x", "c", "v", "b", "n", "m", " ", " ", "\n",
    ".?123", " ", " ", NULL
};
static const char* kb_map_uc[] = {
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",
    " ", "A", "S", "D", "F", "G", "H", "J", "K", "L", " ", "\n",
    "abc", " ", "Z", "X", "C", "V", "B", "N", "M", " ", " ", "\n",
    ".?123", " ", " ", NULL
};
static const char* kb_map_spec[] = {
    "[", "]", "{", "}", "#", "%", "^", "*", "+", "=", "\n",
    "_", "\\", "|", "~", "<", ">", "€", "£", "￥", "·", "\n",
    ".?123", " ", ".", ",", "?", "!", "'", " ", " ", "\n",
    "abc", " ", " ", NULL
};
static const char* kb_map_num[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "-", "/", ":", ";", "(", ")", "$", "&", "@", "\"", "\n",
    "#+=", " ", ".", ",", "?", "!", "'", " ", " ", "\n",
    "abc", " ", " ", NULL
};

/*Set the relative width of the buttons and other controls*/
static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    LV_BTNMATRIX_CTRL_HIDDEN | 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, LV_BTNMATRIX_CTRL_HIDDEN | 2,
    8, LV_BTNMATRIX_CTRL_HIDDEN | 1, 6, 6, 6, 6, 6, 6, 6, LV_BTNMATRIX_CTRL_HIDDEN | 1, LV_BTNMATRIX_CTRL_CUSTOM_2 | 8,
    2, 4, LV_BTNMATRIX_CTRL_CUSTOM_1 | LV_KEYBOARD_CTRL_BUTTON_FLAGS | 2
};
static const lv_buttonmatrix_ctrl_t kb_ctrl_spec_map[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    8, LV_BTNMATRIX_CTRL_HIDDEN | 1, 8, 8, 8, 8, 8, LV_BTNMATRIX_CTRL_HIDDEN | 1, LV_BTNMATRIX_CTRL_CUSTOM_2 | 8,
    2, 4, LV_BTNMATRIX_CTRL_CUSTOM_1 | LV_KEYBOARD_CTRL_BUTTON_FLAGS | 2
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void vg_keyboard_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void vg_keyboard_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void vg_keyboard_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void vg_keyboard_value_changed(lv_event_t* e);

static int calcMapItemCount(lv_obj_t* obj);
/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t vg_keyboard_class = {
    .base_class = &lv_keyboard_class,
    .constructor_cb = vg_keyboard_constructor,
    .destructor_cb = vg_keyboard_destructor,
    .event_cb = vg_keyboard_event,
    .instance_size = sizeof(vg_keyboard_t)
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t* vg_keyboard_create(lv_obj_t* parent)
{
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

static void vg_keyboard_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    vg_keyboard_t* keyboard = (vg_keyboard_t*)obj;
#ifdef CONFIG_UIKIT_FONT_USE_LV_FONT_DEFAULT
    keyboard->font = LV_FONT_DEFAULT;
#endif /* CONFIG_UIKIT_FONT_USE_LV_FONT_DEFAULT */
    keyboard->delete_icon = (void*)&image_delete_icon;
    keyboard->enter_key.text = KEYBAORD_DEFAULT_ENTER_KEY_TEXT;
    keyboard->enter_key.text_color = lv_color_hex(KEYBAORD_DEFAULT_TEXT_COLOR);
    keyboard->enter_key.bg_color = lv_color_hex(KEYBAORD_DEFAULT_ENETR_KEY_BACKGROUND_COLOR);
    keyboard->enter_key.pressed_color = lv_color_hex(KEYBAORD_DEFAULT_ENETR_KEY_PRESSED_BACKGROUND_COLOR);

    lv_obj_set_height(obj, KEYBAORD_DEFAULT_HEIGHT);

    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    lv_keyboard_set_map(obj, LV_KEYBOARD_MODE_TEXT_LOWER, kb_map_lc, kb_ctrl);
    lv_keyboard_set_map(obj, LV_KEYBOARD_MODE_TEXT_UPPER, kb_map_uc, kb_ctrl);
    lv_keyboard_set_map(obj, LV_KEYBOARD_MODE_SPECIAL, kb_map_spec, kb_ctrl_spec_map);
    lv_keyboard_set_map(obj, LV_KEYBOARD_MODE_NUMBER, kb_map_num, kb_ctrl_spec_map);
    lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_TEXT_LOWER);

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_hex(KEYBAORD_DEFAULT_BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(obj, lv_color_hex(KEYBAORD_DEFAULT_ITEM_BACKGROUND_COLOR), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(obj, lv_color_hex(KEYBAORD_DEFAULT_ITEM_PRESSED_BACKGROUND_COLOR), LV_PART_ITEMS | LV_STATE_PRESSED);

    lv_obj_set_style_radius(obj, 8, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(obj, 8, LV_PART_MAIN);

    lv_obj_set_style_pad_row(obj, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_column(obj, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(obj, 10, LV_PART_MAIN);

    lv_obj_set_style_text_color(obj, lv_color_hex(KEYBAORD_DEFAULT_TEXT_COLOR), LV_PART_ITEMS);

    for (int i = 0; i < lv_obj_get_event_count(obj); i++) {
        lv_event_dsc_t* dsc = lv_obj_get_event_dsc(obj, i);
        if (dsc->filter == LV_EVENT_VALUE_CHANGED) {
            lv_obj_remove_event_dsc(obj, dsc);
        }
    }
    lv_obj_add_event(obj, vg_keyboard_value_changed, LV_EVENT_VALUE_CHANGED, NULL);
}

static void vg_keyboard_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    vg_keyboard_t* keyboard = (vg_keyboard_t*)obj;
    if (keyboard) {
        vg_font_destroy((lv_font_t*)keyboard->font);
    }
}

static void vg_keyboard_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
    vg_keyboard_t* keyboard = (vg_keyboard_t*)obj;
    if (code == LV_EVENT_DRAW_TASK_ADDED) {
        lv_draw_task_t* draw_task = (lv_draw_task_t*)lv_event_get_param(e);
        if (draw_task->type != LV_DRAW_TASK_TYPE_FILL) {
            return;
        }

        lv_draw_rect_dsc_t* rect_draw_dsc = (lv_draw_rect_dsc_t*)draw_task->draw_dsc;
        if (rect_draw_dsc->base.part == LV_PART_ITEMS
            && (lv_buttonmatrix_has_button_ctrl(obj, rect_draw_dsc->base.id1, LV_BTNMATRIX_CTRL_CUSTOM_1)
                || lv_buttonmatrix_has_button_ctrl(obj, rect_draw_dsc->base.id1, LV_BTNMATRIX_CTRL_CUSTOM_2))) {

            const char* buf = NULL;
            const lv_font_t* font = NULL;
            lv_color_t bg_color;
            lv_color_t text_color;
            lv_opa_t bg_opa;

            if (lv_buttonmatrix_has_button_ctrl(obj, rect_draw_dsc->base.id1, LV_BTNMATRIX_CTRL_CUSTOM_1)) {
                buf = keyboard->enter_key.text;
                text_color = keyboard->enter_key.text_color;
                font = keyboard->font;
                bg_opa = LV_OPA_COVER;
                uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj);
                if (btn_id == calcMapItemCount(obj) - 1 && keyboard->enter_key.state == LV_STATE_PRESSED) {
                    bg_color = keyboard->enter_key.pressed_color;
                } else {
                    bg_color = keyboard->enter_key.bg_color;
                }
            } else {
                bg_color = lv_obj_get_style_bg_color(obj, LV_PART_ITEMS);
                bg_opa = LV_OPA_TRANSP;
            }

            lv_point_t text_size;
            lv_text_get_size(&text_size, buf, font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = draw_task->area.x1 + lv_area_get_width(&draw_task->area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y1 = draw_task->area.y1 + lv_area_get_height(&draw_task->area) / 2 - text_size.y / 2;
            txt_area.y2 = txt_area.y1 + text_size.y;

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = bg_color;
            rect_dsc.bg_opa = bg_opa;
            rect_dsc.radius = lv_obj_get_style_radius(obj, LV_PART_ITEMS);
            lv_draw_rect(rect_draw_dsc->base.layer, &rect_dsc, &draw_task->area);

            if (buf) {
                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                label_dsc.color = text_color;
                label_dsc.opa = LV_OPA_COVER;
                label_dsc.font = font;
                label_dsc.text = buf;
                label_dsc.text_local = 1;
                lv_draw_label(rect_draw_dsc->base.layer, &label_dsc, &txt_area);
            } else {
                lv_image_header_t header;
                lv_result_t res = lv_image_decoder_get_info(keyboard->delete_icon, &header);
                if (res != LV_RESULT_OK) {
                    return;
                }
                lv_area_t image_area;
                image_area.x1 = draw_task->area.x1 + lv_area_get_width(&draw_task->area) / 2 - header.w / 4;
                image_area.x2 = image_area.x1 + header.w - 1;
                image_area.y1 = draw_task->area.y1 + lv_area_get_height(&draw_task->area) / 2 - header.h / 4;
                image_area.y2 = image_area.y1 + header.h - 1;

                lv_draw_image_dsc_t img_draw_dsc;
                lv_draw_image_dsc_init(&img_draw_dsc);
                img_draw_dsc.src = keyboard->delete_icon;
                img_draw_dsc.scale_x = LV_ZOOM_NONE / 2;
                img_draw_dsc.scale_y = LV_ZOOM_NONE / 2;
                lv_draw_image(rect_draw_dsc->base.layer, &img_draw_dsc, &image_area);
            }
        }
    } else if (code == LV_EVENT_RELEASED) {
        keyboard->enter_key.state = LV_STATE_DEFAULT;
    } else if (code == LV_EVENT_PRESSED) {
        keyboard->enter_key.state = LV_STATE_PRESSED;
    }

    /*Call the ancestor's event handler*/
    lv_result_t res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RESULT_OK) {
        return;
    }
}

static void vg_keyboard_value_changed(lv_event_t* e)
{
    lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
    uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj);
    if (btn_id == LV_BUTTONMATRIX_BUTTON_NONE) {
        return;
    }

    lv_keyboard_t* kb = (lv_keyboard_t*)obj;

    if (lv_buttonmatrix_has_button_ctrl(obj, btn_id, LV_BTNMATRIX_CTRL_CUSTOM_2)) {
        lv_textarea_delete_char(kb->ta);
        return;
    }

    if (btn_id == calcMapItemCount(obj) - 1) {
        lv_obj_send_event(kb->ta, LV_EVENT_READY, NULL);
        return;
    }

    const char* txt = lv_buttonmatrix_get_button_text(obj, btn_id);
    if (txt == NULL) {
        return;
    }

    if (strcmp(txt, ".?123") == 0) {
        lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_NUMBER);
    } else if (strcmp(txt, "#+=") == 0) {
        lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_SPECIAL);
    } else if (strcmp(txt, "abc") == 0) {
        lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_TEXT_LOWER);
    } else if (strcmp(txt, "ABC") == 0) {
        lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_TEXT_UPPER);
    } else if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
        lv_textarea_delete_char(kb->ta);
    } else {
        lv_textarea_add_text(kb->ta, txt);
    }
}

void vg_keyboard_set_bg_color(lv_obj_t* kb, lv_color_t color, lv_opa_t opa)
{
    lv_obj_set_style_bg_opa(kb, opa, LV_PART_MAIN);
    lv_obj_set_style_bg_color(kb, color, LV_PART_MAIN);
}

void vg_keyboard_set_key_color(lv_obj_t* kb, lv_color_t color, lv_opa_t opa, lv_state_t state, vg_keyboard_key_t key)
{
    vg_keyboard_t* keyboard = (vg_keyboard_t*)kb;
    if (keyboard == NULL) {
        return;
    }

    if (key & ENTER_KEY) {
        if (state == LV_STATE_PRESSED) {
            keyboard->enter_key.pressed_color = color;
        } else {
            keyboard->enter_key.bg_color = color;
        }
    }

    if (key & COMMON_KEY) {
        lv_obj_set_style_bg_color(kb, color, LV_PART_ITEMS | state);
        lv_obj_set_style_bg_opa(kb, opa, LV_PART_ITEMS | state);
    }
}

void vg_keyboard_set_key_text(lv_obj_t* kb, const char* text, vg_keyboard_key_t key)
{
    if (!(key & ENTER_KEY)) {
        return;
    }

    vg_keyboard_t* keyboard = (vg_keyboard_t*)kb;
    if (keyboard == NULL) {
        return;
    }

    keyboard->enter_key.text = text;

    lv_obj_invalidate(kb);
}

void vg_keyboard_set_key_text_color(lv_obj_t* kb, lv_color_t color, lv_opa_t opa, vg_keyboard_key_t key)
{
    vg_keyboard_t* keyboard = (vg_keyboard_t*)kb;
    if (keyboard == NULL) {
        return;
    }

    if (key & ENTER_KEY) {
        keyboard->enter_key.text_color = color;
    }

    if (key & COMMON_KEY) {
        lv_obj_set_style_text_color(kb, color, LV_PART_ITEMS);
        lv_obj_set_style_text_opa(kb, opa, LV_PART_ITEMS);
    }
}

void vg_keyboard_set_font(lv_obj_t* kb, const char* font_name)
{
    vg_keyboard_t* keyboard = (vg_keyboard_t*)kb;
    if (keyboard == NULL) {
        return;
    }

    keyboard->font = vg_font_create(font_name, KEYBAORD_DEFAULT_FONT_SIZE, LV_FREETYPE_FONT_STYLE_NORMAL);

    lv_obj_set_style_text_font(kb, keyboard->font, LV_PART_MAIN);
}

void vg_keyboard_set_delete_icon(lv_obj_t* kb, void* icon)
{
    vg_keyboard_t* keyboard = (vg_keyboard_t*)kb;
    if (keyboard == NULL) {
        return;
    }

    keyboard->delete_icon = icon;
    lv_obj_invalidate(kb);
}

int calcMapItemCount(lv_obj_t* obj)
{
    char** obj_map = (char**)lv_btnmatrix_get_map(obj);
    int c = 0;
    while (*obj_map) {
        if (strcmp(*obj_map, "\n") != 0) {
            c++;
        }
        obj_map++;
    }
    return c;
}

#endif /*CONFIG_UIKIT_KEYBOARD*/