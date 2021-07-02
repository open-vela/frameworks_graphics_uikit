/**
 * @file lvx_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_widgets.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Remove all event callbacks added by developers.
 * @param obj pointer to an object
 * @return none
 */
void lvx_obj_remove_all_event_cb(lv_obj_t* obj)
{
    if (obj->spec_attr && obj->spec_attr->event_dsc) {
        lv_mem_free(obj->spec_attr->event_dsc);
        obj->spec_attr->event_dsc = NULL;
        obj->spec_attr->event_dsc_cnt = 0;
    }
}

#if LV_USE_CANVAS != 0
/**
 * Copy canvas content from src to dst canvas.
 * Only pixel in range of canvas is copied.
 * @param dst: the destination canvas, must be a canvas obj.
 * @param src: the source canvas to copy from.
 * @param pos_dst: the position on destination canvas.
 * @param pos_src: the position on source canvas.
 * @param w: width of content to copy.
 * @param h: heigh of content to copy.
 */
lv_area_t lvx_canvas_copy_area(lv_obj_t* dst, lv_obj_t* src,
                               const lv_point_t* pos_dst,
                               const lv_point_t* pos_src, lv_coord_t w,
                               lv_coord_t h)
{
    extern lv_obj_class_t lv_canvas_class;
    LV_ASSERT_OBJ(dst, &lv_canvas_class);
    LV_ASSERT_OBJ(src, &lv_canvas_class);

    lv_area_t result = { 0 };

    lv_canvas_t* canvas_dst = (lv_canvas_t*)dst;
    lv_canvas_t* canvas_src = (lv_canvas_t*)src;
    bool ret;

    if (canvas_dst->dsc.header.cf != canvas_src->dsc.header.cf) {
        LV_LOG_ERROR("cannot copy different color format canvas.\n");
        return result;
    }

    w = LV_MIN(w, canvas_src->dsc.header.w - pos_src->x);
    h = LV_MIN(h, canvas_src->dsc.header.h - pos_src->y);

    w = LV_MIN(w, canvas_dst->dsc.header.w - pos_dst->x);
    h = LV_MIN(h, canvas_dst->dsc.header.h - pos_dst->y);

    if (w < 0 || h < 0) {
        LV_LOG_ERROR("area out of canvas range.\n");
        return result;
    }

    /* Now copy data from src to dst with size of w/h */
    uint32_t px_size = lv_img_cf_get_px_size(canvas_dst->dsc.header.cf) >> 3;
    uint32_t px_src = canvas_src->dsc.header.w * pos_src->y * px_size
        + pos_src->x * px_size;
    uint32_t px_dst = canvas_dst->dsc.header.w * pos_dst->y * px_size
        + pos_dst->x * px_size;

    lv_coord_t i;
    for (i = 0; i < h; i++) {
        lv_memcpy((void*)&canvas_dst->dsc.data[px_dst],
                  (void*)&canvas_src->dsc.data[px_src], w * px_size);
        px_src += canvas_src->dsc.header.w * px_size;
        px_dst += canvas_dst->dsc.header.w * px_size;
    }
    result.x1 = pos_dst->x;
    result.y1 = pos_dst->y;
    result.x2 = result.x1 + w - 1;
    result.y2 = result.y1 + h - 1;
    return result;
}
#endif

/*======================
 * Add/remove functions
 *=====================*/

/*
 * New object specific "add" or "remove" functions come here
 */

/*=====================
 * Setter functions
 *====================*/
void lvx_obj_set_text_styles(lv_obj_t* obj, text_styles_t* styles,
                                    lv_style_selector_t selector)
{
    AMCK(!styles, , , return);

    lv_obj_set_style_text_font(obj, styles->font, selector);
    lv_obj_set_style_text_color(obj, styles->color, selector);
    lv_obj_set_style_text_opa(obj, styles->opa, selector);
    lv_obj_set_style_text_align(obj, styles->align, selector);

    return;
}

/*=====================
 * Getter functions
 *====================*/
void lvx_obj_get_text_styles(lv_obj_t* obj, text_styles_t* styles,
                             uint32_t part)
{
    AMCK(!styles, , , return);

    styles->font = lv_obj_get_style_text_font(obj, part);
    styles->color = lv_obj_get_style_text_color(obj, part);
    styles->opa = lv_obj_get_style_text_opa(obj, part);
    styles->align = lv_obj_get_style_text_align(obj, part);

    return;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
