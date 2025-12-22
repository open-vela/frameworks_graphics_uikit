/**
 * @file vg_input.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "../uikit_internal.h"

/*********************
 *      DEFINES
 *********************/

#if UIKIT_INPUT_METHOD

#define MY_CLASS (&vg_input_class)

/**********************
 *  GLOBAL VARIABLES
 **********************/
#define g_input_context_ops VG_GLOBAL_DEFAULT()->input_context_ops

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void vg_input_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void vg_input_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void vg_input_event(const lv_obj_class_t* class_p, lv_event_t* e);

static void vg_input_text_changed(vg_input_context_t* im, const char* text);
static void vg_input_focus_lost(vg_input_context_t* im);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t vg_input_class = {
    .base_class = &lv_textarea_class,
    .constructor_cb = vg_input_constructor,
    .destructor_cb = vg_input_destructor,
    .event_cb = vg_input_event,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .width_def = LV_DPI_DEF * 2,
    .height_def = LV_DPI_DEF,
    .instance_size = sizeof(vg_input_t),
    .name = "vg_input"
};

static void vg_input_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    vg_input_t* input = (vg_input_t*)obj;
    input->kb_obj = NULL;
    input->focus_lost = false;
    input->kb_disable = false;

    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_set_style_anim_duration(obj, 500, LV_PART_CURSOR);
    lv_obj_set_style_border_color(obj, lv_palette_main(LV_PALETTE_GREY), LV_PART_CURSOR);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_LEFT, LV_PART_CURSOR);

    /* input method type default */
    vg_input_set_method_type(obj, UIKIT_INPUT_METHOD_DEFAULT_TYPE);

    LV_TRACE_OBJ_CREATE("finish");
}

static void vg_input_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    vg_input_t* input = (vg_input_t*)obj;

    if (input->im_type == VG_INPUT_METHOD_TYPE_KEYBOARD) {
        if (input->kb_obj == NULL) {
            return;
        }
        lv_obj_delete(input->kb_obj);
        input->kb_obj = NULL;
    } else if (input->im_type == VG_INPUT_METHOD_TYPE_FRAMEWORK) {
        if (g_input_context_ops && g_input_context_ops->destroy) {
            g_input_context_ops->destroy(input->context);
        }
    }

    LV_TRACE_OBJ_CREATE("finish");
}

static void vg_input_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);

    /*Call the ancestor's event handler*/
    lv_result_t res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RESULT_OK) {
        return;
    }

    lv_event_code_t code = lv_event_get_code(e);
    vg_input_t* input = (vg_input_t*)lv_event_get_current_target(e);
    if (input == NULL) {
        return;
    }

    switch (code) {
    case LV_EVENT_FOCUSED:
        if (input->im_type == VG_INPUT_METHOD_TYPE_KEYBOARD) {
            if (input->kb_obj == NULL) {
                return;
            }
            lv_keyboard_set_textarea(input->kb_obj, (lv_obj_t*)input);
            lv_obj_remove_flag(input->kb_obj, LV_OBJ_FLAG_HIDDEN);
        } else if (input->im_type == VG_INPUT_METHOD_TYPE_FRAMEWORK) {
            if (g_input_context_ops == NULL || input->context == NULL) {
                return;
            }
            /* set text change callback */
            if (g_input_context_ops->set_text_changed_callback == NULL) {
                return;
            }
            g_input_context_ops->set_text_changed_callback(input->context, vg_input_text_changed);
            /* require input method */
            if (g_input_context_ops->require == NULL) {
                return;
            }
            input->focus_lost = false;
            if (!input->kb_disable) {
                g_input_context_ops->require(input->context, lv_textarea_get_text((lv_obj_t*)input));
            }
        }
        lv_obj_set_style_border_width((lv_obj_t*)input, 2, LV_PART_CURSOR);
        break;
    case LV_EVENT_DEFOCUSED:
        if (input->im_type == VG_INPUT_METHOD_TYPE_KEYBOARD) {
            if (input->kb_obj == NULL) {
                return;
            }
            lv_keyboard_set_textarea(input->kb_obj, NULL);
            lv_obj_add_flag(input->kb_obj, LV_OBJ_FLAG_HIDDEN);
        } else if (input->im_type == VG_INPUT_METHOD_TYPE_FRAMEWORK) {
            if (g_input_context_ops == NULL || input->context == NULL) {
                return;
            }
            /* lost control */
            if (g_input_context_ops->set_focus_lost_callback == NULL) {
                return;
            }
            g_input_context_ops->set_focus_lost_callback(input->context, vg_input_focus_lost);
            /* lost focus */
            if (!input->focus_lost) {
                return;
            }
        }
        lv_obj_set_style_border_width((lv_obj_t*)input, 0, LV_PART_CURSOR);
        break;
    case LV_EVENT_READY:
        /* input ready */
        break;
    case LV_EVENT_VALUE_CHANGED:
        /* input text changed */
        break;
    default:
        break;
    }
}

