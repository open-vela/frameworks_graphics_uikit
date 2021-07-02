/**
 * @file lvx_theme_extended.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_theme_extended.h"

#if LVX_USE_THEME_EXTENDED != 0

#include "../lv_ext.h" /*To see all the widgets*/
#include <lvgl/src/core/lv_obj.h>
#include <lvgl/src/core/lv_theme.h>
#include <lvgl/src/lv_conf_internal.h>
#include <lvgl/src/misc/lv_gc.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_style_t bg;
    lv_style_t no_border;
    lv_style_t pad_zero;
    lv_style_t no_radius;
    lv_style_t radius_circle;

#if LVX_USE_BTN
    lv_style_t btn;
    lv_style_t btn_pressed;
    lv_style_t btn_disable;
#endif

#if LVX_USE_RADIO
    lv_style_t radio_unchecked;
    lv_style_t radio_unchecked_pressed;
    lv_style_t radio_unchecked_disable;
    lv_style_t radio_checked;
    lv_style_t radio_checked_pressed;
    lv_style_t radio_checked_disable;
#endif

#if LVX_USE_SWITCH
    lv_style_t switch_unchecked;
    lv_style_t switch_checked;
    lv_style_t switch_knob;
#endif

#if LVX_USE_PICKER
    lv_style_t picker;
    lv_style_t picker_selected;
#endif

#if LVX_USE_MSGBOX
    lv_style_t msgbox_title;
    lv_style_t msgbox_body;
#endif

} my_theme_styles_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void style_init_reset(lv_style_t* style);
static void theme_apply(lv_theme_t* th, lv_obj_t* obj);

/**********************
 *  STATIC VARIABLES
 **********************/
static my_theme_styles_t* styles;
static lv_theme_t theme;
static bool inited;

/**********************
 *      MACROS
 **********************/
#if LVX_USE_BTN
#define BTN_BG_COLOR_DEFAULT           lv_color_hex(0x0D84FF)
#endif


/**********************
 *   STATIC FUNCTIONS
 **********************/

#if LVX_USE_BTN
static inline void lvx_btn_styles_init(void)
{
    style_init_reset(&styles->btn);
    lv_style_set_radius(&styles->btn, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&styles->btn, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->btn, WIDGET_BG_COLOR_NORMOL(BTN_BG_COLOR_DEFAULT));
    lv_style_set_text_color(&styles->btn, SYSTEM_COLOR_WHITE);
    lv_style_set_bg_img_tiled(&styles->btn, false);

    style_init_reset(&styles->btn_pressed);
    lv_style_set_bg_color(&styles->btn_pressed, WIDGET_BG_COLOR_PRESSED(BTN_BG_COLOR_DEFAULT));
    lv_style_set_bg_img_opa(&styles->btn_pressed, LV_OPA_70);

    style_init_reset(&styles->btn_disable);
    lv_style_set_bg_color(&styles->btn_disable, WIDGET_BG_COLOR_DISABLE(BTN_BG_COLOR_DEFAULT));
    // lv_style_set_bg_img_recolor(&styles->btn_disable, lv_color_hex(0xFF0000));
    // lv_style_set_bg_img_recolor_opa(&styles->btn_disable, LV_OPA_40);
    lv_style_set_bg_img_opa(&styles->btn_disable, LV_OPA_40);
}

static inline void lvx_btn_styles_apply(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &styles->btn, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, &styles->btn_pressed, LV_STATE_DEFAULT | LV_STATE_PRESSED);
    lv_obj_add_style(obj, &styles->btn_disable, LV_STATE_DEFAULT | LV_STATE_DISABLED);
}
#endif

#if LVX_USE_RADIO
static inline void lvx_radio_styles_init(void)
{
    style_init_reset(&styles->radio_unchecked);
    lv_style_set_bg_color(&styles->radio_unchecked, RADIO_BG_COLOR_UNCHECKED);
    lv_style_set_bg_opa(&styles->radio_unchecked, LV_OPA_COVER);
    lv_style_set_border_color(&styles->radio_unchecked, RADIO_BORDER_COLOR);
    lv_style_set_border_opa(&styles->radio_unchecked, RADIO_BORDER_OPA);
    lv_style_set_border_width(&styles->radio_unchecked, RADIO_BORDER_WIDTH);
    lv_style_set_bg_img_opa(&styles->radio_unchecked, LV_OPA_0);

    style_init_reset(&styles->radio_unchecked_pressed);
    lv_style_set_bg_color(&styles->radio_unchecked_pressed, RADIO_BG_COLOR_UNCHECKED);
    lv_style_set_border_opa(&styles->radio_unchecked_pressed, RADIO_BORDER_OPA_PRESSED);
    lv_style_set_bg_img_opa(&styles->radio_unchecked_pressed, LV_OPA_0);

    style_init_reset(&styles->radio_unchecked_disable);
    lv_style_set_bg_color(&styles->radio_unchecked_disable, RADIO_BG_COLOR_UNCHECKED);
    lv_style_set_border_opa(&styles->radio_unchecked_disable, RADIO_BORDER_OPA_DISABLE);
    lv_style_set_bg_img_opa(&styles->radio_unchecked_disable, LV_OPA_0);

    style_init_reset(&styles->radio_checked);
    lv_style_set_border_opa(&styles->radio_checked, RADIO_BORDER_OPA_CHECKED);
    lv_style_set_border_width(&styles->radio_checked, RADIO_BORDER_WIDTH_CHECKED);
    lv_style_set_bg_color(&styles->radio_checked, RADIO_BG_COLOR_CHECKED);
    lv_style_set_bg_img_opa(&styles->radio_checked, LV_OPA_COVER);

    style_init_reset(&styles->radio_checked_pressed);
    lv_style_set_bg_color(&styles->radio_checked_pressed, RADIO_BG_COLOR_CHECKED_PRESSED);
    lv_style_set_bg_img_opa(&styles->radio_checked_pressed, LV_OPA_70);

    style_init_reset(&styles->radio_checked_disable);
    lv_style_set_bg_color(&styles->radio_checked_disable, RADIO_BG_COLOR_CHECKED_DISABLE);
    lv_style_set_bg_img_opa(&styles->radio_checked_disable, LV_OPA_40);
}

