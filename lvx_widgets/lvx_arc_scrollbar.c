/*********************
 *      INCLUDES
 *********************/
#include "lvx_arc_scrollbar.h"

#if (LVX_USE_ARC_SCROLLBAR != 0)

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_arc_scrollbar_class
#define START_ANGLE 338
#define END_ANGLE 382
#define MAX_VALUE 100
#define MIN_SPAN 5

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_arc_scrollbar_constructor(const lv_obj_class_t* class_p,
                                          lv_obj_t* obj);
static void lvx_arc_scrollbar_destructor(const lv_obj_class_t* class_p,
                                         lv_obj_t* obj);
static void lvx_arc_scrollbar_event(const lv_obj_class_t* class_p,
                                    lv_event_t* e);
static void set_default_scrollbar_style(lv_obj_t* scrollbar);
static void draw_scrollbar(lv_event_t* e);
static void get_center(lv_obj_t* obj, lv_point_t* center, lv_coord_t* arc_r);
static void parent_event_handle(lv_event_t* e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_arc_scrollbar_class = {
    .constructor_cb = lvx_arc_scrollbar_constructor,
    .destructor_cb = lvx_arc_scrollbar_destructor,
    .event_cb = lvx_arc_scrollbar_event,
    .width_def = LV_DPI_DEF * 2,
    .height_def = LV_DPI_DEF / 10,
    .instance_size = sizeof(lvx_arc_scrollbar_t),
    .base_class = &lv_obj_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a scrollbar object on the parent
 * @param parent pointer to an object, it will be the parent of the new
 * scrollbar will be copied from it
 * @return pointer to the created scrollbar
 */
lv_obj_t* lvx_arc_scrollbar_create(lv_obj_t* parent)
{
    LVX_WIDGET_CREATE(MY_CLASS, parent);
}

/**
 * Set scrollbar options
 * @param obj pointer to scrollbar
 * @param start_angle The starting angle of the background of the curved scroll
 * bar, 0~360
 * @param end_angle The ending angle of the background of the curved scroll bar,
 * 0~360
 * @param pix_cnt Total pixels of height/width of all content,
 * lv_obj_get_scroll_top() + parent height + lv_obj_get_scroll_bottom()
 * @param span parent height - pad_top
 * @return pointer to the created scrollbar
 */
void lvx_arc_scrollbar_set_options(lv_obj_t* obj, uint16_t start_angle,
                                   uint16_t end_angle, uint32_t pix_cnt,
                                   uint32_t span)
{
    lvx_arc_scrollbar_t* sb = (lvx_arc_scrollbar_t*)obj;

    while (start_angle >= 360) {
        start_angle -= 360;
    }
    while (end_angle >= 360) {
        end_angle -= 360;
    }
    if (end_angle < 360) {
        end_angle += 360;
    }

    if (sb->start_angle == start_angle && sb->end_angle == end_angle
        && sb->pix_cnt == pix_cnt && sb->span == span)
        return;

    sb->start_angle = start_angle;
    sb->end_angle = end_angle;
    sb->span = span;
    sb->pix_cnt = pix_cnt;
    sb->user_set_option = 1;
    lv_obj_invalidate(obj);
}

void lvx_arc_scrollbar_set_cusor(lv_obj_t* obj, int64_t cursor)
{
    lvx_arc_scrollbar_t* sb = (lvx_arc_scrollbar_t*)obj;
    if (sb->cursor == cursor)
        return;
    sb->cursor = cursor;
    sb->user_set_cursor = 1;
    lv_obj_invalidate(obj);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_arc_scrollbar_constructor(const lv_obj_class_t* class_p,
                                          lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lvx_arc_scrollbar_t* sb = (lvx_arc_scrollbar_t*)obj;
    sb->rotation = 0;
    sb->start_angle = START_ANGLE;
    sb->end_angle = END_ANGLE;
    sb->span = MIN_SPAN;
    sb->cursor = 0;
    sb->pix_cnt = MAX_VALUE;
    sb->user_set_option = 0;
    sb->user_set_cursor = 0;

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SNAPABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_FLOATING);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_IGNORE_LAYOUT);

    lv_obj_t* parent = lv_obj_get_parent(obj);
    if (parent != NULL) {
        lv_obj_update_layout(obj);
        lv_obj_set_size(obj, lv_obj_get_width(parent),
                        lv_obj_get_height(parent));

        lv_coord_t top = lv_obj_get_scroll_top(parent);
        lv_coord_t botoom = lv_obj_get_scroll_bottom(parent);
        lv_coord_t obj_h = lv_obj_get_height(parent);

        lvx_arc_scrollbar_set_options(obj, START_ANGLE, END_ANGLE,
                                      top + botoom + obj_h, obj_h);
        sb->user_set_option = 0;
        lv_obj_center(obj);
        lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_event_cb(parent, parent_event_handle, LV_EVENT_ALL, obj);
    }

    LV_TRACE_OBJ_CREATE("finished");
}

static void lvx_arc_scrollbar_destructor(const lv_obj_class_t* class_p,
                                         lv_obj_t* obj)
{
    LV_UNUSED(class_p);
}

static void lvx_arc_scrollbar_event(const lv_obj_class_t* class_p,
                                    lv_event_t* e)
{
    LV_UNUSED(class_p);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_DRAW_MAIN) {
        draw_scrollbar(e);
    }
}

