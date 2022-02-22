/**
 * @file lvx_chart.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_chart.h"
#include <stdlib.h>
#ifdef CONFIG_LV_GPU_DRAW_POLYGON
#include "lv_porting/lv_gpu_interface.h"
#endif
#if LVX_USE_CHART != 0
/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_chart_class

#define LVX_CHART_HDIV_DEF 3
#define LVX_CHART_VDIV_DEF 5
#define LVX_CHART_POINT_CNT_DEF 10

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_chart_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_chart_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_chart_event(const lv_obj_class_t* class_p, lv_event_t* e);

static void draw_div_lines(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx);
static void draw_series_line(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx);
static void draw_series_bar(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx);
static void draw_series_scatter(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx);
static void draw_cursors(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx);
static void draw_axes(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx);
static uint32_t get_index_from_x(lv_obj_t* obj, lv_coord_t x);
static void invalidate_point(lv_obj_t* obj, uint16_t i);
static void new_points_alloc(lv_obj_t* obj, lvx_chart_series_t* ser,
                             uint32_t cnt, lv_coord_t** a);
static lvx_chart_tick_dsc_t* get_tick_gsc(lv_obj_t* obj, lvx_chart_axis_t axis);

#if LVX_CHART_EXTENTIONS == 1
static void draw_series_point_bar(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx);
static int point_cmp(const void* var1, const void* var2);
static void draw_line_chart_event_cb(lv_event_t* e);
static void draw_avg_line(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx,
                          bool const_flag);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_chart_class
    = { .constructor_cb = lvx_chart_constructor,
        .destructor_cb = lvx_chart_destructor,
        .event_cb = lvx_chart_event,
        .width_def = LV_PCT(100),
        .height_def = LV_DPI_DEF * 2,
        .instance_size = sizeof(lvx_chart_t),
        .base_class = &lv_obj_class };

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t* lvx_chart_create(lv_obj_t* parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lvx_chart_set_type(lv_obj_t* obj, lvx_chart_type_t type)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->type == type)
        return;

    if (chart->type == LVX_CHART_TYPE_SCATTER
        || chart->type == LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
        || chart->type == LVX_CHART_TYPE_POINT_BAR
#endif
    ) {
        lvx_chart_series_t* ser;
        _LV_LL_READ_BACK(&chart->series_ll, ser)
        {
            lv_mem_free(ser->x_points);
            ser->x_points = NULL;
        }
    }

    if (type == LVX_CHART_TYPE_SCATTER
#if LVX_CHART_EXTENTIONS == 1
        || type == LVX_CHART_TYPE_POINT_BAR
#endif
    ) {
        lvx_chart_series_t* ser;
        _LV_LL_READ_BACK(&chart->series_ll, ser)
        {
            ser->x_points = lv_mem_alloc(sizeof(lv_point_t) * chart->point_cnt);
            LV_ASSERT_MALLOC(ser->x_points);
            if (ser->x_points == NULL)
                return;
        }
    }

    chart->type = type;

    lvx_chart_refresh(obj);
}

void lvx_chart_set_point_count(lv_obj_t* obj, uint16_t cnt)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->point_cnt == cnt)
        return;

    lvx_chart_series_t* ser;

    if (cnt < 1)
        cnt = 1;

    _LV_LL_READ_BACK(&chart->series_ll, ser)
    {
        if (chart->type == LVX_CHART_TYPE_SCATTER
            || chart->type == LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
            || chart->type == LVX_CHART_TYPE_POINT_BAR
#endif
        ) {
            if (!ser->x_ext_buf_assigned)
                new_points_alloc(obj, ser, cnt, &ser->x_points);
        }
        if (!ser->y_ext_buf_assigned)
            new_points_alloc(obj, ser, cnt, &ser->y_points);
        ser->start_point = 0;
    }

    chart->point_cnt = cnt;

    lvx_chart_refresh(obj);
}

void lvx_chart_set_range(lv_obj_t* obj, lvx_chart_axis_t axis, lv_coord_t min,
                         lv_coord_t max)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    max = max == min ? max + 1 : max;

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    switch (axis) {
    case LVX_CHART_AXIS_PRIMARY_Y:
        chart->ymin[0] = min;
        chart->ymax[0] = max;
        break;
    case LVX_CHART_AXIS_SECONDARY_Y:
        chart->ymin[1] = min;
        chart->ymax[1] = max;
        break;
    case LVX_CHART_AXIS_PRIMARY_X:
        chart->xmin[0] = min;
        chart->xmax[0] = max;
        break;
    case LVX_CHART_AXIS_SECONDARY_X:
        chart->xmin[1] = min;
        chart->xmax[1] = max;
        break;
    default:
        LV_LOG_WARN("Invalid axis: %d", axis);
        return;
    }

    lvx_chart_refresh(obj);
}

lv_coord_t lvx_chart_get_x_max(lv_obj_t* obj, lvx_chart_axis_t axis)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lv_coord_t value = 0;

    switch (axis) {
    case LVX_CHART_AXIS_PRIMARY_Y:
        value = chart->xmax[0];
        break;
    case LVX_CHART_AXIS_SECONDARY_Y:
        value = chart->xmax[1];
        break;
    default:
        LV_LOG_WARN("Invalid axis: %d", axis);
        return value;
    }

    return value;
}

lv_coord_t lvx_chart_get_x_min(lv_obj_t* obj, lvx_chart_axis_t axis)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lv_coord_t value = 0;

    switch (axis) {
    case LVX_CHART_AXIS_PRIMARY_Y:
        value = chart->xmin[0];
        break;
    case LVX_CHART_AXIS_SECONDARY_Y:
        value = chart->xmin[1];
        break;
    default:
        LV_LOG_WARN("Invalid axis: %d", axis);
        return value;
    }

    return value;
}

lv_coord_t lvx_chart_get_y_max(lv_obj_t* obj, lvx_chart_axis_t axis)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lv_coord_t value = 0;

    switch (axis) {
    case LVX_CHART_AXIS_PRIMARY_Y:
        value = chart->ymax[0];
        break;
    case LVX_CHART_AXIS_SECONDARY_Y:
        value = chart->ymax[1];
        break;
    default:
        LV_LOG_WARN("Invalid axis: %d", axis);
        return value;
    }

    return value;
}

lv_coord_t lvx_chart_get_y_min(lv_obj_t* obj, lvx_chart_axis_t axis)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lv_coord_t value = 0;

    switch (axis) {
    case LVX_CHART_AXIS_PRIMARY_Y:
        value = chart->ymin[0];
        break;
    case LVX_CHART_AXIS_SECONDARY_Y:
        value = chart->ymin[1];
        break;
    default:
        LV_LOG_WARN("Invalid axis: %d", axis);
        return value;
    }

    return value;
}

void lvx_chart_set_update_mode(lv_obj_t* obj,
                               lvx_chart_update_mode_t update_mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->update_mode == update_mode)
        return;

    chart->update_mode = update_mode;
    lv_obj_invalidate(obj);
}

void lvx_chart_set_div_line_count(lv_obj_t* obj, uint8_t hdiv, uint8_t vdiv)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->hdiv_cnt == hdiv && chart->vdiv_cnt == vdiv)
        return;

    chart->hdiv_cnt = hdiv;
    chart->vdiv_cnt = vdiv;

    lv_obj_invalidate(obj);
}

void lvx_chart_set_zoom_x(lv_obj_t* obj, uint16_t zoom_x)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->zoom_x == zoom_x)
        return;

    chart->zoom_x = zoom_x;
    lv_obj_refresh_self_size(obj);
    /*Be the chart doesn't remain scrolled out*/
    lv_obj_readjust_scroll(obj, LV_ANIM_OFF);
    lv_obj_invalidate(obj);
}

void lvx_chart_set_zoom_y(lv_obj_t* obj, uint16_t zoom_y)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->zoom_y == zoom_y)
        return;

    chart->zoom_y = zoom_y;
    lv_obj_refresh_self_size(obj);
    /*Be the chart doesn't remain scrolled out*/
    lv_obj_readjust_scroll(obj, LV_ANIM_OFF);
    lv_obj_invalidate(obj);
}

uint16_t lvx_chart_get_zoom_x(const lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    return chart->zoom_x;
}

uint16_t lvx_chart_get_zoom_y(const lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    return chart->zoom_y;
}

void lvx_chart_set_axis_tick(lv_obj_t* obj, lvx_chart_axis_t axis,
                             lv_coord_t major_len, lv_coord_t minor_len,
                             lv_coord_t major_cnt, lv_coord_t minor_cnt,
                             bool label_en, lv_coord_t draw_size)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_tick_dsc_t* t = get_tick_gsc(obj, axis);
    t->major_len = major_len;
    t->minor_len = minor_len;
    t->minor_cnt = minor_cnt;
    t->major_cnt = major_cnt;
    t->label_en = label_en;
    t->draw_size = draw_size;

    lv_obj_refresh_ext_draw_size(obj);
    lv_obj_invalidate(obj);
}

#if LVX_CHART_EXTENTIONS == 1
void lvx_chart_set_draw_mask_below_line(lv_obj_t* obj, bool en)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->type != LVX_CHART_TYPE_LINE
        && chart->type != LVX_CHART_TYPE_STEP_LINE) {
        LV_LOG_WARN("Type must be LVX_CHART_TYPE_LINE");
        return;
    }

    lv_obj_remove_event_cb(obj, draw_line_chart_event_cb);
    if (en) {
        lv_obj_add_event_cb(obj, draw_line_chart_event_cb,
                            LV_EVENT_DRAW_PART_BEGIN, NULL);
    }
}
#endif

lvx_chart_type_t lvx_chart_get_type(const lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    return chart->type;
}

uint16_t lvx_chart_get_point_count(const lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    return chart->point_cnt;
}

uint16_t lvx_chart_get_x_start_point(const lv_obj_t* obj,
                                     lvx_chart_series_t* ser)
{
    LV_UNUSED(obj);
    LV_ASSERT_NULL(ser);

    return ser->start_point;
}

