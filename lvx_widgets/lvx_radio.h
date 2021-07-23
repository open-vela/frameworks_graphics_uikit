#ifndef LV_EXTENSIONS_WIDGETS_RADIO_H_
#define LV_EXTENSIONS_WIDGETS_RADIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvx_common.h"
#include "lvx_btn.h"

#if (LVX_USE_RADIO != 0)

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/


typedef struct {
    lvx_btn_t btn;
}lvx_radio_t;

extern const lv_obj_class_t lvx_radio_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/* lvx_radio api */
lv_obj_t* lvx_radio_create(lv_obj_t* parent);
void lvx_radio_set_img_src(lv_obj_t* obj, const void* src);
void lvx_radio_set_style_bg_color(lv_obj_t* obj, lv_color_t color);

/**********************
 *      MACROS
 **********************/

#endif /* LVX_USE_RADIO */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_EXTENSIONS_WIDGETS_RADIO_H_ */
