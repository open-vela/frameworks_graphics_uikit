#ifndef LV_EXTENSIONS_WIDGETS_BTN_H_
#define LV_EXTENSIONS_WIDGETS_BTN_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvx_widgets.h"

#if (LVX_USE_BTN != 0 && LV_USE_BTN != 0)

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_btn_t btn;
}lvx_btn_t;

extern const lv_obj_class_t lvx_btn_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/* lvx_btn api */
lv_obj_t* lvx_btn_create(lv_obj_t* parent);
void lvx_btn_set_img_src(lv_obj_t* obj, void* src);
void lvx_btn_set_text_fmt(lv_obj_t* obj, char* fmt, ...);
void lvx_btn_set_style_bg_color(lv_obj_t* obj, lv_color_t color);
void lvx_btn_set_style_text_color(lv_obj_t* obj, lv_color_t color);

/**********************
 *      MACROS
 **********************/

#endif /* LVX_USE_BTN && LV_USE_BTN */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_EXTENSIONS_WIDGETS_BTN_H_ */
