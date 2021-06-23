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
static void lvx_radio_event(const lv_obj_class_t * class_p, lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_radio_class = {
    .constructor_cb = lvx_radio_constructor,
    .event_cb = lvx_radio_event,
    .base_class = &lvx_btn_class,
    .instance_size = sizeof(lvx_radio_t)
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

void lvx_radio_set_img_src(lv_obj_t* obj, void* src)
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
    lvx_radio_t* radio = (lvx_radio_t* )obj;

    radio->checked = false;
}

static void lvx_radio_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    lvx_radio_t* radio = (lvx_radio_t* )obj;

    if (code == LV_EVENT_CLICKED) {
        radio->checked = ~radio->checked;
        if (radio->checked) {
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(obj, LV_STATE_CHECKED);
        }
    }
}

#endif