void lvx_chart_get_point_pos_by_id(lv_obj_t* obj, lvx_chart_series_t* ser,
                                   uint16_t id, lv_point_t* p_out)
{
    LV_ASSERT_NULL(obj);
    LV_ASSERT_NULL(ser);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (id >= chart->point_cnt) {
        LV_LOG_WARN("Invalid index: %d", id);
        p_out->x = 0;
        p_out->y = 0;
        return;
    }

    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    lv_coord_t h
        = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;

    if (chart->type == LVX_CHART_TYPE_LINE) {
        p_out->x = (w * id) / (chart->point_cnt - 1);
    } else if (chart->type == LVX_CHART_TYPE_SCATTER
               || chart->type == LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
               || chart->type == LVX_CHART_TYPE_POINT_BAR
#endif
    ) {
        p_out->x = lv_map(ser->x_points[id], chart->xmin[ser->x_axis_sec],
                          chart->xmax[ser->x_axis_sec], 0, w);
    } else if (chart->type == LVX_CHART_TYPE_BAR) {
        uint32_t ser_cnt = _lv_ll_get_len(&chart->series_ll);
        int32_t ser_gap
            = ((int32_t)lv_obj_get_style_pad_column(obj, LV_PART_ITEMS)
               * chart->zoom_x)
            >> 8; /*Gap between the column on the ~same X*/
        int32_t block_gap
            = ((int32_t)lv_obj_get_style_pad_column(obj, LV_PART_MAIN)
               * chart->zoom_x)
            >> 8; /*Gap between the column on ~adjacent X*/
        lv_coord_t block_w
            = (w - ((chart->point_cnt - 1) * block_gap)) / chart->point_cnt;
        lv_coord_t col_w = block_w / ser_cnt;

        p_out->x = (int32_t)((int32_t)w * id) / chart->point_cnt;

        lvx_chart_series_t* ser_i = NULL;
        _LV_LL_READ_BACK(&chart->series_ll, ser_i)
        {
            if (ser_i == ser)
                break;
            p_out->x += col_w;
        }

        p_out->x += (col_w - ser_gap) / 2;
    }

    lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    p_out->x += lv_obj_get_style_pad_left(obj, LV_PART_MAIN) + border_width;
    p_out->x -= lv_obj_get_scroll_left(obj);

    int32_t temp_y = 0;
    temp_y
        = (int32_t)((int32_t)ser->y_points[id] - chart->ymin[ser->y_axis_sec])
        * h;
    temp_y = temp_y
        / (chart->ymax[ser->y_axis_sec] - chart->ymin[ser->y_axis_sec]);
    p_out->y = h - temp_y;
    p_out->y += lv_obj_get_style_pad_top(obj, LV_PART_MAIN) + border_width;
    p_out->y -= lv_obj_get_scroll_top(obj);
}

void lvx_chart_refresh(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_obj_invalidate(obj);
}

/*======================
 * Series
 *=====================*/

lvx_chart_series_t* lvx_chart_add_series(lv_obj_t* obj, lv_color_t color,
                                         lvx_chart_axis_t axis)
{
    LV_LOG_INFO("begin");

    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lvx_chart_series_t* ser = _lv_ll_ins_head(&chart->series_ll);
    LV_ASSERT_MALLOC(ser);
    if (ser == NULL)
        return NULL;

#if LVX_CHART_EXTENTIONS == 1
    lv_memset_00(ser, sizeof(lvx_chart_series_t));
#endif

    lv_coord_t def = LVX_CHART_POINT_NONE;

    ser->color = color;
    ser->y_points = lv_mem_alloc(sizeof(lv_coord_t) * chart->point_cnt);
    LV_ASSERT_MALLOC(ser->y_points);

    if (chart->type == LVX_CHART_TYPE_SCATTER
        || chart->type == LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
        || chart->type == LVX_CHART_TYPE_POINT_BAR
#endif
    ) {
        ser->x_points = lv_mem_alloc(sizeof(lv_coord_t) * chart->point_cnt);
        LV_ASSERT_MALLOC(ser->x_points);
#if LVX_CHART_EXTENTIONS == 1
        for (uint16_t j = 0; j < chart->point_cnt; j++) {
            ser->x_points[j] = def;
        }
#endif
    }
    if (ser->y_points == NULL) {
        _lv_ll_remove(&chart->series_ll, ser);
        lv_mem_free(ser);
        return NULL;
    }

    ser->start_point = 0;
    ser->x_ext_buf_assigned = false;
    ser->y_ext_buf_assigned = false;
    ser->hidden = 0;
    ser->x_axis_sec = axis & LVX_CHART_AXIS_SECONDARY_X ? 1 : 0;
    ser->y_axis_sec = axis & LVX_CHART_AXIS_SECONDARY_Y ? 1 : 0;
#if LVX_CHART_EXTENTIONS == 1
    ser->threshold = 10;
#endif

    uint16_t i;
    lv_coord_t* p_tmp = ser->y_points;
    for (i = 0; i < chart->point_cnt; i++) {
        *p_tmp = def;
        p_tmp++;
    }

    return ser;
}

void lvx_chart_remove_series(lv_obj_t* obj, lvx_chart_series_t* series)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(series);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (!series->y_ext_buf_assigned && series->y_points)
        lv_mem_free(series->y_points);

    _lv_ll_remove(&chart->series_ll, series);
    lv_mem_free(series);

    return;
}

void lvx_chart_hide_series(lv_obj_t* chart, lvx_chart_series_t* series,
                           bool hide)
{
    LV_ASSERT_OBJ(chart, MY_CLASS);
    LV_ASSERT_NULL(series);

    series->hidden = hide ? 1 : 0;
    lvx_chart_refresh(chart);
}

void lvx_chart_set_series_color(lv_obj_t* chart, lvx_chart_series_t* series,
                                lv_color_t color)
{
    LV_ASSERT_OBJ(chart, MY_CLASS);
    LV_ASSERT_NULL(series);

    series->color = color;
    lvx_chart_refresh(chart);
}

#if LVX_CHART_EXTENTIONS == 1
void lvx_chart_set_series_threshold(lv_obj_t* chart, lvx_chart_series_t* series,
                                    lv_coord_t threshold)
{
    LV_ASSERT_OBJ(chart, MY_CLASS);
    LV_ASSERT_NULL(series);

    series->threshold = threshold;
    lvx_chart_refresh(chart);
}
#endif

void lvx_chart_set_x_start_point(lv_obj_t* obj, lvx_chart_series_t* ser,
                                 uint16_t id)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (id >= chart->point_cnt)
        return;
    ser->start_point = id;
}

lvx_chart_series_t* lvx_chart_get_series_next(const lv_obj_t* obj,
                                              const lvx_chart_series_t* ser)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (ser == NULL)
        return _lv_ll_get_head(&chart->series_ll);
    else
        return _lv_ll_get_next(&chart->series_ll, ser);
}

/*=====================
 * Cursor
 *====================*/

/**
 * Add a cursor with a given color
 * @param chart     pointer to chart object
 * @param color     color of the cursor
 * @param dir       direction of the cursor.
 * `LV_DIR_RIGHT/LEFT/TOP/DOWN/HOR/VER/ALL`. OR-ed values are possible
 * @return          pointer to the created cursor
 */
lvx_chart_cursor_t* lvx_chart_add_cursor(lv_obj_t* obj, lv_color_t color,
                                         lv_dir_t dir)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lvx_chart_cursor_t* cursor = _lv_ll_ins_head(&chart->cursor_ll);
    LV_ASSERT_MALLOC(cursor);
    if (cursor == NULL)
        return NULL;

    cursor->pos.x = LVX_CHART_POINT_NONE;
    cursor->pos.y = LVX_CHART_POINT_NONE;
    cursor->point_id = LVX_CHART_POINT_NONE;
    cursor->pos_set = 0;
    cursor->color = color;
    cursor->dir = dir;

    return cursor;
}

/**
 * Set the coordinate of the cursor with respect
 * to the origin of series area of the chart.
 * @param chart pointer to a chart object.
 * @param cursor pointer to the cursor.
 * @param pos the new coordinate of cursor relative to the series area
 */
void lvx_chart_set_cursor_pos(lv_obj_t* chart, lvx_chart_cursor_t* cursor,
                              lv_point_t* pos)
{
    LV_ASSERT_NULL(cursor);
    LV_UNUSED(chart);

    cursor->pos.x = pos->x;
    cursor->pos.y = pos->y;
    cursor->pos_set = 1;
    lvx_chart_refresh(chart);
}

/**
 * Set the coordinate of the cursor with respect
 * to the origin of series area of the chart.
 * @param chart pointer to a chart object.
 * @param cursor pointer to the cursor.
 * @param pos the new coordinate of cursor relative to the series area
 */
void lvx_chart_set_cursor_point(lv_obj_t* chart, lvx_chart_cursor_t* cursor,
                                lvx_chart_series_t* ser, uint16_t point_id)
{
    LV_ASSERT_NULL(cursor);
    LV_UNUSED(chart);

    cursor->point_id = point_id;
    cursor->pos_set = 0;
    if (ser == NULL)
        ser = lvx_chart_get_series_next(chart, NULL);
    cursor->ser = ser;
    lvx_chart_refresh(chart);
}
/**
 * Get the coordinate of the cursor with respect
 * to the origin of series area of the chart.
 * @param chart pointer to a chart object
 * @param cursor pointer to cursor
 * @return coordinate of the cursor as lv_point_t
 */
lv_point_t lvx_chart_get_cursor_point(lv_obj_t* chart,
                                      lvx_chart_cursor_t* cursor)
{
    LV_ASSERT_NULL(cursor);
    LV_UNUSED(chart);

    return cursor->pos;
}

/*=====================
 * Set/Get value(s)
 *====================*/

void lvx_chart_set_all_value(lv_obj_t* obj, lvx_chart_series_t* ser,
                             lv_coord_t value)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    uint16_t i;
    for (i = 0; i < chart->point_cnt; i++) {
        ser->y_points[i] = value;
    }
    ser->start_point = 0;
    lvx_chart_refresh(obj);
}

void lvx_chart_set_next_value(lv_obj_t* obj, lvx_chart_series_t* ser,
                              lv_coord_t value)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    ser->y_points[ser->start_point] = value;
    invalidate_point(obj, ser->start_point);
    ser->start_point = (ser->start_point + 1) % chart->point_cnt;
    invalidate_point(obj, ser->start_point);
}

void lvx_chart_set_next_value2(lv_obj_t* obj, lvx_chart_series_t* ser,
                               lv_coord_t x_value, lv_coord_t y_value)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);

    lvx_chart_t* chart = (lvx_chart_t*)obj;

    if (chart->type != LVX_CHART_TYPE_SCATTER
        && chart->type != LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
        && chart->type != LVX_CHART_TYPE_POINT_BAR
#endif
    ) {
        LV_LOG_WARN("Type must be LVX_CHART_TYPE_SCATTER or "
                    "LVX_CHART_TYPE_POINT_BAR");
        return;
    }

    ser->x_points[ser->start_point] = x_value;
    ser->y_points[ser->start_point] = y_value;
    invalidate_point(obj, ser->start_point);
    ser->start_point = (ser->start_point + 1) % chart->point_cnt;
    invalidate_point(obj, ser->start_point);
    lvx_chart_refresh(obj);
}

void lvx_chart_set_value_by_id(lv_obj_t* obj, lvx_chart_series_t* ser,
                               uint16_t id, lv_coord_t value)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);
    lvx_chart_t* chart = (lvx_chart_t*)obj;

    if (id >= chart->point_cnt)
        return;
    ser->y_points[id] = value;
    lvx_chart_refresh(obj);
}

void lvx_chart_set_value_by_id2(lv_obj_t* obj, lvx_chart_series_t* ser,
                                uint16_t id, lv_coord_t x_value,
                                lv_coord_t y_value)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);
    lvx_chart_t* chart = (lvx_chart_t*)obj;

    if (chart->type != LVX_CHART_TYPE_SCATTER
        && chart->type != LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
        && chart->type != LVX_CHART_TYPE_POINT_BAR
#endif
    ) {
        LV_LOG_WARN("Type must be LVX_CHART_TYPE_SCATTER or "
                    "LVX_CHART_TYPE_POINT_BAR");
        return;
    }

    if (id >= chart->point_cnt)
        return;
    ser->x_points[id] = x_value;
    ser->y_points[id] = y_value;
}

