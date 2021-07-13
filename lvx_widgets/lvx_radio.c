/*********************
 *      INCLUDES
 *********************/
#include "lvx_radio.h"

#if (LVX_USE_RADIO != 0)

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_radio_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_radio_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_radio_class = {
    .constructor_cb = lvx_radio_constructor,
    .width_def = RADIO_DEFAULT_WIDTH,
    .height_def = RADIO_DEFAULT_HEIGHT,
    .instance_size = sizeof(lvx_radio_t),
    .base_class = &lvx_btn_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Create a btn object, which
 * @param parent pointer to an object, it will be the parent of the new arc
 * @return pointer to the created arc
 */
lv_obj_t* lvx_radio_create(lv_obj_t* parent)
{
    LVX_WIDGET_CREATE(MY_CLASS, parent);
}

/*======================
 * Add/remove functions
 *=====================*/

/*
 * New object specific "add" or "remove" functions come here
 */

/*=====================
 * Setter functions
 *====================*/

void lvx_radio_set_img_src(lv_obj_t* obj, const void* src)
{
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_CHECKED);
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_CHECKED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_CHECKED | LV_STATE_DISABLED);
}

void lvx_radio_set_style_bg_color(lv_obj_t* obj, lv_color_t color)
{
    lvx_btn_set_style_bg_color(obj, color);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_radio_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
}

#endif
