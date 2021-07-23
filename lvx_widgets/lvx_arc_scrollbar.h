#ifndef LVX_ARC_SCROLLBAR_H_
#define LVX_ARC_SCROLLBAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvx_common.h"

#if (LVX_USE_ARC_SCROLLBAR != 0)

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of scrollbar*/
typedef struct {
    lv_obj_t obj;
    uint16_t rotation;
    uint16_t start_angle;
    uint16_t end_angle;
    uint32_t span;
    int64_t cursor;
    uint32_t pix_cnt;
    uint8_t user_set_option : 1;
    uint8_t user_set_cursor : 1;
} lvx_arc_scrollbar_t;

extern const lv_obj_class_t lvx_arc_scrollbar_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t* lvx_arc_scrollbar_create(lv_obj_t* parent);
void lvx_arc_scrollbar_set_options(lv_obj_t* obj, uint16_t start_angle,
                                   uint16_t end_angle, uint32_t pix_cnt,
                                   uint32_t span);
void lvx_arc_scrollbar_set_cusor(lv_obj_t* obj, int64_t cursor);
/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LVX_ARC_SCROLLBAR_H_ */