void lvx_chart_set_ext_y_array(lv_obj_t* obj, lvx_chart_series_t* ser,
                               lv_coord_t array[])
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);

    if (!ser->y_ext_buf_assigned && ser->y_points)
        lv_mem_free(ser->y_points);
    ser->y_ext_buf_assigned = true;
    ser->y_points = array;
    lv_obj_invalidate(obj);
}

void lvx_chart_set_ext_x_array(lv_obj_t* obj, lvx_chart_series_t* ser,
                               lv_coord_t array[])
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);

    if (!ser->x_ext_buf_assigned && ser->x_points)
        lv_mem_free(ser->x_points);
    ser->x_ext_buf_assigned = true;
    ser->x_points = array;
    lv_obj_invalidate(obj);
}

lv_coord_t* lvx_chart_get_y_array(const lv_obj_t* obj, lvx_chart_series_t* ser)
{
    LV_UNUSED(obj);
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);
    return ser->y_points;
}

lv_coord_t* lvx_chart_get_x_array(const lv_obj_t* obj, lvx_chart_series_t* ser)
{
    LV_UNUSED(obj);
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(ser);
    return ser->x_points;
}

uint32_t lvx_chart_get_pressed_point(const lv_obj_t* obj)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    return chart->pressed_point_id;
}

void lvx_chart_set_const_value(lv_obj_t* obj, lv_coord_t value)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    chart->has_const_value = true;
    chart->const_value = value;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_chart_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lvx_chart_t* chart = (lvx_chart_t*)obj;

    _lv_ll_init(&chart->series_ll, sizeof(lvx_chart_series_t));
    _lv_ll_init(&chart->cursor_ll, sizeof(lvx_chart_cursor_t));

    chart->ymin[0] = 0;
    chart->xmin[0] = 0;
    chart->ymin[1] = 0;
    chart->xmin[1] = 0;
    chart->ymax[0] = 100;
    chart->xmax[0] = 100;
    chart->ymax[1] = 100;
    chart->xmax[1] = 100;

    chart->hdiv_cnt = LVX_CHART_HDIV_DEF;
    chart->vdiv_cnt = LVX_CHART_VDIV_DEF;
    chart->point_cnt = LVX_CHART_POINT_CNT_DEF;
    chart->pressed_point_id = LVX_CHART_POINT_NONE;
    chart->type = LVX_CHART_TYPE_LINE;
    chart->update_mode = LVX_CHART_UPDATE_MODE_SHIFT;
    chart->zoom_x = LV_IMG_ZOOM_NONE;
    chart->zoom_y = LV_IMG_ZOOM_NONE;
    chart->has_const_value = false;
    chart->const_value = 0;

    LV_TRACE_OBJ_CREATE("finished");
}

static void lvx_chart_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lvx_chart_series_t* ser;
    while (chart->series_ll.head) {
        ser = _lv_ll_get_head(&chart->series_ll);

        if (!ser->y_ext_buf_assigned)
            lv_mem_free(ser->y_points);

        _lv_ll_remove(&chart->series_ll, ser);
        lv_mem_free(ser);
    }
    _lv_ll_clear(&chart->series_ll);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lvx_chart_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);

    /*Call the ancestor's event handler*/
    lv_res_t res;

    lv_event_code_t code = lv_event_get_code(e);

    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;

    lv_obj_t* obj = lv_event_get_target(e);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (code == LV_EVENT_PRESSED) {
        lv_indev_t* indev = lv_indev_get_act();
        lv_point_t p;
        lv_indev_get_point(indev, &p);

        p.x -= obj->coords.x1;
        uint32_t id = get_index_from_x(obj, p.x + lv_obj_get_scroll_left(obj));
        if (id != chart->pressed_point_id) {
            invalidate_point(obj, id);
            invalidate_point(obj, chart->pressed_point_id);
            chart->pressed_point_id = id;
            lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
        }
    } else if (code == LV_EVENT_RELEASED) {
        invalidate_point(obj, chart->pressed_point_id);
        chart->pressed_point_id = LVX_CHART_POINT_NONE;
    } else if (code == LV_EVENT_SIZE_CHANGED) {
        lv_obj_refresh_self_size(obj);
    } else if (code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_event_set_ext_draw_size(
            e,
            LV_MAX4(chart->tick[0].draw_size, chart->tick[1].draw_size,
                    chart->tick[2].draw_size, chart->tick[3].draw_size));
    } else if (code == LV_EVENT_GET_SELF_SIZE) {
        lv_point_t* p = lv_event_get_param(e);
        p->x = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
        p->y = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
    } else if (code == LV_EVENT_DRAW_MAIN) {
        lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);
        draw_div_lines(obj, draw_ctx);
        draw_axes(obj, draw_ctx);

#if LVX_CHART_EXTENTIONS == 1
        draw_avg_line(obj, draw_ctx, chart->has_const_value);
#endif

        if (_lv_ll_is_empty(&chart->series_ll) == false) {
            switch (chart->type) {
            case LVX_CHART_TYPE_LINE:
            case LVX_CHART_TYPE_STEP_LINE:
                draw_series_line(obj, draw_ctx);
                break;
            case LVX_CHART_TYPE_BAR:
                draw_series_bar(obj, draw_ctx);
                break;
            case LVX_CHART_TYPE_SCATTER:
                draw_series_scatter(obj, draw_ctx);
                break;
#if LVX_CHART_EXTENTIONS == 1
            case LVX_CHART_TYPE_POINT_BAR:
                draw_series_point_bar(obj, draw_ctx);
                break;
#endif
            default:
                break;
            }
        }

        draw_cursors(obj, draw_ctx);
    }
}

static void draw_div_lines(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;

    lv_area_t series_clip_area;
    bool mask_ret = _lv_area_intersect(&series_clip_area, &obj->coords,
                                       draw_ctx->clip_area);
    if (mask_ret == false)
        return;

    const lv_area_t* clip_area_ori = draw_ctx->clip_area;
    draw_ctx->clip_area = &series_clip_area;

    int16_t i;
    int16_t i_start;
    int16_t i_end;
    lv_point_t p1;
    lv_point_t p2;
    lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_coord_t pad_left
        = lv_obj_get_style_pad_left(obj, LV_PART_MAIN) + border_width;
    lv_coord_t pad_top
        = lv_obj_get_style_pad_top(obj, LV_PART_MAIN) + border_width;
    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    lv_coord_t h
        = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &line_dsc);

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.part = LV_PART_MAIN;
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LVX_CHART_DRAW_PART_DIV_LINE_INIT;
    part_draw_dsc.line_dsc = &line_dsc;
    part_draw_dsc.id = 0xFFFFFFFF;
    part_draw_dsc.p1 = NULL;
    part_draw_dsc.p2 = NULL;
    lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);

    lv_opa_t border_opa = lv_obj_get_style_border_opa(obj, LV_PART_MAIN);
    lv_coord_t border_w = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_border_side_t border_side
        = lv_obj_get_style_border_side(obj, LV_PART_MAIN);

    lv_coord_t scroll_left = lv_obj_get_scroll_left(obj);
    lv_coord_t scroll_top = lv_obj_get_scroll_top(obj);
    if (chart->hdiv_cnt != 0) {
        lv_coord_t y_ofs = obj->coords.y1 + pad_top - scroll_top;
        p1.x = obj->coords.x1;
        p2.x = obj->coords.x2;

        i_start = 0;
        i_end = chart->hdiv_cnt;
        if (border_opa > LV_OPA_MIN && border_w > 0) {
            if ((border_side & LV_BORDER_SIDE_TOP)
                && (lv_obj_get_style_pad_top(obj, LV_PART_MAIN) == 0))
                i_start++;
            if ((border_side & LV_BORDER_SIDE_BOTTOM)
                && (lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN) == 0))
                i_end--;
        }

        for (i = i_start; i < i_end; i++) {
            p1.y = (int32_t)((int32_t)h * i) / (chart->hdiv_cnt - 1);
            p1.y += y_ofs;
            p2.y = p1.y;

            part_draw_dsc.class_p = MY_CLASS;
            part_draw_dsc.type = LVX_CHART_DRAW_PART_DIV_LINE_HOR;
            part_draw_dsc.p1 = &p1;
            part_draw_dsc.p2 = &p2;
            part_draw_dsc.id = i;

            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
            lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
            lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
        }
    }

    if (chart->vdiv_cnt != 0) {
        lv_coord_t x_ofs = obj->coords.x1 + pad_left - scroll_left;
        p1.y = obj->coords.y1;
        p2.y = obj->coords.y2;
        i_start = 0;
        i_end = chart->vdiv_cnt;
        if (border_opa > LV_OPA_MIN && border_w > 0) {
            if ((border_side & LV_BORDER_SIDE_LEFT)
                && (lv_obj_get_style_pad_left(obj, LV_PART_MAIN) == 0))
                i_start++;
            if ((border_side & LV_BORDER_SIDE_RIGHT)
                && (lv_obj_get_style_pad_right(obj, LV_PART_MAIN) == 0))
                i_end--;
        }

        for (i = i_start; i < i_end; i++) {
            p1.x = (int32_t)((int32_t)w * i) / (chart->vdiv_cnt - 1);
            p1.x += x_ofs;
            p2.x = p1.x;

            part_draw_dsc.class_p = MY_CLASS;
            part_draw_dsc.type = LVX_CHART_DRAW_PART_DIV_LINE_VER;
            part_draw_dsc.p1 = &p1;
            part_draw_dsc.p2 = &p2;
            part_draw_dsc.id = i;

            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
            lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
            lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
        }
    }

    part_draw_dsc.id = 0xFFFFFFFF;
    part_draw_dsc.p1 = NULL;
    part_draw_dsc.p2 = NULL;
    lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);

    draw_ctx->clip_area = clip_area_ori;
}

