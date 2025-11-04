/**
 * @file input_demo.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "uikit/uikit.h"

#ifdef CONFIG_UIKIT_DEMO_INPUT

#include "input_demo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _vg_input_context_t {
    void* user_data;
    vg_input_text_changed_callback text_changed_callback;
    vg_input_focus_lost_callback focus_lost_callback;
};

static vg_input_context_t* input_create(void* user_data);
static void input_destroy(vg_input_context_t* im);
static void input_require(vg_input_context_t* im, const char* initial_text);
static void* input_get_user_data(vg_input_context_t* im);
static void input_set_text_changed(vg_input_context_t* im, vg_input_text_changed_callback callback);
static void input_set_focus_lost(vg_input_context_t* im, vg_input_focus_lost_callback callback);

static vg_input_context_ops_t g_input_context_ops;

/* input app's properity */
static vg_input_context_t* g_focus_input_context = NULL;

#if UIKIT_INPUT_METHOD_DEFAULT_TYPE == 1
const char* buf[18] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "clean" };
static void input_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    switch (code) {
    case LV_EVENT_CLICKED:
        if (!g_focus_input_context) {
            return;
        }
        char* text = (char*)lv_event_get_user_data(e);
        if (strcmp(text, "clean") == 0) {
            g_focus_input_context->text_changed_callback(g_focus_input_context, "");
            return;
        }

        lv_obj_t* input = (lv_obj_t*)(g_focus_input_context->user_data);
        if (input == NULL) {
            return;
        }

        const char* cont = vg_input_get_text(input);
        size_t len = strlen(cont);
        char* new_str = (char*)lv_malloc_zeroed(len + 2);
        lv_memcpy(new_str, cont, len);
        strncat(new_str, text, 1);
        g_focus_input_context->text_changed_callback(g_focus_input_context, new_str);
        lv_free(new_str);
        break;
    default:
        break;
    }
}
#endif

void uikit_demo_input(char* info[], int size, void* param)
{
    g_input_context_ops.create = input_create,
    g_input_context_ops.destroy = input_destroy,
    g_input_context_ops.require = input_require,
    g_input_context_ops.get_user_data = input_get_user_data,
    g_input_context_ops.set_text_changed_callback = input_set_text_changed,
    g_input_context_ops.set_focus_lost_callback = input_set_focus_lost,

    vg_set_input_context_ops(&g_input_context_ops);

    lv_obj_t* input1 = vg_input_create(lv_screen_active());
    lv_obj_align(input1, LV_ALIGN_TOP_LEFT, 80, 80);
    lv_obj_set_style_border_width(input1, 1, 0);
    lv_obj_set_style_border_color(input1, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_border_opa(input1, LV_OPA_COVER, 0);
    vg_input_set_placeholder_text(input1, "placeholder");
    lv_obj_set_size(input1, 100, 40);
    vg_input_set_keyboard_font(input1, "MiSans-Regular");

    lv_obj_t* input2 = vg_input_create(lv_screen_active());
    lv_obj_align(input2, LV_ALIGN_TOP_RIGHT, -80, 80);
    vg_input_set_text(input2, "input text");
    lv_obj_set_style_border_width(input2, 1, 0);
    lv_obj_set_style_border_color(input2, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_border_opa(input2, LV_OPA_COVER, 0);
    lv_obj_set_size(input2, 100, 40);
    vg_input_set_keyboard_font(input2, "MiSans-Regular");

#if UIKIT_INPUT_METHOD_DEFAULT_TYPE == 1
    lv_obj_t* label = lv_label_create(lv_screen_active());
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 50);
    lv_label_set_text(label, "set text---------------------------------------------------------------------------");

    for (int i = 0; i < 18; i++) {
        lv_obj_t* btn = lv_button_create(lv_screen_active());
        lv_obj_align(btn, LV_ALIGN_CENTER, -125 + i % 6 * 50, 100 + i / 6 * 50);
        lv_obj_set_size(btn, 40, 40);
        lv_obj_t* btn_label = lv_label_create(btn);
        lv_label_set_text(btn_label, buf[i]);
        lv_obj_center(btn_label);
        lv_obj_add_event_cb(btn, input_event_cb, LV_EVENT_ALL, (void*)buf[i]);
    }
#endif
}

vg_input_context_t* input_create(void* user_data)
{
    LV_LOG_USER("input_create");

    vg_input_context_t* im = malloc(sizeof(vg_input_context_t));
    im->user_data = user_data;

    return im;
}

void input_destroy(vg_input_context_t* im)
{
    LV_LOG_USER("input_destroy");

    if (im) {
        free(im);
        im = NULL;
    }
}

void input_require(vg_input_context_t* im, const char* initial_text)
{
    LV_LOG_USER("input_require");

    g_focus_input_context = im;
}

void* input_get_user_data(vg_input_context_t* im)
{
    LV_LOG_USER("input_get_user_data");

    return im->user_data;
}

void input_set_text_changed(vg_input_context_t* im, vg_input_text_changed_callback callback)
{
    LV_LOG_USER("input_set_text_changed");

    im->text_changed_callback = callback;
}

void input_set_focus_lost(vg_input_context_t* im, vg_input_focus_lost_callback callback)
{
    LV_LOG_USER("input_set_focus_lost");

    im->focus_lost_callback = callback;
}

#endif