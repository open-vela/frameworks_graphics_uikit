/*********************
 *      INCLUDES
 *********************/
#include "lvx_msgbox.h"

#include "lvx_btn.h"

#if (LVX_USE_MSGBOX != 0)

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_msgbox_class

#define TOP_BTN_SIZE_W 51
#define TOP_BTN_SIZE_H 51

#define BOTTOM_BTN1_SIZE_W 196
#define BOTTOM_BTN1_SIZE_H 96
#define BTN_OFFSET_Y_1 -7

#define BOTTOM_BTN2_SIZE_W 110
#define BOTTOM_BTN2_SIZE_H 110
#define BTN_OFFSET_X_2 97
#define BTN_OFFSET_Y_2 -36

#define ICON_OFFSET_Y 31
#define MAIN_TEXT_OFFSET_Y 40
#define TITLE_OFFSET_Y 30

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_msgbox_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void refr_obj_text_styles(lv_obj_t* obj);
static lv_obj_t* lvx_msgbox_btn_create(lv_obj_t* parent, msgbox_btn_t* btn);
/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_msgbox_class = {
    .instance_size = sizeof(lvx_msgbox_t),
    .event_cb = lvx_msgbox_event,
    .base_class = &lv_obj_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Create a message box object
 * @param parent pointer to an object
 * @return pointer to the created object
 */
lv_obj_t* lvx_msgbox_create(lv_obj_t* parent, lvx_msgbox_type_t type,
                            const void* src, const char* txt,
                            msgbox_btn_t btns[], lv_coord_t btn_cnt)
{
    lv_obj_t* obj = lv_obj_class_create_obj(&lvx_msgbox_class, parent);
    lv_obj_class_init_obj(obj);
    if (obj == NULL)
        return NULL;
    lv_obj_set_size(obj, LV_DPI_DEF * 2, LV_SIZE_CONTENT);
    // lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);
    // lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
    //                       LV_FLEX_ALIGN_START);
    lvx_msgbox_t* mbox = (lvx_msgbox_t* )obj;
    lv_obj_t* top = NULL;
    lv_coord_t top_offset_y = ICON_OFFSET_Y;
    switch (type) {
    case LVX_MSGBOX_TYPE_ICON: {
        // top = lv_img_create(obj);
        // lv_img_set_src(top, src);
        top = lvx_btn_create(obj);
        if (src) {
            lvx_btn_set_img_src(top, (void* )src);
        }
        lvx_btn_set_style_bg_color(top, SYSTEM_COLOR_BTN_GREY);
        lv_obj_set_size(top, TOP_BTN_SIZE_W, TOP_BTN_SIZE_H);
        top_offset_y = ICON_OFFSET_Y;
        break;
    }
    case LVX_MSGBOX_TYPE_MAIN_TEXT: {
        top = lv_label_create(obj);
        lv_label_set_text(top, src);
        lv_label_set_long_mode(top, LV_LABEL_LONG_SCROLL_CIRCULAR);
        top_offset_y = MAIN_TEXT_OFFSET_Y;
        break;
    }
    case LVX_MSGBOX_TYPE_TITLE: {
        top = lv_label_create(obj);
        lv_label_set_text(top, src);
        lv_label_set_long_mode(top, LV_LABEL_LONG_SCROLL_CIRCULAR);
        top_offset_y = TITLE_OFFSET_Y;
        break;
    }
    default:
        break;
    }
    if (top) {
        lv_obj_align(top, LV_ALIGN_TOP_MID, 0, top_offset_y);
    }

    /** body part */
    lv_obj_t* label = lv_label_create(obj);
    lv_label_set_text(label, txt);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(label, 398, 245);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 103);

    mbox->top = top;
    mbox->body = label;

    /** btns */
    lv_obj_t* btn = NULL;
    if (btn_cnt == 1) {
        /** text button */
        btn = lvx_msgbox_btn_create(obj, btns);
        lvx_btn_set_style_bg_color(btn, SYSTEM_COLOR_BTN_GREY);
        lvx_btn_set_style_text_color(btn, SYSTEM_COLOR_WHITE);
        lvx_btn_set_style_text_font(btn, &lv_font_montserrat_48);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, BTN_OFFSET_Y_1);
        lv_obj_set_size(btn, BOTTOM_BTN1_SIZE_W, BOTTOM_BTN1_SIZE_H);
    } else if (btn_cnt == 2) {
        /** image button */
        lv_color_t color = SYSTEM_COLOR_BLUE;
        lv_align_t align = LV_ALIGN_BOTTOM_LEFT;
        lv_coord_t offset_x = BTN_OFFSET_X_2;
        lv_coord_t offset_y = BTN_OFFSET_Y_2;
        uint32_t index = 0;
        while (index < btn_cnt) {
            if (index == 0) {
                color = SYSTEM_COLOR_BTN_GREY;
                align = LV_ALIGN_BOTTOM_LEFT;
                offset_x = BTN_OFFSET_X_2;
                offset_y = BTN_OFFSET_Y_2;
            } else if (index == 1) {
                color = SYSTEM_COLOR_BLUE;
                align = LV_ALIGN_BOTTOM_RIGHT;
                offset_x = -BTN_OFFSET_X_2;
                offset_y = BTN_OFFSET_Y_2;
            }

            btn = lvx_msgbox_btn_create(obj, btns + index);
            lvx_btn_set_style_bg_color(btn, color);
            lv_obj_align(btn, align, offset_x, offset_y);
            lv_obj_set_size(btn, BOTTOM_BTN2_SIZE_W, BOTTOM_BTN2_SIZE_H);
            index++;
        }
    }
    return obj;
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

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_msgbox_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_current_target(e);

    if (code == LV_EVENT_STYLE_CHANGED) {
        refr_obj_text_styles(obj);
    }
}

static void refr_obj_text_styles(lv_obj_t* obj)
{
    lvx_msgbox_t* mbox = (lvx_msgbox_t*)obj;
    text_styles_t main, item;
    lvx_obj_get_text_styles(obj, &main, LV_PART_MAIN);
    lvx_obj_get_text_styles(obj, &item, LV_PART_ITEMS);

    if (mbox->top) {
        lvx_obj_set_text_styles(mbox->top, &main, 0);
    }
    if (mbox->body) {
        lvx_obj_set_text_styles(mbox->body, &item, 0);
    }
}

static lv_obj_t* lvx_msgbox_btn_create(lv_obj_t* parent, msgbox_btn_t* btn)
{
    AMCK(!btn, , , return NULL);
    lv_obj_t* obj = lvx_btn_create(parent);
    switch (btn->type)
    {
    case LVX_MSGBOX_BTN_TYPE_ICON:
        lvx_btn_set_img_src(obj, btn->src);
        break;
    case LVX_MSGBOX_BTN_TYPE_TXT:
        lvx_btn_set_text_fmt(obj, btn->src);
        break;
    default:
        break;
    }
    if (btn->cb) {
        lv_obj_add_event_cb(obj, btn->cb, LV_EVENT_CLICKED, NULL);
    }
    return obj;
}

#endif