static void draw_series_line(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx)
{
    lv_area_t clip_area;
    if (!_lv_area_intersect(&clip_area, &obj->coords, draw_ctx->clip_area))
        return;

    const lv_area_t* clip_area_ori = draw_ctx->clip_area;
    draw_ctx->clip_area = &clip_area;

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (chart->point_cnt < 2)
        return;

    uint16_t i;
    lv_point_t p1;
    lv_point_t p2;
    lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_coord_t pad_left, pad_top, w, h;
    pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN) + border_width;
    pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN) + border_width;
    w = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    h = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
    lv_coord_t x_ofs = obj->coords.x1 + pad_left - lv_obj_get_scroll_left(obj);
    lv_coord_t y_ofs = obj->coords.y1 + pad_top - lv_obj_get_scroll_top(obj);
    lvx_chart_series_t* ser;

    lv_area_t series_clip_area;
    bool mask_ret = _lv_area_intersect(&series_clip_area, &obj->coords,
                                       draw_ctx->clip_area);
    if (mask_ret == false)
        return;

    lv_draw_line_dsc_t line_dsc_default;
    lv_draw_line_dsc_init(&line_dsc_default);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &line_dsc_default);

    lv_draw_rect_dsc_t point_dsc_default;
    lv_draw_rect_dsc_init(&point_dsc_default);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_INDICATOR, &point_dsc_default);

    lv_coord_t point_w = lv_obj_get_style_width(obj, LV_PART_INDICATOR) / 2;
    lv_coord_t point_h = lv_obj_get_style_height(obj, LV_PART_INDICATOR) / 2;

    /*Do not bother with line ending is the point will over it*/
    if (LV_MIN(point_w, point_h) > line_dsc_default.width / 2)
        line_dsc_default.raw_end = 1;
    if (line_dsc_default.width == 1)
        line_dsc_default.raw_end = 1;

    /*If there are more points than pixels draw only vertical lines*/
    bool crowded_mode = chart->point_cnt >= w ? true : false;

    /*Go through all data lines*/
    _LV_LL_READ_BACK(&chart->series_ll, ser)
    {
        if (ser->hidden)
            continue;
        line_dsc_default.color = ser->color;
        point_dsc_default.bg_color = ser->color;

        lv_coord_t start_point
            = chart->update_mode == LVX_CHART_UPDATE_MODE_SHIFT
            ? ser->start_point
            : 0;

        p1.x = 0;
        p2.x = 0;

        lv_coord_t p_act = start_point;
        lv_coord_t p_prev = start_point;

        if (chart->type == LVX_CHART_TYPE_STEP_LINE) {
            p2.x = lv_map(ser->x_points[p_act], chart->xmin[ser->x_axis_sec],
                          chart->xmax[ser->x_axis_sec], 0, w);
        }
        p2.x += x_ofs;

        /** the y coordinate direction is from top to bottom */
        p2.y = lv_map(ser->y_points[p_act], chart->ymin[ser->y_axis_sec],
                      chart->ymax[ser->y_axis_sec], h, 0);
        p2.y += y_ofs;

        lv_obj_draw_part_dsc_t part_draw_dsc;
        lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
        part_draw_dsc.class_p = MY_CLASS;
        part_draw_dsc.type = LVX_CHART_DRAW_PART_LINE_AND_POINT;
        part_draw_dsc.part = LV_PART_ITEMS;
        part_draw_dsc.line_dsc = &line_dsc_default;
        part_draw_dsc.rect_dsc = &point_dsc_default;
        part_draw_dsc.sub_part_ptr = ser;

        lv_coord_t y_min = p2.y;
        lv_coord_t y_max = p2.y;

        for (i = 0; i < chart->point_cnt; i++) {
            p1.x = p2.x;
            p1.y = p2.y;

            if (p1.x > clip_area_ori->x2 + point_w + 1)
                break;

            p_act = (start_point + i) % chart->point_cnt;

            if (chart->type == LVX_CHART_TYPE_STEP_LINE) {
                p2.x
                    = lv_map(ser->x_points[p_act], chart->xmin[ser->x_axis_sec],
                             chart->xmax[ser->x_axis_sec], 0, w);
            } else {
                p2.x = lv_map(i, 0, chart->point_cnt - 1, 0, w);
            }
            p2.x += x_ofs;

            p2.y = lv_map(ser->y_points[p_act], chart->ymin[ser->y_axis_sec],
                          chart->ymax[ser->y_axis_sec], h, 0);
            p2.y += y_ofs;

            if (p2.x < clip_area_ori->x1 - point_w - 1) {
                p_prev = p_act;
                continue;
            }

            /*Don't draw the first point. A second point is also required to
             * draw the line*/
            if (i != 0) {
                if (crowded_mode) {
                    if (ser->y_points[p_prev] != LVX_CHART_POINT_NONE
                        && ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
                        /*Draw only one vertical line between the min an max y
                         * values on the same x value*/
                        y_max = LV_MAX(y_max, p2.y);
                        y_min = LV_MIN(y_min, p2.y);
                        if (p1.x != p2.x) {
                            lv_coord_t y_cur = p2.y;
                            p2.x--; /*It's already on the next x value*/
                            p1.x = p2.x;
                            p1.y = y_min;
                            p2.y = y_max;
                            if (p1.y == p2.y)
                                p2.y++; /*If they are the same no line will be
                                           drawn*/
                            lv_draw_line(draw_ctx, &line_dsc_default, &p1, &p2);
                            p2.x++; /*Compensate the previous x--*/
                            y_min = y_cur; /*Start the line of the next x from
                                              the current last y*/
                            y_max = y_cur;
                        }
                    }
                } else {
                    lv_area_t point_area;
                    point_area.x1 = p1.x - point_w;
                    point_area.x2 = p1.x + point_w;
                    point_area.y1 = p1.y - point_h;
                    point_area.y2 = p1.y + point_h;

                    part_draw_dsc.id = i - 1;
                    part_draw_dsc.p1
                        = ser->y_points[p_prev] != LVX_CHART_POINT_NONE ? &p1
                                                                        : NULL;
                    part_draw_dsc.p2
                        = ser->y_points[p_act] != LVX_CHART_POINT_NONE ? &p2
                                                                       : NULL;
                    part_draw_dsc.draw_area = &point_area;
                    part_draw_dsc.value = ser->y_points[p_prev];

                    lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN,
                                  &part_draw_dsc);

                    if (ser->y_points[p_prev] != LVX_CHART_POINT_NONE
                        && ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
                        lv_draw_line(draw_ctx, &line_dsc_default, &p1, &p2);
                    }

                    if (point_w && point_h
                        && ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
                        lv_draw_rect(draw_ctx, &point_dsc_default, &point_area);
                    }

                    lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
                }
            }
            p_prev = p_act;
        }

        /*Draw the last point*/
        if (!crowded_mode && i == chart->point_cnt) {

            if (ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
                lv_area_t point_area;
                point_area.x1 = p2.x - point_w;
                point_area.x2 = p2.x + point_w;
                point_area.y1 = p2.y - point_h;
                point_area.y2 = p2.y + point_h;

                part_draw_dsc.id = i - 1;
                part_draw_dsc.p1 = NULL;
                part_draw_dsc.p2 = NULL;
                part_draw_dsc.draw_area = &point_area;
                part_draw_dsc.value = ser->y_points[p_act];
                lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
                lv_draw_rect(draw_ctx, &point_dsc_default, &point_area);
                lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
            }
        }
    }
    draw_ctx->clip_area = clip_area_ori;
}

static void draw_series_scatter(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx)
{
    lv_area_t clip_area;
    if (!_lv_area_intersect(&clip_area, &obj->coords, draw_ctx->clip_area))
        return;

    const lv_area_t* clip_area_ori = draw_ctx->clip_area;
    draw_ctx->clip_area = &clip_area;

    lvx_chart_t* chart = (lvx_chart_t*)obj;

    uint16_t i;
    lv_point_t p1;
    lv_point_t p2;
    lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    lv_coord_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    lv_coord_t w, h, x_ofs, y_ofs;
    w = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    h = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
    x_ofs = obj->coords.x1 + pad_left + border_width
        - lv_obj_get_scroll_left(obj);
    y_ofs
        = obj->coords.y1 + pad_top + border_width - lv_obj_get_scroll_top(obj);
    lvx_chart_series_t* ser;

    lv_draw_line_dsc_t line_dsc_default;
    lv_draw_line_dsc_init(&line_dsc_default);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &line_dsc_default);

    lv_draw_rect_dsc_t point_dsc_default;
    lv_draw_rect_dsc_init(&point_dsc_default);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_INDICATOR, &point_dsc_default);

    lv_coord_t point_w = lv_obj_get_style_width(obj, LV_PART_INDICATOR) / 2;
    lv_coord_t point_h = lv_obj_get_style_height(obj, LV_PART_INDICATOR) / 2;

    /*Do not bother with line ending is the point will over it*/
    if (LV_MIN(point_w, point_h) > line_dsc_default.width / 2)
        line_dsc_default.raw_end = 1;
    if (line_dsc_default.width == 1)
        line_dsc_default.raw_end = 1;

    /*Go through all data lines*/
    _LV_LL_READ_BACK(&chart->series_ll, ser)
    {
        if (ser->hidden)
            continue;
        line_dsc_default.color = ser->color;
        point_dsc_default.bg_color = ser->color;

        lv_coord_t start_point
            = chart->update_mode == LVX_CHART_UPDATE_MODE_SHIFT
            ? ser->start_point
            : 0;

        p1.x = x_ofs;
        p2.x = x_ofs;

        lv_coord_t p_act = start_point;
        lv_coord_t p_prev = start_point;
        if (ser->y_points[p_act] != LVX_CHART_POINT_CNT_DEF) {
            p2.x = lv_map(ser->x_points[p_act], chart->xmin[ser->x_axis_sec],
                          chart->xmax[ser->x_axis_sec], 0, w);
            p2.x += x_ofs;

            p2.y = lv_map(ser->y_points[p_act], chart->ymin[ser->y_axis_sec],
                          chart->ymax[ser->y_axis_sec], h, 0);
            p2.y += y_ofs;
        } else {
            p2.x = LV_COORD_MIN;
            p2.y = LV_COORD_MIN;
        }

        lv_obj_draw_part_dsc_t part_draw_dsc;
        lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
        part_draw_dsc.part = LV_PART_ITEMS;
        part_draw_dsc.class_p = MY_CLASS;
        part_draw_dsc.type = LVX_CHART_DRAW_PART_LINE_AND_POINT;
        part_draw_dsc.line_dsc = &line_dsc_default;
        part_draw_dsc.rect_dsc = &point_dsc_default;
        part_draw_dsc.sub_part_ptr = ser;

        for (i = 0; i < chart->point_cnt; i++) {
            p1.x = p2.x;
            p1.y = p2.y;

            p_act = (start_point + i) % chart->point_cnt;
            if (ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
                p2.y
                    = lv_map(ser->y_points[p_act], chart->ymin[ser->y_axis_sec],
                             chart->ymax[ser->y_axis_sec], h, 0);
                p2.y += y_ofs;

                p2.x
                    = lv_map(ser->x_points[p_act], chart->xmin[ser->x_axis_sec],
                             chart->xmax[ser->x_axis_sec], 0, w);
                p2.x += x_ofs;
            } else {
                p_prev = p_act;
                continue;
            }

            /*Don't draw the first point. A second point is also required to
             * draw the line*/
            if (i != 0) {
                lv_area_t point_area;
                point_area.x1 = p1.x - point_w;
                point_area.x2 = p1.x + point_w;
                point_area.y1 = p1.y - point_h;
                point_area.y2 = p1.y + point_h;

                part_draw_dsc.id = i - 1;
                part_draw_dsc.p1 = ser->y_points[p_prev] != LVX_CHART_POINT_NONE
                    ? &p1
                    : NULL;
                part_draw_dsc.p2
                    = ser->y_points[p_act] != LVX_CHART_POINT_NONE ? &p2 : NULL;
                part_draw_dsc.draw_area = &point_area;
                part_draw_dsc.value = ser->y_points[p_prev];

                lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);

                if (ser->y_points[p_prev] != LVX_CHART_POINT_NONE
                    && ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
                    lv_draw_line(draw_ctx, &line_dsc_default, &p1, &p2);
                    if (point_w && point_h) {
                        lv_draw_rect(draw_ctx, &point_dsc_default, &point_area);
                    }
                }

                lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
            }
            p_prev = p_act;
        }

        /*Draw the last point*/
        if (i == chart->point_cnt) {

            if (ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
                lv_area_t point_area;
                point_area.x1 = p2.x - point_w;
                point_area.x2 = p2.x + point_w;
                point_area.y1 = p2.y - point_h;
                point_area.y2 = p2.y + point_h;

                part_draw_dsc.id = i - 1;
                part_draw_dsc.p1 = NULL;
                part_draw_dsc.p2 = NULL;
                part_draw_dsc.draw_area = &point_area;
                part_draw_dsc.value = ser->y_points[p_act];
                lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
                lv_draw_rect(draw_ctx, &point_dsc_default, &point_area);
                lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
            }
        }
    }
    draw_ctx->clip_area = clip_area_ori;
}

