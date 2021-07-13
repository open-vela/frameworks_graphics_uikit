/*********************
 *      INCLUDES
 *********************/
#include "lvx_btn.h"

#if (LVX_USE_BTN != 0 && LV_USE_BTN != 0)

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_btn_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_btn_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_btn_event(const lv_obj_class_t * class_p, lv_event_t * e);
static inline lv_obj_t* lvx_btn_get_label(lv_obj_t* obj);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_btn_class = {
    .constructor_cb = lvx_btn_constructor,
    .event_cb = lvx_btn_event,
    .width_def = BTN_DEFAULT_WIDTH,
    .height_def = BTN_DEFAULT_HEIGHT,
    .base_class = &lv_btn_class,
    .instance_size = sizeof(lvx_btn_t),
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
lv_obj_t* lvx_btn_create(lv_obj_t* parent)
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

void lvx_btn_set_img_src(lv_obj_t* obj, const void* src)
{
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_PRESSED);
    lv_obj_set_style_bg_img_src(obj, src, LV_STATE_DISABLED);
}

void lvx_btn_set_style_bg_color(lv_obj_t* obj, lv_color_t color)
{
    lv_obj_set_style_bg_color(obj, color, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, WIDGET_BG_COLOR_PRESSED(color), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, WIDGET_BG_COLOR_DISABLE(color), LV_STATE_DISABLED);
}

void lvx_btn_set_style_text_color(lv_obj_t* obj, lv_color_t color)
{
    lv_obj_t* label = lvx_btn_get_label(obj);

    lv_obj_set_style_text_color(label, color, LV_STATE_DEFAULT);
}

void lvx_btn_set_style_text_font(lv_obj_t* obj, const lv_font_t* font)
{
    lv_obj_t* label = lvx_btn_get_label(obj);

    lv_obj_set_style_text_font(label, font, LV_STATE_DEFAULT);
}

void lvx_btn_set_text_fmt(lv_obj_t* obj, const char* fmt, ...)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_obj_t* label = lvx_btn_get_label(obj);

    /*If text is NULL then refresh*/
    if(fmt == NULL) {
        return ;
    }

    va_list args;
    va_start(args, fmt);
    char* text = _lv_txt_set_text_vfmt(fmt, args);
    va_end(args);

    lv_label_set_text(label, text);

    lv_mem_free(text);

    return ;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_btn_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

}

static void lvx_btn_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);
    lv_obj_t* label = lv_obj_get_child(obj, 0);
    /** change state of label to state of btn */
    if (label)
        lv_obj_add_state(label, lv_obj_get_state(obj));
}

static inline lv_obj_t* lvx_btn_get_label(lv_obj_t* obj)
{
    lv_obj_t* label = lv_obj_get_child(obj, 0);
    if (!label) {
        label = lv_label_create(obj);
        lv_obj_center(label);
        lv_obj_set_style_text_opa(label, WIDGET_BG_OPA_PRESSED, LV_STATE_PRESSED);
        lv_obj_set_style_text_opa(label, WIDGET_BG_OPA_DISABLE, LV_STATE_DISABLED);
    }

    return label;
}

#endif