static void draw_scrollbar(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    lvx_arc_scrollbar_t* sb = (lvx_arc_scrollbar_t*)obj;

    /* Don't draw */
    if (sb->pix_cnt <= 0 || sb->pix_cnt == sb->span) {
        return;
    }

    const lv_area_t* clip_area = lv_event_get_param(e);

    lv_point_t center;
    lv_coord_t arc_r;
    get_center(obj, &center, &arc_r);

    lv_obj_draw_part_dsc_t obj_draw_dsc;
    lv_obj_draw_dsc_init(&obj_draw_dsc, clip_area);

    /*Draw the background arc*/
    lv_draw_arc_dsc_t arc_dsc;
    if (arc_r > 0) {
        lv_draw_arc_dsc_init(&arc_dsc);
        lv_obj_init_draw_arc_dsc(obj, LV_PART_SCROLLBAR, &arc_dsc);

        obj_draw_dsc.part = LV_PART_SCROLLBAR;
        obj_draw_dsc.p1 = &center;
        obj_draw_dsc.radius = arc_r;
        obj_draw_dsc.arc_dsc = &arc_dsc;
        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &obj_draw_dsc);

        lv_draw_arc(center.x, center.y, arc_r, sb->start_angle + sb->rotation,
                    sb->end_angle + sb->rotation, clip_area, &arc_dsc);

        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &obj_draw_dsc);
    }

    /*make the indicator arc smaller or larger according to its greatest padding
     * value*/
    lv_coord_t left_indic = lv_obj_get_style_pad_left(obj, LV_PART_INDICATOR);
    lv_coord_t right_indic = lv_obj_get_style_pad_right(obj, LV_PART_INDICATOR);
    lv_coord_t top_indic = lv_obj_get_style_pad_top(obj, LV_PART_INDICATOR);
    lv_coord_t bottom_indic
        = lv_obj_get_style_pad_bottom(obj, LV_PART_INDICATOR);
    lv_coord_t indic_r
        = arc_r - LV_MAX4(left_indic, right_indic, top_indic, bottom_indic);

    if (indic_r > 0) {
        lv_draw_arc_dsc_init(&arc_dsc);
        lv_obj_init_draw_arc_dsc(obj, LV_PART_INDICATOR, &arc_dsc);

        obj_draw_dsc.part = LV_PART_INDICATOR;
        obj_draw_dsc.p1 = &center;
        obj_draw_dsc.radius = indic_r;
        obj_draw_dsc.arc_dsc = &arc_dsc;
        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &obj_draw_dsc);

        uint16_t span_angle
            = sb->span * (sb->end_angle - sb->start_angle) / sb->pix_cnt;
        span_angle = LV_MAX(span_angle, MIN_SPAN);
        int64_t theta = sb->cursor
            * (sb->end_angle - sb->start_angle - span_angle)
            / (sb->pix_cnt - sb->span);
        uint16_t indic_start_angle = sb->start_angle + theta;
        uint16_t indic_end_angle = indic_start_angle + span_angle;

        if (indic_start_angle < sb->start_angle) {
            indic_start_angle = sb->start_angle;
            if (indic_end_angle - indic_start_angle < MIN_SPAN / 2) {
                indic_end_angle = indic_start_angle + MIN_SPAN / 2;
            }
        }

        if (indic_end_angle > sb->end_angle) {
            indic_end_angle = sb->end_angle;
            if (indic_end_angle - indic_start_angle < MIN_SPAN / 2) {
                indic_start_angle = indic_end_angle - MIN_SPAN / 2;
            }
        }

        lv_draw_arc(center.x, center.y, indic_r,
                    indic_start_angle + sb->rotation,
                    indic_end_angle + sb->rotation + 1, clip_area, &arc_dsc);

        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &obj_draw_dsc);
    }
}