#if LVX_CHART_EXTENTIONS == 1
static void draw_series_point_bar(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx)
{
    lv_area_t com_area;
    if (_lv_area_intersect(&com_area, &obj->coords, draw_ctx->clip_area)
        == false)
        return;

    lvx_chart_t* chart = (lvx_chart_t*)obj;

    lv_point_t p1;
    lv_point_t p2;
    lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    lv_coord_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    lv_coord_t h
        = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
    lv_coord_t x_ofs = obj->coords.x1 + pad_left + border_width
        - lv_obj_get_scroll_left(obj);
    lv_coord_t y_ofs
        = obj->coords.y1 + pad_top + border_width - lv_obj_get_scroll_top(obj);

    lvx_chart_series_t* ser;

    lv_area_t series_mask;
    bool mask_ret
        = _lv_area_intersect(&series_mask, &obj->coords, draw_ctx->clip_area);
    if (mask_ret == false)
        return;

    lv_draw_line_dsc_t line_dsc_default;
    lv_draw_line_dsc_init(&line_dsc_default);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &line_dsc_default);

    lv_draw_rect_dsc_t point_dsc_default;
    lv_draw_rect_dsc_init(&point_dsc_default);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_INDICATOR, &point_dsc_default);

    lv_coord_t point_w = lv_obj_get_style_width(obj, LV_PART_INDICATOR) / 2;
    lv_coord_t point_h = lv_obj_get_style_height(obj, LV_PART_INDICATOR) / 2;

    /* The edge of the point in the middle of the line */
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &line_dsc);
    if (line_dsc.width > 0 && line_dsc.opa > LV_OPA_MIN) {
        if (chart->vdiv_cnt) {
            x_ofs += point_w;
            w -= (line_dsc.width + point_w * 2);
        }
        if (chart->hdiv_cnt) {
            y_ofs += point_h;
            h -= (line_dsc.width + point_w * 2);
        }
    }

    /*Do not bother with line ending is the point will over it*/
    if (LV_MIN(point_w, point_h) > line_dsc_default.width / 2)
        line_dsc_default.raw_end = 1;
    if (line_dsc_default.width == 1)
        line_dsc_default.raw_end = 1;

    /*Go through all data lines*/
    _LV_LL_READ_BACK(&chart->series_ll, ser)
    {
        if (ser->hidden)
            continue;

        /* Cache data */
        lv_coord_t point_cnt = chart->point_cnt;
        lv_point_t* points = lv_mem_buf_get(sizeof(lv_point_t) * point_cnt);
        if (!points) {
            continue;
        }
        lv_memset_00(points, sizeof(lv_point_t) * point_cnt);

        for (lv_coord_t i = 0; i < point_cnt; i++) {
            points[i].x = ser->x_points[i];
            points[i].y = ser->y_points[i];
        }

        lv_obj_draw_part_dsc_t part_draw_dsc;
        lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);

        /* Sort data */
        qsort(points, point_cnt, sizeof(lv_point_t), point_cmp);

        line_dsc_default.color = ser->color;
        point_dsc_default.bg_color = ser->color;

        part_draw_dsc.part = LV_PART_ITEMS;
        part_draw_dsc.class_p = MY_CLASS;
        part_draw_dsc.type = LVX_CHART_DRAW_PART_LINE_AND_POINT;
        part_draw_dsc.line_dsc = &line_dsc_default;
        part_draw_dsc.rect_dsc = &point_dsc_default;
        part_draw_dsc.sub_part_ptr = ser;

        for (lv_coord_t i = 0; i < point_cnt; i++) {
            if (points[i].x == LVX_CHART_POINT_NONE
                || points[i].y == LVX_CHART_POINT_NONE) {
                continue;
            }

            p1.x = lv_map(points[i].x, chart->xmin[ser->x_axis_sec],
                          chart->xmax[ser->x_axis_sec], 0, w);
            p1.x += x_ofs;
            p1.y = lv_map(points[i].y, chart->ymin[ser->y_axis_sec],
                          chart->ymax[ser->y_axis_sec], 0, h);
            p1.y = h - p1.y;
            p1.y += y_ofs;

            lv_area_t point_area;
            point_area.x1 = p1.x - point_w;
            point_area.x2 = p1.x + point_w;
            point_area.y1 = p1.y - point_h;
            point_area.y2 = p1.y + point_h;
            part_draw_dsc.id = i;
            part_draw_dsc.p1 = &p1;
            part_draw_dsc.p2 = NULL;
            part_draw_dsc.draw_area = &point_area;
            part_draw_dsc.value = points[i].y;

            if (point_w && point_h) {
                lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
                lv_draw_rect(draw_ctx, &point_dsc_default, &point_area);
                lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
                if (i == point_cnt - 1) {
                    break;
                }
            }

            if (points[i + 1].x == LVX_CHART_POINT_CNT_DEF
                && points[i + 1].y == LVX_CHART_POINT_CNT_DEF) {
                continue;
            }

            p2.x = lv_map(points[i + 1].x, chart->xmin[ser->x_axis_sec],
                          chart->xmax[ser->x_axis_sec], 0, w);
            p2.x += x_ofs;
            p2.y = lv_map(points[i + 1].y, chart->ymin[ser->y_axis_sec],
                          chart->ymax[ser->y_axis_sec], h, 0);
            p2.y += y_ofs;

            if ((points[i].x == points[i + 1].x)
                && LV_ABS(points[i].y - points[i + 1].y) <= ser->threshold) {
                part_draw_dsc.p2 = &p2;
                lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
                lv_draw_line(draw_ctx, &line_dsc_default, &p1, &p2);
                lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
            }
        }
        lv_mem_buf_release(points);
    }
}
#endif

static void draw_series_bar(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx)
{
    lv_area_t clip_area;
    if (!_lv_area_intersect(&clip_area, &obj->coords, draw_ctx->clip_area))
        return;

    const lv_area_t* clip_area_ori = draw_ctx->clip_area;
    draw_ctx->clip_area = &clip_area;
    lvx_chart_t* chart = (lvx_chart_t*)obj;

    uint16_t i;
    lv_area_t col_a;
    lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    lv_coord_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    lv_coord_t h
        = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
    int32_t y_tmp;
    lvx_chart_series_t* ser;
    uint32_t ser_cnt = _lv_ll_get_len(&chart->series_ll);
    int32_t block_gap = ((int32_t)lv_obj_get_style_pad_column(obj, LV_PART_MAIN)
                         * chart->zoom_x)
        >> 8; /*Gap between the column on ~adjacent X*/
    lv_coord_t block_w
        = (w - ((chart->point_cnt - 1) * block_gap)) / chart->point_cnt;
    lv_coord_t col_w = block_w / ser_cnt;
    int32_t ser_gap = ((int32_t)lv_obj_get_style_pad_column(obj, LV_PART_ITEMS)
                       * chart->zoom_x)
        >> 8; /*Gap between the column on the ~same X*/

#if LVX_CHART_EXTENTIONS == 1
    /* Single column does not need this interval */
    if (ser_cnt <= 1) {
        ser_gap = 0;
    }
#endif

    lv_coord_t x_ofs = pad_left - lv_obj_get_scroll_left(obj);
    lv_coord_t y_ofs = pad_top - lv_obj_get_scroll_top(obj);

    lv_draw_rect_dsc_t col_dsc;
    lv_draw_rect_dsc_init(&col_dsc);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_ITEMS, &col_dsc);
    col_dsc.bg_grad.dir = LV_GRAD_DIR_NONE;
    col_dsc.bg_opa = LV_OPA_COVER;

    /*Make the cols longer with `radius` to clip the rounding from the bottom*/
    col_a.y2 = obj->coords.y2 + col_dsc.radius;

#if LVX_CHART_EXTENTIONS == 1
    /* need grad */
    lv_obj_init_draw_rect_dsc(obj, LV_PART_ITEMS, &col_dsc);

    /* bar bg */
    lv_area_t col_a_bg;
    lv_draw_rect_dsc_t col_dsc_bg;
    lv_draw_rect_dsc_init(&col_dsc_bg);
    lv_obj_init_draw_rect_dsc(obj, LVX_PART_ITEMS_BG, &col_dsc_bg);

    col_a.y2 -= col_dsc.radius;
    col_a_bg.y2 = col_a.y2;
    if (col_dsc_bg.radius != 0) {
        col_dsc_bg.radius = col_w;
        col_dsc.radius = col_w;
    } else {
        col_dsc.radius = 0;
    }
#endif

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.part = LV_PART_ITEMS;
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LVX_CHART_DRAW_PART_BAR;

    /*Go through all points*/
    for (i = 0; i < chart->point_cnt; i++) {
        lv_coord_t x_act
            = (int32_t)((int32_t)(w + block_gap) * i) / (chart->point_cnt)
            + obj->coords.x1 + x_ofs;

        part_draw_dsc.id = i;

        /*Draw the current point of all data line*/
        _LV_LL_READ_BACK(&chart->series_ll, ser)
        {
            if (ser->hidden)
                continue;
            lv_coord_t start_point
                = chart->update_mode == LVX_CHART_UPDATE_MODE_SHIFT
                ? ser->start_point
                : 0;

            col_a.x1 = x_act;
            col_a.x2 = col_a.x1 + col_w - ser_gap - 1;
            x_act += col_w;

            if (col_a.x2 < clip_area.x1)
                continue;
            if (col_a.x1 > clip_area.x2)
                break;

            col_dsc.bg_color = ser->color;

            lv_coord_t p_act = (start_point + i) % chart->point_cnt;
            y_tmp = (int32_t)((int32_t)ser->y_points[p_act]
                              - chart->ymin[ser->y_axis_sec])
                * h;
            y_tmp = y_tmp
                / (chart->ymax[ser->y_axis_sec] - chart->ymin[ser->y_axis_sec]);
            col_a.y1 = h - y_tmp + obj->coords.y1 + y_ofs;

#if LVX_CHART_EXTENTIONS == 1
            /* draw column bg */
            col_a_bg.x1 = col_a.x1;
            col_a_bg.x2 = col_a.x2;
            col_a_bg.y1 = obj->coords.y1 + y_ofs;

            part_draw_dsc.draw_area = &col_a_bg;
            part_draw_dsc.rect_dsc = &col_dsc_bg;
            part_draw_dsc.sub_part_ptr = ser;
            part_draw_dsc.value = ser->y_points[p_act];
            part_draw_dsc.part = LVX_PART_ITEMS_BG;
            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
            lv_draw_rect(draw_ctx, &col_dsc_bg, &col_a_bg);
            lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
            part_draw_dsc.part = LV_PART_ITEMS;
#endif
            if (ser->y_points[p_act] != LVX_CHART_POINT_NONE) {
#if LVX_CHART_EXTENTIONS == 1
                /* don't draw */
                if ((int32_t)ser->y_points[p_act] - chart->ymin[ser->y_axis_sec]
                    == 0) {
                    continue;
                }
#endif
                part_draw_dsc.draw_area = &col_a;
                part_draw_dsc.rect_dsc = &col_dsc;
                part_draw_dsc.sub_part_ptr = ser;
                part_draw_dsc.value = ser->y_points[p_act];
                lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
                lv_draw_rect(draw_ctx, &col_dsc, &col_a);
                lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
            }
        }
    }
    draw_ctx->clip_area = clip_area_ori;
}

