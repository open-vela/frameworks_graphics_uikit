/**
 * @file uikit_input.h
 *
 */

#ifndef UIKIT_INPUT_H
#define UIKIT_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>
#include <nuttx/config.h>

#include "uikit/uikit_conf.h"

#if UIKIT_INPUT_METHOD

/*********************
 *      DEFINES
 *********************/

/********************
 *     ENUMS
 ********************/
typedef enum {
    VG_INPUT_METHOD_TYPE_KEYBOARD,
    VG_INPUT_METHOD_TYPE_FRAMEWORK,
    VG_INPUT_METHOD_TYPE_NONE
} vg_input_method_type_t;

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _vg_input_context_t vg_input_context_t;
typedef void (*vg_input_text_changed_callback)(
    vg_input_context_t* im,
    const char* text);

typedef void (*vg_input_focus_lost_callback)(
    vg_input_context_t* im);

typedef vg_input_context_t* (*vg_input_context_create_t)(
    void* user_data);

typedef void (*vg_input_context_destroy_t)(
    vg_input_context_t* im);

typedef void (*vg_input_context_require_t)(
    vg_input_context_t* im,
    const char* initial_text);

typedef void* (*vg_input_context_get_user_data_t)(
    vg_input_context_t* im);

typedef void (*vg_input_context_set_text_changed_t)(
    vg_input_context_t* im,
    vg_input_text_changed_callback callback);

typedef void (*vg_input_context_set_focus_lost_t)(
    vg_input_context_t* im,
    vg_input_focus_lost_callback callback);

typedef struct vg_input_context_ops {
    vg_input_context_create_t create;
    vg_input_context_destroy_t destroy;
    vg_input_context_require_t require;
    vg_input_context_get_user_data_t get_user_data;
    vg_input_context_set_text_changed_t set_text_changed_callback;
    vg_input_context_set_focus_lost_t set_focus_lost_callback;
} vg_input_context_ops_t;

typedef struct {
    lv_textarea_t obj;
    vg_input_method_type_t im_type;
    vg_input_context_t* context;
    lv_obj_t* kb_obj;
    bool focus_lost;
    bool kb_disable;
    void* user_data;
} vg_input_t;

/**********************
 * GLOBAL FUNCTIONS
 **********************/
void vg_set_input_context_ops(vg_input_context_ops_t* ops);

/**
 * Create a input object
 * @param parent pointer to an object, it will be the parent of the new input
 * @return pointer to the created input
 */
lv_obj_t* vg_input_create(lv_obj_t* parent);

/**
 * Set the text of the input
 * @param obj pointer to a input object
 * @param text the text to be displayed
 * @return void
 */
void vg_input_set_text(lv_obj_t* obj, const char* text);

/**
 * Get the text of the input
 * @param obj pointer to a input object
 * @return the text of the input
 */
const char* vg_input_get_text(lv_obj_t* obj);

/**
 * Set the placeholder text of the input
 * @param obj pointer to a input object
 * @param text the placeholder text to be displayed
 * @return void
 */
void vg_input_set_placeholder_text(lv_obj_t* obj, const char* text);

/**
 * Set the property of input keyboard
 * @param obj pointer to a input object
 * @param type the type of keyboard
 */
void vg_input_set_method_type(lv_obj_t* obj, vg_input_method_type_t type);

/**
 * Get the keyboard object
 * @param obj pointer to a input object
 * @return pointer to the keyboard object
 */
lv_obj_t* vg_input_get_keyboard(lv_obj_t* obj);

/**
 * Set the font of input keyboard
 * @param obj pointer to a input object
 * @param font font name of the keyboard
 */
void vg_input_set_keyboard_font(lv_obj_t* obj, const char* font_name);

/**
 * Check if the input is focus lost
 * @param obj pointer to a input object
 * @return return event id
 */
bool vg_input_is_focus_lost(lv_obj_t* obj);

/**
 * Disable the keyboard
 * @param obj pointer to a input object
 * @param disable true to disable the keyboard, false to enable the keyboard
 * @return void
 */
void vg_input_disable_keyboard(lv_obj_t* obj, bool disable);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UIKIT_INPUT_H */