static void get_center(lv_obj_t* obj, lv_point_t* center, lv_coord_t* arc_r)
{
    lv_coord_t left_bg = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    lv_coord_t right_bg = lv_obj_get_style_pad_right(obj, LV_PART_MAIN);
    lv_coord_t top_bg = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    lv_coord_t bottom_bg = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);

    lv_coord_t r = (LV_MIN(lv_obj_get_width(obj) - left_bg - right_bg,
                           lv_obj_get_height(obj) - top_bg - bottom_bg))
        / 2;

    *arc_r = r;
    center->x = obj->coords.x1 + r + left_bg;
    center->y = obj->coords.y1 + r + top_bg;

    lv_coord_t indic_width = lv_obj_get_style_arc_width(obj, LV_PART_INDICATOR);
    r -= indic_width;
}

static void parent_event_handle(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_current_target(e);
    lv_obj_t* sb = lv_event_get_user_data(e);

    if (code == LV_EVENT_DRAW_POST) {
        lvx_arc_scrollbar_t* sb1 = (lvx_arc_scrollbar_t*)sb;
        if (sb && sb1->user_set_cursor == 0 && lv_obj_has_class(sb, MY_CLASS)) {
            lv_coord_t scroll_y = lv_obj_get_scroll_y(obj);
            lvx_arc_scrollbar_set_cusor(sb, scroll_y);
            sb1->user_set_cursor = 0;
        }
    } else if (code == LV_EVENT_CHILD_CHANGED) {
        lv_obj_t* p = lv_event_get_param(e);
        if (sb && p && !lv_obj_has_class(p, MY_CLASS)) {
            lv_obj_move_foreground(sb);
        }
    } else if (code == LV_EVENT_DRAW_POST_END) {
        lvx_arc_scrollbar_t* sb1 = (lvx_arc_scrollbar_t*)sb;
        if (sb && obj && sb1->user_set_option == 0) {
            lv_coord_t top = lv_obj_get_scroll_top(obj);
            lv_coord_t botoom = lv_obj_get_scroll_bottom(obj);
            lv_coord_t obj_h = lv_obj_get_height(obj);
            lvx_arc_scrollbar_set_options(sb, START_ANGLE, END_ANGLE,
                                          top + botoom + obj_h, obj_h);
            sb1->user_set_option = 0;
        }
    } else if (code == LV_EVENT_SIZE_CHANGED) {
        if (sb && obj) {
            lv_obj_set_size(sb, lv_obj_get_width(obj), lv_obj_get_height(obj));
        }
    }
}

#endif