static void draw_cursors(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (_lv_ll_is_empty(&chart->cursor_ll))
        return;

    lv_point_t p1;
    lv_point_t p2;
    lvx_chart_cursor_t* cursor;

    lv_draw_line_dsc_t line_dsc_ori;
    lv_draw_line_dsc_init(&line_dsc_ori);
    lv_obj_init_draw_line_dsc(obj, LV_PART_CURSOR, &line_dsc_ori);

    lv_draw_rect_dsc_t point_dsc_ori;
    lv_draw_rect_dsc_init(&point_dsc_ori);
    point_dsc_ori.bg_opa = line_dsc_ori.opa;
    point_dsc_ori.radius = LV_RADIUS_CIRCLE;

    lv_draw_line_dsc_t line_dsc_tmp;
    lv_draw_rect_dsc_t point_dsc_tmp;

    lv_coord_t point_w = lv_obj_get_style_width(obj, LV_PART_CURSOR) / 2;
    lv_coord_t point_h = lv_obj_get_style_width(obj, LV_PART_CURSOR) / 2;

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.line_dsc = &line_dsc_tmp;
    part_draw_dsc.rect_dsc = &point_dsc_tmp;
    part_draw_dsc.part = LV_PART_CURSOR;
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LVX_CHART_DRAW_PART_CURSOR;

    lv_area_t clip_area2;
    _lv_area_intersect(&clip_area2, draw_ctx->clip_area, &obj->coords);

    /*Go through all cursor lines*/
    _LV_LL_READ_BACK(&chart->cursor_ll, cursor)
    {
        lv_memcpy(&line_dsc_tmp, &line_dsc_ori, sizeof(lv_draw_line_dsc_t));
        lv_memcpy(&point_dsc_tmp, &point_dsc_ori, sizeof(lv_draw_rect_dsc_t));
        line_dsc_tmp.color = cursor->color;
        point_dsc_tmp.bg_color = cursor->color;

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        lv_coord_t cx;
        lv_coord_t cy;
        if (cursor->pos_set) {
            cx = cursor->pos.x;
            cy = cursor->pos.y;
        } else {
            if (cursor->point_id == LVX_CHART_POINT_NONE)
                continue;
            lv_point_t p;
            lvx_chart_get_point_pos_by_id(obj, cursor->ser, cursor->point_id,
                                          &p);
            cx = p.x;
            cy = p.y;
        }

        cx += obj->coords.x1;
        cy += obj->coords.y1;

        lv_area_t point_area;
        if (point_w && point_h) {
            point_area.x1 = cx - point_w;
            point_area.x2 = cx + point_w;
            point_area.y1 = cy - point_h;
            point_area.y2 = cy + point_h;

            part_draw_dsc.draw_area = &point_area;
        } else {
            part_draw_dsc.draw_area = NULL;
        }

        if (cursor->dir & LV_DIR_HOR) {
            p1.x = cursor->dir & LV_DIR_LEFT ? obj->coords.x1 : cx;
            p1.y = cy;
            p2.x = cursor->dir & LV_DIR_RIGHT ? obj->coords.x2 : cx;
            p2.y = p1.y;

            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
            lv_draw_line(draw_ctx, &line_dsc_tmp, &p1, &p2);
            lv_draw_rect(draw_ctx, &point_dsc_tmp, &point_area);
            lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
        }

        if (cursor->dir & LV_DIR_VER) {
            p1.x = cx;
            p1.y = cursor->dir & LV_DIR_TOP ? obj->coords.y1 : cy;
            p2.x = p1.x;
            p2.y = cursor->dir & LV_DIR_BOTTOM ? obj->coords.y2 : cy;

            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
            const lv_area_t* area_ori = draw_ctx->clip_area;
            draw_ctx->clip_area = &clip_area2;
            lv_draw_line(draw_ctx, &line_dsc_tmp, &p1, &p2);
            lv_draw_rect(draw_ctx, &point_dsc_tmp, &point_area);
            draw_ctx->clip_area = area_ori;
            lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
        }
    }
}

static void draw_y_ticks(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx,
                         lvx_chart_axis_t axis)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;

    lvx_chart_tick_dsc_t* t = get_tick_gsc(obj, axis);

    if (t->major_cnt <= 1)
        return;
    if (!t->label_en && !t->major_len && !t->minor_len)
        return;

    uint8_t sec_axis = axis == LVX_CHART_AXIS_PRIMARY_Y ? 0 : 1;

    uint32_t i;

    lv_point_t p1;
    lv_point_t p2;

    lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_coord_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    lv_coord_t h
        = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
    lv_coord_t y_ofs
        = obj->coords.y1 + pad_top + border_width - lv_obj_get_scroll_top(obj);

    lv_coord_t label_gap;
    lv_coord_t x_ofs;
    if (axis == LVX_CHART_AXIS_PRIMARY_Y) {
        label_gap = lv_obj_get_style_pad_left(obj, LV_PART_TICKS);
        x_ofs = obj->coords.x1;
    } else {
        label_gap = lv_obj_get_style_pad_right(obj, LV_PART_TICKS);
        x_ofs = obj->coords.x2;
    }

    lv_coord_t major_len = t->major_len;
    lv_coord_t minor_len = t->minor_len;
    /*tick lines on secondary y axis are drawn in other direction*/
    if (axis == LVX_CHART_AXIS_SECONDARY_Y) {
        major_len *= -1;
        minor_len *= -1;
    }

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_TICKS, &line_dsc);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    lv_obj_init_draw_label_dsc(obj, LV_PART_TICKS, &label_dsc);

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LVX_CHART_DRAW_PART_TICK_LABEL;
    part_draw_dsc.id = axis;
    part_draw_dsc.part = LV_PART_TICKS;
    part_draw_dsc.line_dsc = &line_dsc;
    part_draw_dsc.label_dsc = &label_dsc;

    uint32_t total_tick_num = (t->major_cnt - 1) * (t->minor_cnt);
    for (i = 0; i <= total_tick_num; i++) {
        /*draw a line at moving y position*/
        p2.y = p1.y = y_ofs
            + (int32_t)((int32_t)(h - line_dsc.width) * i) / total_tick_num;

        /*first point of the tick*/
        p1.x = x_ofs;

        /*move extra pixel out of chart boundary*/
        if (axis == LVX_CHART_AXIS_PRIMARY_Y)
            p1.x--;
        else
            p1.x++;

        /*second point of the tick*/
        bool major = false;
        if (i % t->minor_cnt == 0)
            major = true;

        if (major)
            p2.x = p1.x - major_len; /*major tick*/
        else
            p2.x = p1.x - minor_len; /*minor tick*/

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        int32_t tick_value
            = lv_map(total_tick_num - i, 0, total_tick_num,
                     chart->ymin[sec_axis], chart->ymax[sec_axis]);
        part_draw_dsc.value = tick_value;

        /*add text only to major tick*/
        if (major && t->label_en) {
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d", tick_value);
            part_draw_dsc.label_dsc = &label_dsc;
            part_draw_dsc.text = buf;
            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);

            /*reserve appropriate area*/
            lv_point_t size;
            lv_txt_get_size(&size, part_draw_dsc.text, label_dsc.font,
                            label_dsc.letter_space, label_dsc.line_space,
                            LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            /*set the area at some distance of the major tick len left of the
             * tick*/
            lv_area_t a;
            a.y1 = p2.y - size.y / 2;
            a.y2 = p2.y + size.y / 2;

            if (!sec_axis) {
                a.x1 = p2.x - size.x - label_gap;
                a.x2 = p2.x - label_gap;
            } else {
                a.x1 = p2.x + label_gap;
                a.x2 = p2.x + size.x + label_gap;
            }

            if (a.y2 >= obj->coords.y1 && a.y1 <= obj->coords.y2) {
                lv_draw_label(draw_ctx, &label_dsc, &a, part_draw_dsc.text,
                              NULL);
            }
        } else {
            part_draw_dsc.label_dsc = NULL;
            part_draw_dsc.text = NULL;
            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        }

        if (p1.y + line_dsc.width / 2 >= obj->coords.y1
            && p2.y - line_dsc.width / 2 <= obj->coords.y2) {
            lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
        }

        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}

