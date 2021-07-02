#ifndef LV_EXTENSIONS_WIDGETS_MSGBOX_H_
#define LV_EXTENSIONS_WIDGETS_MSGBOX_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvx_widgets.h"

#if (LVX_USE_MSGBOX != 0)

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
enum {
    LVX_MSGBOX_TYPE_ICON,
    LVX_MSGBOX_TYPE_MAIN_TEXT,
    LVX_MSGBOX_TYPE_TITLE,
    LVX_MSGBOX_TYPE_NONE,
};
typedef uint8_t lvx_msgbox_type_t;

enum {
    LVX_MSGBOX_BTN_TYPE_ICON,
    LVX_MSGBOX_BTN_TYPE_TXT,
};
typedef uint8_t lvx_msgbox_btn_type_t;

typedef struct msgbox_btn {
    lvx_msgbox_btn_type_t type;
    const void* src;
    lv_event_cb_t cb;
} msgbox_btn_t;

typedef struct {
    lv_obj_t msgbox;
    lv_obj_t* top;
    lv_obj_t* body;
} lvx_msgbox_t;

extern const lv_obj_class_t lvx_msgbox_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/* lvx_msgbox api */
lv_obj_t* lvx_msgbox_create(lv_obj_t* parent, lvx_msgbox_type_t type,
                            const void* src, const char* txt,
                            msgbox_btn_t btns[], lv_coord_t btn_cnt);
/**********************
 *      MACROS
 **********************/

#endif /* LVX_USE_MSGBOX && LV_USE_MSGBOX */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_EXTENSIONS_WIDGETS_MSGBOX_H_ */
