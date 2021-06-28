#ifndef LV_EXTENSIONS_WIDGETS_SWITCH_H_
#define LV_EXTENSIONS_WIDGETS_SWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvx_widgets.h"

#if (LVX_USE_SWITCH != 0 && (LV_USE_SWITCH != 0))

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_switch_t switcher;
}lvx_switch_t;

extern const lv_obj_class_t lvx_switch_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/* lvx_switch api */
lv_obj_t* lvx_switch_create(lv_obj_t* parent);

/**********************
 *      MACROS
 **********************/

#endif /* LVX_USE_SWITCH */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_EXTENSIONS_WIDGETS_SWITCH_H_ */
