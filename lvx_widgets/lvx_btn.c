/*********************
 *      INCLUDES
 *********************/
#include "lvx_btn.h"

#if (LVX_USE_BTN != 0 && LV_USE_BTN != 0)

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_btn_class
#define ANIME_TIME 150
#define LONGBTN_WIDTH 270
#define LONGBTN_HEIGH 96

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    bool has_init;
    lv_style_t local_style;
} lvx_obj_style_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_btn_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_btn_event(const lv_obj_class_t* class_p, lv_event_t* e);
static lv_obj_t* lvx_btn_set_label(lv_obj_t* obj);
static void style_init(void);

static lvx_obj_style_t stripe_style = { 0 };
static lvx_obj_style_t* styles = &stripe_style;

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_btn_class = {
    .constructor_cb = lvx_btn_constructor,
    .event_cb = lvx_btn_event,
    .width_def = 240,
    .height_def = 90,
    .base_class = &lv_btn_class,
    .instance_size = sizeof(lvx_btn_t),
};

const lv_obj_class_t lvx_btn_label_class = {
    .base_class = &lv_label_class,
};

static lv_style_transition_dsc_t trans_delayed;
static const lv_style_prop_t trans_props[] = { LV_STYLE_BG_OPA,
                                               LV_STYLE_BG_COLOR,
                                               LV_STYLE_TRANSFORM_WIDTH,
                                               LV_STYLE_TRANSFORM_HEIGHT,
                                               LV_STYLE_TRANSLATE_Y,
                                               LV_STYLE_TRANSLATE_X,
                                               LV_STYLE_TRANSFORM_ZOOM,
                                               LV_STYLE_TRANSFORM_ANGLE,
                                               LV_STYLE_COLOR_FILTER_OPA,
                                               LV_STYLE_COLOR_FILTER_DSC,
                                               LV_STYLE_OPA,
                                               0 };

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
    LV_LOG_INFO("begin");
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

lv_obj_t* lvx_btn_get_label(lv_obj_t* obj)
{
    lv_obj_t* label = lv_obj_get_child(obj, 0);
    if (!label) {
        label = lv_obj_class_create_obj(&lvx_btn_label_class, obj);
        lv_obj_class_init_obj(label);
        lv_obj_center(label);
    }
    return label;
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
    lv_obj_set_style_bg_color(obj, WIDGET_BG_COLOR_PRESSED(color),
                              LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, WIDGET_BG_COLOR_DISABLE(color),
                              LV_STATE_DISABLED);
}

void lvx_btn_set_style_text_color(lv_obj_t* obj, lv_color_t color)
{
    lvx_btn_t* btn = (lvx_btn_t*)obj;
    if (btn->label == NULL) {
        btn->label = lvx_btn_set_label(obj);
    }

    lv_obj_set_style_text_color(btn->label, color, LV_STATE_DEFAULT);
}

void lvx_btn_set_style_text_font(lv_obj_t* obj, const lv_font_t* font)
{
    lvx_btn_t* btn = (lvx_btn_t*)obj;
    if (btn->label == NULL) {
        btn->label = lvx_btn_set_label(obj);
    }

    lv_obj_set_style_text_font(btn->label, font, LV_STATE_DEFAULT);
}

void lvx_btn_set_text_fmt(lv_obj_t* obj, const char* fmt, ...)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_btn_t* btn = (lvx_btn_t*)obj;
    if (btn->label == NULL) {
        btn->label = lvx_btn_set_label(obj);
    }

    /*If text is NULL then refresh*/
    if (fmt == NULL) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    char* text = _lv_txt_set_text_vfmt(fmt, args);
    va_end(args);

    lv_label_set_text(btn->label, text);

    lv_mem_free(text);

    return;
}

/*=====================
 * Getter functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_btn_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    style_init();
    lv_obj_add_style(obj, &styles->local_style, LV_STATE_PRESSED);
    lv_obj_add_style(obj, &styles->local_style, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
}

static void lvx_btn_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;

    lv_obj_t* obj = lv_event_get_current_target(e);
    lvx_btn_t* btn = (lvx_btn_t*)obj;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SIZE_CHANGED) {
        if (!btn->label && btn->has_style_transform == false) {
            btn->has_style_transform = true;
            lv_coord_t w = 0, h = 0;
            w = lv_obj_get_width(obj);
            h = lv_obj_get_height(obj);
            lv_obj_set_style_transform_width(obj, -w / 10 / 2,
                                             LV_STATE_PRESSED);
            lv_obj_set_style_transform_height(obj, -h / 10 / 2,
                                              LV_STATE_PRESSED);
            lv_opa_t opa = lv_obj_get_style_opa(obj, 0);
            lv_obj_set_style_opa(obj, opa * 80 / 100, LV_STATE_PRESSED);
        }
    } else if (code == LV_EVENT_DRAW_MAIN_BEGIN) {
        if (btn->has_style_transform == true && btn->label) {
            btn->has_style_transform = false;
            lv_opa_t opa = lv_obj_get_style_opa(obj, 0);
            lv_obj_set_style_opa(obj, opa * 80 / 100, LV_STATE_PRESSED);
            lv_obj_remove_local_style_prop(obj, LV_STYLE_TRANSFORM_WIDTH,
                                           LV_STATE_PRESSED);
            lv_obj_remove_local_style_prop(obj, LV_STYLE_TRANSFORM_HEIGHT,
                                           LV_STATE_PRESSED);
        }
    }
}

static lv_obj_t* lvx_btn_set_label(lv_obj_t* obj)
{
    lv_obj_t* label = lv_label_create(obj);
    lv_obj_set_size(label, LV_PCT(90), LV_SIZE_CONTENT);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);
    return label;
}

static void style_init(void)
{
    if (styles->has_init)
        return;
    styles->has_init = true;

    lv_style_init(&styles->local_style);

    lv_style_transition_dsc_init(&trans_delayed, trans_props,
                                 lv_anim_path_linear, ANIME_TIME, 0, NULL);
    lv_style_set_transition(
        &styles->local_style,
        &trans_delayed); /*Go back to default state with delay*/
}

#endif