lv_obj_t* vg_input_create(lv_obj_t* parent)
{
    LV_LOG_INFO("input create begin");
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    LV_LOG_INFO("input create end");

    return obj;
}

void vg_input_set_text(lv_obj_t* obj, const char* text)
{
    LV_LOG_USER("vg_input_set_text: %s", text);

    lv_textarea_set_text(obj, text);
}

const char* vg_input_get_text(lv_obj_t* obj)
{
    LV_LOG_USER("vg_input_get_text");

    return lv_textarea_get_text(obj);
}

void vg_input_set_placeholder_text(lv_obj_t* obj, const char* text)
{
    LV_LOG_USER("vg_input_set_placeholder_text: %s", text);

    lv_textarea_set_placeholder_text(obj, text);
}

void vg_input_set_method_type(lv_obj_t* obj, vg_input_method_type_t type)
{
    LV_LOG_USER("vg_input_set_keyboard_type: %d", type);

    vg_input_t* input = (vg_input_t*)obj;
    if (input == NULL) {
        return;
    }

    input->im_type = type;
    if (type == VG_INPUT_METHOD_TYPE_KEYBOARD) {
        /* create system input keyboard */
#ifdef CONFIG_UIKIT_KEYBOARD
        input->kb_obj = vg_keyboard_create(lv_layer_top());
#endif
    } else if (type == VG_INPUT_METHOD_TYPE_FRAMEWORK) {
        /* create input method app */
        if (g_input_context_ops == NULL || g_input_context_ops->create == NULL) {
            return;
        }
        input->context = g_input_context_ops->create((void*)obj);
    } else {
        LV_LOG_ERROR("no keyboard set");
    }
}

lv_obj_t* vg_input_get_keyboard(lv_obj_t* obj)
{
    vg_input_t* input = (vg_input_t*)obj;
    if (input == NULL) {
        return NULL;
    }

    return input->kb_obj;
}

void vg_input_set_keyboard_font(lv_obj_t* obj, const char* font_name)
{
    LV_LOG_USER("vg_input_set_keyboard_font name: %s", font_name);

    vg_input_t* input = (vg_input_t*)obj;
    if (input == NULL) {
        return;
    }

    if (input->im_type == VG_INPUT_METHOD_TYPE_KEYBOARD) {
#ifdef CONFIG_UIKIT_KEYBOARD
        if (input->kb_obj == NULL) {
            return;
        }
        vg_keyboard_set_font(input->kb_obj, font_name);
#endif
    }
}

bool vg_input_is_focus_lost(lv_obj_t* obj)
{
    vg_input_t* input = (vg_input_t*)obj;
    if (input == NULL) {
        return 0;
    }

    return input->focus_lost;
}

void vg_set_input_context_ops(vg_input_context_ops_t* ops)
{
    if (g_input_context_ops) {
        LV_LOG_ERROR("input context is set!");
        return;
    }

    g_input_context_ops = ops;
};

void vg_input_text_changed(vg_input_context_t* im, const char* text)
{
    if (g_input_context_ops == NULL || g_input_context_ops->get_user_data == NULL) {
        return;
    }

    LV_LOG_USER("input method text changed");
    lv_obj_t* input = (lv_obj_t*)(g_input_context_ops->get_user_data(im));
    if (input == NULL) {
        return;
    }

    vg_input_set_text(input, text);
}

void vg_input_focus_lost(vg_input_context_t* im)
{
    LV_LOG_USER("input method lost focus");

    if (g_input_context_ops == NULL || g_input_context_ops->get_user_data == NULL) {
        return;
    }

    vg_input_t* input = (vg_input_t*)(g_input_context_ops->get_user_data(im));
    if (input == NULL) {
        return;
    }

    lv_obj_send_event((lv_obj_t*)input, LV_EVENT_DEFOCUSED, NULL);

    input->focus_lost = true;
}

void vg_input_disable_keyboard(lv_obj_t* obj, bool disable)
{
    vg_input_t* input = (vg_input_t*)obj;
    if (input == NULL) {
        return;
    }

    input->kb_disable = disable;
}

#endif