static inline void lvx_radio_styles_apply(lv_obj_t* obj)
{
    #if LVX_USE_BTN
    if (lv_obj_has_class(obj, &lvx_btn_class)) {
        lvx_btn_styles_apply(obj);
    }
    #endif
    lv_obj_add_style(obj, &styles->radio_unchecked, 0);
    lv_obj_add_style(obj, &styles->radio_unchecked_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(obj, &styles->radio_unchecked_disable, LV_STATE_DISABLED);

    lv_obj_add_style(obj, &styles->radio_checked, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &styles->radio_checked_pressed, LV_STATE_CHECKED | LV_STATE_PRESSED);
    lv_obj_add_style(obj, &styles->radio_checked_disable, LV_STATE_CHECKED | LV_STATE_DISABLED);
}
#endif


#if LVX_USE_SWITCH
static inline void lvx_switch_styles_init(void)
{
    style_init_reset(&styles->switch_unchecked);
    lv_style_set_bg_color(&styles->switch_unchecked, WIDGET_BG_COLOR_NORMOL(SWITCH_BG_COLOR_DEFAULT));
    lv_style_set_bg_opa(&styles->switch_unchecked, WIDGET_BG_OPA_NORMOL);
    lv_style_set_radius(&styles->switch_unchecked, LV_RADIUS_CIRCLE);

    style_init_reset(&styles->switch_checked);
    lv_style_set_bg_color(&styles->switch_checked, WIDGET_BG_COLOR_NORMOL(SWITCH_BG_COLOR_CHECKED));
    lv_style_set_bg_opa(&styles->switch_checked, WIDGET_BG_OPA_NORMOL);
    lv_style_set_radius(&styles->switch_checked, LV_RADIUS_CIRCLE);

    style_init_reset(&styles->switch_knob);
    lv_style_set_bg_color(&styles->switch_knob, WIDGET_BG_COLOR_NORMOL(SWITCH_KNOB_COLOR));
    lv_style_set_bg_opa(&styles->switch_knob, WIDGET_BG_OPA_NORMOL);

    lv_style_set_pad_all(&styles->switch_knob, - lv_disp_dpx(theme.disp, SWITCH_KNOB_OFFSET));
}

static inline void lvx_switch_styles_apply(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &styles->switch_unchecked, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, &styles->switch_checked, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &styles->radius_circle, LV_PART_KNOB);
    lv_obj_add_style(obj, &styles->switch_knob, LV_PART_KNOB);
}
#endif

#if LVX_USE_PICKER
static inline void lvx_picker_styles_init(void)
{
    style_init_reset(&styles->picker);
    lv_style_set_pad_all(&styles->picker, PICKER_PAD_ALL);
    lv_style_set_pad_gap(&styles->picker, PICKER_PAD_GAP);
    lv_style_set_text_font(&styles->picker, PICKER_TEXT_FONT_DEFAULT);
    // lv_style_set_text_font(&styles->picker, &lv_font_montserrat_22);
    lv_style_set_text_color(&styles->picker, PICKER_TEXT_COLOR_DEFAULT);
    lv_style_set_text_opa(&styles->picker, PICKER_TEXT_OPA_DEFAULT);
    lv_style_set_text_align(&styles->picker, PICKER_TEXT_ALIGN_DEFAULT);

    style_init_reset(&styles->picker_selected);
    lv_style_set_text_font(&styles->picker_selected, PICKER_TEXT_FONT_SELECTED);
    // lv_style_set_text_font(&styles->picker_selected, &lv_font_montserrat_48);
    lv_style_set_text_color(&styles->picker_selected, PICKER_TEXT_COLOR_SELECTED);
    lv_style_set_text_opa(&styles->picker_selected, PICKER_TEXT_OPA_SELECTED);
    lv_style_set_text_align(&styles->picker_selected, PICKER_TEXT_ALIGN_SELECTED);
}

