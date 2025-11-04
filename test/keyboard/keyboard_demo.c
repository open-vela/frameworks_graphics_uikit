/**
 * @file keyboard_demo.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "uikit/uikit.h"

#if CONFIG_UIKIT_KEYBOARD

#include "keyboard_demo.h"

static void ta_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* ta = lv_event_get_target(e);
    lv_obj_t* kb = lv_event_get_user_data(e);
    if (code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

void uikit_demo_keyboard(char* info[], int size, void* param)
{
    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t* kb = vg_keyboard_create(lv_screen_active());
    lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);

    /*Create a text area. The keyboard will write here*/
    lv_obj_t* ta = lv_textarea_create(lv_screen_active());
    lv_obj_set_size(ta, 140, 80);

    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
    lv_textarea_set_placeholder_text(ta, "Hello world");

    vg_keyboard_set_bg_color(kb, lv_color_make(255, 0, 0), LV_OPA_COVER);

    vg_keyboard_set_key_color(kb, lv_color_make(0, 255, 0), LV_OPA_COVER, LV_STATE_DEFAULT, COMMON_KEY | ENTER_KEY);

    vg_keyboard_set_key_text(kb, "Enter", ENTER_KEY);

    vg_keyboard_set_key_text_color(kb, lv_color_make(255, 255, 255), LV_OPA_COVER, COMMON_KEY | ENTER_KEY);

    vg_keyboard_set_font(kb, "MiSans-Regular");
}

#endif /*CONFIG_UIKIT_KEYBOARD*/