#ifndef LVX_COMMON_H_
#define LVX_COMMON_H_

/* lvx widgets internal header file. */
#include <lvgl/src/lv_conf_internal.h>
#include <lvgl/lvgl.h>

#include "lv_ext_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef struct text_styles {
    const lv_font_t* font;
    lv_color_t color;
    lv_opa_t opa;
    lv_text_align_t align;
} text_styles_t;

#ifndef AMCK
#define AMCK(judge, print, ret, next)                                          \
    if (judge) {                                                               \
        print;                                                                 \
        ret;                                                                   \
        next;                                                                  \
    }
#endif
/**********************
 *      MACROS
 **********************/
#define LVX_WIDGET_CREATE(clazz, parent)                                       \
    LV_LOG_INFO("begin");                                                      \
    lv_obj_t* obj = lv_obj_class_create_obj(clazz, parent);                    \
    lv_obj_class_init_obj(obj);                                                \
    return obj;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lvx_obj_remove_all_event_cb(lv_obj_t* obj);
void lvx_obj_get_text_styles(lv_obj_t* obj, text_styles_t* styles,
                             uint32_t part);
void lvx_obj_set_text_styles(lv_obj_t* obj, text_styles_t* styles,
                                    lv_style_selector_t selector);
#if LV_USE_CANVAS != 0
lv_area_t lvx_canvas_copy_area(lv_obj_t* dst, lv_obj_t* src,
                               const lv_point_t* pos_dst,
                               const lv_point_t* pos_src, lv_coord_t w,
                               lv_coord_t h);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // LVX_COMMON_H_