static inline void lvx_picker_styles_apply(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &styles->bg, 0);
    lv_obj_add_style(obj, &styles->picker, LV_PART_MAIN);
    lv_obj_add_style(obj, &styles->picker_selected, LV_PART_SELECTED);
}
#endif

#if LVX_USE_MSGBOX
static inline void lvx_msgbox_styles_init(void)
{
    style_init_reset(&styles->msgbox_title);
    lv_style_set_text_color(&styles->msgbox_title, MSGBOX_TEXT_COLOR_DEFAULT);
    lv_style_set_text_font(&styles->msgbox_title, MSGBOX_TEXT_FONT_DEFAULT);
    lv_style_set_text_align(&styles->msgbox_title, MSGBOX_TEXT_ALIGN_DEFAULT);

    style_init_reset(&styles->msgbox_body);
    lv_style_set_text_color(&styles->msgbox_body, MSGBOX_TEXT_COLOR_ITEMS);
    lv_style_set_text_font(&styles->msgbox_body, MSGBOX_TEXT_FONT_ITEMS);
    lv_style_set_text_align(&styles->msgbox_body, MSGBOX_TEXT_ALIGN_ITEMS);
}

static inline void lvx_msgbox_styles_apply(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &styles->bg, 0);
    lv_obj_add_style(obj, &styles->radius_circle, LV_PART_MAIN);
    lv_obj_add_style(obj, &styles->msgbox_title, LV_PART_MAIN);
    lv_obj_add_style(obj, &styles->msgbox_body, LV_PART_ITEMS);

}
#endif

static void style_init(void)
{
    style_init_reset(&styles->bg);
    lv_style_set_bg_color(&styles->bg, lv_color_black());
    lv_style_set_bg_opa(&styles->bg, LV_OPA_COVER);

    style_init_reset(&styles->no_border);
    lv_style_set_border_width(&styles->no_border, 0);

    style_init_reset(&styles->pad_zero);
    lv_style_set_pad_all(&styles->pad_zero, 0);
    lv_style_set_pad_gap(&styles->pad_zero, 0);

    style_init_reset(&styles->no_radius);
    lv_style_set_radius(&styles->no_radius, 0);

    style_init_reset(&styles->radius_circle);
    lv_style_set_radius(&styles->radius_circle, LV_RADIUS_CIRCLE);

#if LVX_USE_BTN
    lvx_btn_styles_init();
#endif

#if LVX_USE_RADIO
    lvx_radio_styles_init();
#endif

#if LVX_USE_SWITCH
    lvx_switch_styles_init();
#endif

#if LVX_USE_PICKER
    lvx_picker_styles_init();
#endif

#if LVX_USE_MSGBOX
    lvx_msgbox_styles_init();
#endif

}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_theme_t* lvx_theme_extended_init(lv_disp_t* disp)
{

    /*This trick is required only to avoid the garbage collection of
     *styles' data if LVGL is used in a binding (e.g. Micropython)
     *In a general case styles could be in simple `static lv_style_t
     *my_style...` variables*/
    if (!inited) {
        LV_GC_ROOT(_lv_theme_default_styles)
            = lv_mem_alloc(sizeof(my_theme_styles_t));
        styles = (my_theme_styles_t*)LV_GC_ROOT(_lv_theme_default_styles);
    }

    theme.disp = disp;
    theme.font_small = LV_FONT_DEFAULT;
    theme.font_normal = LV_FONT_DEFAULT;
    theme.font_large = LV_FONT_DEFAULT;
    theme.apply_cb = theme_apply;

    style_init();

    inited = true;

    if (disp == NULL || lv_disp_get_theme(disp) == &theme)
        lv_obj_report_style_change(NULL);

    return (lv_theme_t*)&theme;
}

static void theme_apply(lv_theme_t* th, lv_obj_t* obj)
{
    LV_UNUSED(th);

    if (lv_obj_get_parent(obj) == NULL) {
        return;
    }

    if (lv_obj_check_type(obj, &lv_obj_class)) {
        return;
    }

#if LVX_USE_BTN
    else if (lv_obj_check_type(obj, &lvx_btn_class)) {
        lvx_btn_styles_apply(obj);
        return;
    }
#endif
#if LVX_USE_RADIO
    else if (lv_obj_check_type(obj, &lvx_radio_class)) {
        lvx_radio_styles_apply(obj);
        return;
    }
#endif
#if LVX_USE_SWITCH
    else if (lv_obj_check_type(obj, &lvx_switch_class)) {
        lvx_switch_styles_apply(obj);
        return;
    }
#endif
#if LVX_USE_PICKER
    else if (lv_obj_check_type(obj, &lvx_picker_class)) {
        lvx_picker_styles_apply(obj);
        return;
    }
#endif
#if LVX_USE_MSGBOX
    else if (lv_obj_check_type(obj, &lvx_msgbox_class)) {
        lvx_msgbox_styles_apply(obj);
        return;
    }
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void style_init_reset(lv_style_t* style)
{
    if (inited)
        lv_style_reset(style);
    else
        lv_style_init(style);
}

#endif