static void draw_x_ticks(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx,
                         lvx_chart_axis_t axis)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;

    lvx_chart_tick_dsc_t* t = get_tick_gsc(obj, axis);
    if (t->major_cnt <= 1)
        return;
    if (!t->label_en && !t->major_len && !t->minor_len)
        return;

    uint32_t i;
    lv_point_t p1;
    lv_point_t p2;

    lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN)
        + lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    lv_obj_init_draw_label_dsc(obj, LV_PART_TICKS, &label_dsc);

    lv_coord_t x_ofs = obj->coords.x1 + pad_left - lv_obj_get_scroll_left(obj);
    lv_coord_t y_ofs;
    lv_coord_t label_gap;
    if (axis == LVX_CHART_AXIS_PRIMARY_X) {
        label_gap
            = t->label_en ? lv_obj_get_style_pad_bottom(obj, LV_PART_TICKS) : 0;
        y_ofs = obj->coords.y2;
    } else {
        label_gap
            = t->label_en ? lv_obj_get_style_pad_top(obj, LV_PART_TICKS) : 0;
        y_ofs = obj->coords.y1;
    }

    if (axis == LVX_CHART_AXIS_PRIMARY_X) {
        if (y_ofs > draw_ctx->clip_area->y2)
            return;
        if (y_ofs + label_gap + label_dsc.font->line_height + t->major_len
            < draw_ctx->clip_area->y1)
            return;
    }

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_TICKS, &line_dsc);
    line_dsc.dash_gap = 0;
    line_dsc.dash_width = 0;

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LVX_CHART_DRAW_PART_TICK_LABEL;
    part_draw_dsc.id = LVX_CHART_AXIS_PRIMARY_X;
    part_draw_dsc.part = LV_PART_TICKS;
    part_draw_dsc.label_dsc = &label_dsc;
    part_draw_dsc.line_dsc = &line_dsc;

    uint8_t sec_axis = axis == LVX_CHART_AXIS_PRIMARY_X ? 0 : 1;

    /*The columns ticks should be aligned to the center of blocks*/
    if (chart->type == LVX_CHART_TYPE_BAR) {
        int32_t block_gap
            = ((int32_t)lv_obj_get_style_pad_column(obj, LV_PART_MAIN)
               * chart->zoom_x)
            >> 8; /*Gap between the columns on ~adjacent X*/
        lv_coord_t block_w = (w + block_gap) / (chart->point_cnt);
        x_ofs += (block_w - block_gap) / 2;
        w -= block_w - block_gap;
    }

    p1.y = y_ofs;
    uint32_t total_tick_num = (t->major_cnt - 1) * t->minor_cnt;
    for (i = 0; i <= total_tick_num;
         i++) { /*one extra loop - it may not exist in the list, empty label*/
        bool major = false;
        if (i % t->minor_cnt == 0)
            major = true;

        /*draw a line at moving x position*/
        p2.x = p1.x = x_ofs
            + (int32_t)((int32_t)(w - line_dsc.width) * i) / total_tick_num;

        if (sec_axis)
            p2.y = p1.y - (major ? t->major_len : t->minor_len);
        else
            p2.y = p1.y + (major ? t->major_len : t->minor_len);

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        /*add text only to major tick*/
        int32_t tick_value;
        if (chart->type == LVX_CHART_TYPE_SCATTER
            || chart->type == LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
            /* Use the best value to determine the value of tick */
            || 1
#endif
        ) {
            tick_value = lv_map(i, 0, total_tick_num, chart->xmin[sec_axis],
                                chart->xmax[sec_axis]);
        } else {
            tick_value = i / t->minor_cnt;
        }
        part_draw_dsc.value = tick_value;

        if (major && t->label_en) {
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d", tick_value);
            part_draw_dsc.label_dsc = &label_dsc;
            part_draw_dsc.text = buf;
            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);

            /*reserve appropriate area*/
            lv_point_t size;
            lv_txt_get_size(&size, part_draw_dsc.text, label_dsc.font,
                            label_dsc.letter_space, label_dsc.line_space,
                            LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            /*set the area at some distance of the major tick len under of the
             * tick*/
            lv_area_t a;
            a.x1 = (p2.x - size.x / 2);
            a.x2 = (p2.x + size.x / 2);
            if (sec_axis) {
                a.y2 = p2.y - label_gap;
                a.y1 = a.y2 - size.y;
            } else {
                a.y1 = p2.y + label_gap;
                a.y2 = a.y1 + size.y;
            }

#if LVX_CHART_EXTENTIONS == 1
            if (a.x1 < obj->coords.x1) {
                a.x2 += obj->coords.x1 - a.x1;
                a.x1 = obj->coords.x1;
            }

            if (a.x2 > obj->coords.x2) {
                a.x1 -= a.x2 - obj->coords.x2;
                a.x2 = obj->coords.x2;
            }
#endif
            if (a.x2 >= obj->coords.x1 && a.x1 <= obj->coords.x2) {
                lv_draw_label(draw_ctx, &label_dsc, &a, part_draw_dsc.text,
                              NULL);
            }
        } else {
            part_draw_dsc.label_dsc = NULL;
            part_draw_dsc.text = NULL;
            lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        }

        if (p1.x + line_dsc.width / 2 >= obj->coords.x1
            && p2.x - line_dsc.width / 2 <= obj->coords.x2) {
            lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
        }

        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}

static void draw_axes(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx)
{
    draw_y_ticks(obj, draw_ctx, LVX_CHART_AXIS_PRIMARY_Y);
    draw_y_ticks(obj, draw_ctx, LVX_CHART_AXIS_SECONDARY_Y);
    draw_x_ticks(obj, draw_ctx, LVX_CHART_AXIS_PRIMARY_X);
    draw_x_ticks(obj, draw_ctx, LVX_CHART_AXIS_SECONDARY_X);
}

/**
 * Get the nearest index to an X coordinate
 * @param chart pointer to a chart object
 * @param coord the coordination of the point relative to the series area.
 * @return the found index
 */
static uint32_t get_index_from_x(lv_obj_t* obj, lv_coord_t x)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    x -= pad_left;

    if (x < 0)
        return 0;
    if (x > w)
        return chart->point_cnt - 1;
    if (chart->type == LVX_CHART_TYPE_LINE)
        return (x * (chart->point_cnt - 1) + w / 2) / w;
    if (chart->type == LVX_CHART_TYPE_BAR)
        return (x * chart->point_cnt) / w;

    return 0;
}

static void invalidate_point(lv_obj_t* obj, uint16_t i)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    if (i >= chart->point_cnt)
        return;

    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    lv_coord_t scroll_left = lv_obj_get_scroll_left(obj);

    /*In shift mode the whole chart changes so the whole object*/
    if (chart->update_mode == LVX_CHART_UPDATE_MODE_SHIFT) {
        lv_obj_invalidate(obj);
        return;
    }

    if (chart->type == LVX_CHART_TYPE_LINE) {
        lv_coord_t bwidth = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
        lv_coord_t pleft = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
        lv_coord_t x_ofs = obj->coords.x1 + pleft + bwidth - scroll_left;
        lv_coord_t line_width = lv_obj_get_style_line_width(obj, LV_PART_ITEMS);
        lv_coord_t point_w = lv_obj_get_style_width(obj, LV_PART_INDICATOR);

        lv_area_t coords;
        lv_area_copy(&coords, &obj->coords);
        coords.y1 -= line_width + point_w;
        coords.y2 += line_width + point_w;

        if (i < chart->point_cnt - 1) {
            coords.x1 = ((w * i) / (chart->point_cnt - 1)) + x_ofs - line_width
                - point_w;
            coords.x2 = ((w * (i + 1)) / (chart->point_cnt - 1)) + x_ofs
                + line_width + point_w;
            lv_obj_invalidate_area(obj, &coords);
        }

        if (i > 0) {
            coords.x1 = ((w * (i - 1)) / (chart->point_cnt - 1)) + x_ofs
                - line_width - point_w;
            coords.x2 = ((w * i) / (chart->point_cnt - 1)) + x_ofs + line_width
                + point_w;
            lv_obj_invalidate_area(obj, &coords);
        }
    } else if (chart->type == LVX_CHART_TYPE_BAR) {
        lv_area_t col_a;
        int32_t block_gap
            = ((int32_t)lv_obj_get_style_pad_column(obj, LV_PART_MAIN)
               * chart->zoom_x)
            >> 8; /*Gap between the column on ~adjacent X*/
        lv_coord_t block_w = (w + block_gap) / chart->point_cnt;

        lv_coord_t x_act;
        x_act = (int32_t)((int32_t)(block_w)*i);
        x_act += obj->coords.x1 + lv_obj_get_style_pad_left(obj, LV_PART_MAIN);

        lv_obj_get_coords(obj, &col_a);
        col_a.x1 = x_act - scroll_left;
        col_a.x2 = col_a.x1 + block_w;
        col_a.x1 -= block_gap;

        lv_obj_invalidate_area(obj, &col_a);
    } else if (chart->type == LVX_CHART_TYPE_SCATTER
               || chart->type == LVX_CHART_TYPE_STEP_LINE
#if LVX_CHART_EXTENTIONS == 1
               || chart->type == LVX_CHART_TYPE_POINT_BAR
#endif
    ) {
        lv_obj_invalidate(obj);
    } else {
        lv_obj_invalidate(obj);
    }
}

static void new_points_alloc(lv_obj_t* obj, lvx_chart_series_t* ser,
                             uint32_t cnt, lv_coord_t** a)
{
    if ((*a) == NULL)
        return;

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    uint32_t point_cnt_old = chart->point_cnt;
    uint32_t i;

    if (ser->start_point != 0) {
        lv_coord_t* new_points = lv_mem_alloc(sizeof(lv_coord_t) * cnt);
        LV_ASSERT_MALLOC(new_points);
        if (new_points == NULL)
            return;

        if (cnt >= point_cnt_old) {
            for (i = 0; i < point_cnt_old; i++) {
                new_points[cnt - point_cnt_old + i]
                    = (*a)[(i + ser->start_point) % point_cnt_old];
            }
            for (i = 0; i < cnt - point_cnt_old; i++) {
                new_points[i] = LVX_CHART_POINT_NONE;
            }
        } else {
            for (i = 0; i < cnt; i++) {
                new_points[i]
                    = (*a)[(i + ser->start_point)
                           % point_cnt_old]; /*Copy old contents to new array*/
            }
        }

        /*Switch over pointer from old to new*/
        lv_mem_free((*a));
        (*a) = new_points;
    } else {
        if (cnt <= point_cnt_old) {
            (*a) = lv_mem_realloc((*a), sizeof(lv_coord_t) * cnt);
            LV_ASSERT_MALLOC((*a));
            if ((*a) == NULL)
                return;
        } else {
            lv_coord_t* new_points = lv_mem_alloc(sizeof(lv_coord_t) * cnt);
            LV_ASSERT_MALLOC(new_points);

            for (i = 0; i < point_cnt_old; i++) {
                new_points[cnt - point_cnt_old + i]
                    = (*a)[(i + ser->start_point) % point_cnt_old];
            }
            for (i = 0; i < cnt - point_cnt_old; i++) {
                new_points[i] = LVX_CHART_POINT_NONE;
            }
            lv_mem_free((*a));
            (*a) = new_points;
        }
    }
}

static lvx_chart_tick_dsc_t* get_tick_gsc(lv_obj_t* obj, lvx_chart_axis_t axis)
{
    lvx_chart_t* chart = (lvx_chart_t*)obj;
    switch (axis) {
    case LVX_CHART_AXIS_PRIMARY_Y:
        return &chart->tick[0];
    case LVX_CHART_AXIS_PRIMARY_X:
        return &chart->tick[1];
    case LVX_CHART_AXIS_SECONDARY_Y:
        return &chart->tick[2];
    case LVX_CHART_AXIS_SECONDARY_X:
        return &chart->tick[3];
    default:
        return NULL;
    }
}

#if LVX_CHART_EXTENTIONS == 1

static int point_cmp(const void* var1, const void* var2)
{
    lv_point_t* left = (lv_point_t*)var1;
    lv_point_t* right = (lv_point_t*)var2;
    if (left->x > right->x) {
        return 1;
    } else if (left->x == right->x) {
        if (left->y > right->y) {
            return 1;
        } else if (left->y == right->y) {
            return 0;
        } else
            return -1;
    }
    return -1;
}

