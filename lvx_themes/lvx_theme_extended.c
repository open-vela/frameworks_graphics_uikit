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

/**********************
 *   STATIC FUNCTIONS
 **********************/

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
