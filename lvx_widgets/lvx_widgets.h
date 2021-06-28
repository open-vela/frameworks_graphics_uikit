#ifndef LVX_WIDGETS_H_
#define LVX_WIDGETS_H_

/* lvx widgets internal header file. */
#include <lvgl/src/lv_conf_internal.h>
#include <lvgl/lvgl.h>

#include "lv_ext_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LVX_WIDGET_CREATE(clazz, parent)                                       \
    LV_LOG_INFO("begin");                                                      \
    lv_obj_t* obj = lv_obj_class_create_obj(clazz, parent);                    \
    lv_obj_class_init_obj(obj);                                                \
    return obj;

void lvx_obj_remove_all_event_cb(lv_obj_t* obj);

#if LV_USE_CANVAS != 0
lv_area_t lvx_canvas_copy_area(lv_obj_t* dst, lv_obj_t* src,
                               const lv_point_t* pos_dst,
                               const lv_point_t* pos_src, lv_coord_t w,
                               lv_coord_t h);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // LVX_WIDGETS_H_