static void draw_line_chart_event_cb(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);

    /*Add the faded area before the lines are drawn*/
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);

    if (dsc && dsc->part == LV_PART_ITEMS) {
        if (!dsc->p1 || !dsc->p2)
            return;
        lv_draw_ctx_t* draw_ctx = dsc->draw_ctx;
#ifdef CONFIG_LV_GPU_DRAW_POLYGON
        if (lv_gpu_getmode() != LV_GPU_MODE_POWERSAVE) {
            lvx_chart_series_t* ser = (lvx_chart_series_t*)dsc->sub_part_ptr;
            if (ser != NULL) {
                lvx_chart_t* chart = (lvx_chart_t*)obj;
                lv_coord_t step = dsc->p2->x - dsc->p1->x;
                lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
                lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN) + border_width;
                lv_coord_t x_ofs = obj->coords.x1 + pad_left - lv_obj_get_scroll_left(obj);
                lv_coord_t clip_left_x = LV_MAX(0, draw_ctx->clip_area->x1 - x_ofs);
                lv_coord_t left_bound = dsc->id * step;
                lv_coord_t right_bound = left_bound + step;
                lv_coord_t w = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
                lv_coord_t h = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
                if (clip_left_x >= left_bound && clip_left_x < right_bound) {
                    /* This is the first point to draw the gradient. Do it! */
                    lv_coord_t last_x = draw_ctx->clip_area->x2 - x_ofs;
                    lv_coord_t last_id = LV_MIN(last_x / step + 1, chart->point_cnt - 1);
                    lv_coord_t point_cnt = last_id - dsc->id + 1;
                    lv_point_t *poly_points = lv_mem_alloc((point_cnt + 2) * sizeof(lv_point_t));
                    if (poly_points) {
                        lv_coord_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN) + border_width;
                        lv_coord_t y_ofs = obj->coords.y1 + pad_top - lv_obj_get_scroll_top(obj);
                        lv_coord_t y_range = chart->ymax[ser->y_axis_sec] - chart->ymin[ser->y_axis_sec];
                        lv_coord_t p_act = dsc->id;
                        lv_coord_t y_max = ser->y_points[p_act];

                        for (lv_coord_t i = 0; i < point_cnt; i++, p_act++) {
                            if (p_act == chart->point_cnt) {
                                p_act = 0;
                            }
                            if (ser->y_points[p_act] != LV_CHART_POINT_NONE) {
                                poly_points[i].x = (dsc->id + i) * w / (chart->point_cnt - 1) + x_ofs;
                                poly_points[i].y = h - (ser->y_points[p_act] - chart->ymin[ser->y_axis_sec]) * h / y_range + y_ofs;
                                y_max = LV_MAX(y_max, ser->y_points[p_act]);
                            }
                        }
                        poly_points[point_cnt].x = poly_points[point_cnt - 1].x;
                        poly_points[point_cnt].y = h + y_ofs;
                        poly_points[point_cnt + 1].x = poly_points[0].x;
                        poly_points[point_cnt + 1].y = h + y_ofs;

                        lv_draw_rect_dsc_t draw_dsc;
                        lv_draw_rect_dsc_init(&draw_dsc);
                        draw_dsc.bg_grad.dir = LV_GRAD_DIR_VER;
                        uint8_t color_frac = (y_max - chart->ymin[ser->y_axis_sec]) * 255 / h;
                        draw_dsc.bg_grad.stops[0].color = lv_color_mix(dsc->line_dsc->color, lv_color_black(), color_frac);
                        LV_COLOR_SET_A(draw_dsc.bg_grad.stops[0].color, color_frac);
                        draw_dsc.bg_grad.stops[1].color.full = 0x00;

                        lv_area_t obj_clip_area;
                        _lv_area_intersect(&obj_clip_area, draw_ctx->clip_area, &obj->coords);
                        const lv_area_t * clip_area_ori = draw_ctx->clip_area;
                        draw_ctx->clip_area = &obj_clip_area;
                        lv_draw_polygon(draw_ctx, &draw_dsc, poly_points, point_cnt + 2);
                        lv_mem_free(poly_points);
                        draw_ctx->clip_area = clip_area_ori;
                        return;
                    }
                    else {
                        LV_LOG_WARN("out of memory");
                    }
                } else {
                    return;
                }
            }
        }
#endif
        /*Add a line mask that keeps the area below the line*/
        // lv_coord_t top = lv_obj_get_style_pad_top(obj, 0);
        lv_coord_t bottom = lv_obj_get_style_pad_bottom(obj, 0);

        lv_draw_mask_line_param_t line_mask_param;
        lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y,
                                      dsc->p2->x, dsc->p2->y,
                                      LV_DRAW_MASK_LINE_SIDE_BOTTOM);
        int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

        /*Add a fade effect: transparent bottom covering top*/
        // lv_coord_t h = lv_obj_get_height(obj) - top - bottom;
        lv_draw_mask_fade_param_t fade_mask_param;
        lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER,
                               //    obj->coords.y1 + h / 8, LV_OPA_TRANSP,
                               obj->coords.y1, LV_OPA_TRANSP, obj->coords.y2);
        int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

        /*Draw a rectangle that will be affected by the mask*/
        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_color = dsc->line_dsc->color;

        lv_area_t a;
        a.x1 = dsc->p1->x;
        a.x2 = dsc->p2->x - 1;
        a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
        a.y2 = obj->coords.y2 - bottom;
        if (a.x1 == a.x2) {
            a.x2 = a.x1 + 1;
        }
        lv_draw_rect(draw_ctx, &draw_rect_dsc, &a);

        /*Remove the masks*/
        lv_draw_mask_remove_id(line_mask_id);
        lv_draw_mask_remove_id(fade_mask_id);
    }
}

static void draw_avg_line(lv_obj_t* obj, lv_draw_ctx_t* draw_ctx,
                          bool const_flag)
{
    /* Draw the average label in the expansion area, so you need to judge the
     * size of the expansion area */
    lv_coord_t ext_draw_size = _lv_obj_get_ext_draw_size(obj);
    lv_area_t com_area;
    if ((!_lv_area_intersect(&com_area, &obj->coords, draw_ctx->clip_area))
        && ext_draw_size == 0)
        return;

    /* Average value line description */
    lv_draw_line_dsc_t avg_line_dsc;
    lv_draw_line_dsc_init(&avg_line_dsc);
    lv_obj_init_draw_line_dsc(obj, LVX_PART_AVG_LINE, &avg_line_dsc);
    if (avg_line_dsc.opa < LV_OPA_MIN || avg_line_dsc.width <= 0) {
        return;
    }

    /* Average value label description */
    lv_draw_label_dsc_t avg_label_dsc;
    lv_draw_label_dsc_init(&avg_label_dsc);
    lv_obj_init_draw_label_dsc(obj, LVX_PART_AVG_LINE, &avg_label_dsc);

    lvx_chart_t* chart = (lvx_chart_t*)obj;
    lv_point_t p1;
    lv_point_t p2;
    lv_coord_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    lv_coord_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    lv_coord_t w
        = ((int32_t)lv_obj_get_content_width(obj) * chart->zoom_x) >> 8;
    lv_coord_t h
        = ((int32_t)lv_obj_get_content_height(obj) * chart->zoom_y) >> 8;
    lv_coord_t x_ofs = obj->coords.x1 + pad_left + border_width
        - lv_obj_get_scroll_left(obj);
    lv_coord_t y_ofs
        = obj->coords.y1 + pad_top + border_width - lv_obj_get_scroll_top(obj);

    lv_coord_t point_w = lv_obj_get_style_width(obj, LV_PART_INDICATOR) / 2;
    lv_coord_t point_h = lv_obj_get_style_height(obj, LV_PART_INDICATOR) / 2;

    /* The edge of the point in the middle of the line */
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &line_dsc);
    if (line_dsc.width > 0 && line_dsc.opa > LV_OPA_MIN) {
        if (chart->vdiv_cnt) {
            x_ofs += point_w;
            w -= (line_dsc.width + point_w * 2);
        }
        if (chart->hdiv_cnt) {
            y_ofs += point_h;
            h -= (line_dsc.width + point_w * 2);
        }
    }

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);

    lvx_chart_series_t* ser;
    int32_t sum = 0;
    int32_t avg = 0;
    uint32_t valid_point_cnt = 0;

    /*Go through all data lines*/
    _LV_LL_READ_BACK(&chart->series_ll, ser)
    {
        if (ser->hidden)
            continue;

        for (lv_coord_t i = 0; i < chart->point_cnt; i++) {
            if (ser->y_points[i] != LVX_CHART_POINT_NONE) {
                sum += ser->y_points[i];
                valid_point_cnt++;
            }
        }

        if (valid_point_cnt == 0)
            continue;
        if (const_flag == false) {
            avg = sum / valid_point_cnt;
        } else {
            avg = chart->const_value;
        }

        p1.x = x_ofs;
        p2.x = p1.x + w;
        p1.y = lv_map(avg, chart->ymin[ser->y_axis_sec],
                      chart->ymax[ser->y_axis_sec], 0, h);

        p1.y = h - p1.y;
        p1.y += y_ofs;
        p2.y = p1.y;

        part_draw_dsc.part = LVX_PART_AVG_LINE;
        part_draw_dsc.class_p = MY_CLASS;
        part_draw_dsc.type = LVX_CHART_DRAW_PART_LINE_AND_POINT;
        part_draw_dsc.line_dsc = &avg_line_dsc;
        part_draw_dsc.rect_dsc = NULL;
        part_draw_dsc.label_dsc = &avg_label_dsc;
        part_draw_dsc.sub_part_ptr = ser;

        part_draw_dsc.id = 0;
        part_draw_dsc.draw_area = &com_area;
        part_draw_dsc.value = avg;

        /* avg label */
        lv_area_t a = { 0 };

        lvx_chart_axis_t axis = ser->y_axis_sec == 0
            ? LVX_CHART_AXIS_PRIMARY_Y
            : LVX_CHART_AXIS_SECONDARY_Y;
        lvx_chart_tick_dsc_t* tick = get_tick_gsc(obj, axis);
        lv_coord_t major_len = 0;
        if (tick) {
            major_len = tick->major_len;
        }

        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%d", avg);
        lv_point_t size;
        lv_txt_get_size(&size, buf, avg_label_dsc.font,
                        avg_label_dsc.letter_space, avg_label_dsc.line_space,
                        LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (ser->y_axis_sec == 0) {
            a.x1 = obj->coords.x1;
            a.x1 -= lv_obj_get_style_pad_right(obj, LV_PART_TICKS);
            a.x1 -= 1;
            a.x1 -= major_len;
            a.x1 -= size.x;

            /* Adjust the position of the mean line */

            p1.x -= line_dsc.width + 1 + point_w * 2;
            p2.x += point_w * 2;

        } else {
            a.x1 = obj->coords.x2;
            a.x1 += lv_obj_get_style_pad_left(obj, LV_PART_TICKS);
            a.x1 += 1;
            a.x1 += major_len;

            /* Adjust the position of the mean line */
            p1.x -= point_w + (line_dsc.width >> 1);
            p2.x += line_dsc.width + 1 + point_w * 2;
        }

        a.x2 = a.x1 + size.x;
        a.y1 = p2.y - size.y / 2;
        a.y2 = p2.y + size.y / 2;

        part_draw_dsc.p1 = &p1;
        part_draw_dsc.p2 = &p2;

        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        const lv_area_t* area_ori = draw_ctx->clip_area;
        draw_ctx->clip_area = &com_area;
        lv_draw_line(draw_ctx, &avg_line_dsc, &p1, &p2);
        draw_ctx->clip_area = area_ori;
        lv_draw_label(draw_ctx, &avg_label_dsc, &a, buf, NULL);
        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }
}

#endif /* LVX_CHART_EXTENTIONS == 1 */

#endif /* LVX_USE_CHART != 0 */
