/*********************
 *      INCLUDES
 *********************/
#include "lvx_picker.h"

#if LVX_USE_PICKER != 0

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_picker_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_picker_constructor(const lv_obj_class_t* class_p,
                                   lv_obj_t* obj);
static void lvx_picker_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_picker_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void refr_layout(lv_obj_t* obj);
static void set_rec_default_style(lv_obj_t* rec);
static void set_roller_local_style(lv_obj_t* roller);
static void refr_obj_text_styles(lv_obj_t* obj);
/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_picker_class = {
    .constructor_cb = lvx_picker_constructor,
    .destructor_cb = lvx_picker_destructor,
    .event_cb = lvx_picker_event,
    .instance_size = sizeof(lvx_picker_t),
    .base_class = &lv_obj_class,
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Return with a pointer to the picker
 * @param parent pointer to parent
 * @return pointer to the picker
 */
lv_obj_t* lvx_picker_create(lv_obj_t* parent)
{
    LVX_WIDGET_CREATE(MY_CLASS, parent);
}

/*=====================
 * Setter functions
 *====================*/

void lvx_picker_add_options(lv_obj_t* obj, const char* options,
                            lv_roller_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    AMCK(!options, LV_LOG_ERROR("options is NULL!\n"), , return );

    lvx_picker_t* picker = (lvx_picker_t*)obj;
    uint32_t size = sizeof(lv_obj_t*) * (picker->colume_cnt + 1);
    void* tmp = lv_mem_realloc(picker->colume_array, size);
    AMCK(!tmp, LV_LOG_ERROR("cannot malloc roller array!\n"), , return );
    picker->colume_array = tmp;

    lv_obj_t* colume = lv_roller_create(obj);
    set_roller_local_style(colume);
    lv_roller_set_options(colume, options, mode);

    picker->colume_array[picker->colume_cnt] = colume;
    picker->colume_cnt += 1;
}

void lvx_picker_set_options(lv_obj_t* obj, const char** options_array,
                            lv_coord_t col_cnt, lv_roller_mode_t mode)
{
    AMCK(!options_array, LV_LOG_ERROR("options is NULL!\n"), , return );
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    uint32_t size = sizeof(lv_obj_t*) * col_cnt;
    lv_obj_t** tmp = lv_mem_alloc(size);
    AMCK(!tmp, LV_LOG_ERROR("cannot malloc picker array!\n"), , return );

    lv_obj_t* column = NULL;
    if (picker->colume_array) {
        for (lv_coord_t i = 0; i < picker->colume_cnt; i++) {
            column = picker->colume_array[i];
            lv_obj_del(column);
        }
        lv_mem_free(picker->colume_array);
        picker->colume_cnt = 0;
    }

    for (lv_coord_t i = 0; i < col_cnt; i++) {
        column = lv_roller_create(obj);
        set_roller_local_style(column);
        lv_roller_set_options(column, options_array[i], mode);
        tmp[i] = column;
    }

    picker->colume_array = tmp;
    picker->colume_cnt = col_cnt;
}

void lvx_picker_set_column_visible_row_count(lv_obj_t* obj, uint8_t index,
                                             uint8_t row_cnt)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    AMCK(index >= picker->colume_cnt, LV_LOG_ERROR("index over range\n"), ,
         return );
    lv_obj_t* column = picker->colume_array[index];
    lv_roller_set_visible_row_count(column, row_cnt);
}

void lvx_picker_set_visible_row_count(lv_obj_t* obj, uint8_t row_cnt)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    lv_obj_t* column = NULL;
    for (lv_coord_t i = 0; i < picker->colume_cnt; i++) {
        column = picker->colume_array[i];
        lv_roller_set_visible_row_count(column, row_cnt);
    }
}

void lvx_picker_set_space_text(lv_obj_t* obj, const char* space_text)
{
    AMCK(!space_text, , , return );
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    AMCK(picker->colume_cnt < 2, , , return );
    text_styles_t main;
    lvx_obj_get_text_styles(obj, &main, LV_PART_SELECTED);
    /* create spacer */
    for (lv_coord_t i = 0; i < picker->colume_cnt - 1; i++) {
        lv_obj_t* label = lv_label_create(picker->rec);
        lvx_obj_set_text_styles(label, &main, LV_PART_MAIN);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(label, space_text);
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the selected strings of all rollers, separated by \n, the last one does
 * not
 * @param obj pointer to picker
 * @return pointer to string, if error return NULL
 */
uint32_t lvx_picker_get_select_str(lv_obj_t* obj, char* buf, uint32_t len)
{
    if (!obj || !buf) {
        LV_LOG_ERROR("error: !obj || !buf");
        return 0;
    }
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    memset(buf, 0, len);

    uint32_t total = 0;
    for (uint8_t i = 0; i < picker->colume_cnt; i++) {
        lv_roller_get_selected_str(picker->colume_array[i], buf + total,
                                   len - total);
        total = strlen(buf);
        strcat(buf, "\n");
        total += 1;
    }
    if (total != 0)
        buf[--total] = 0;
    return total;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_picker_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lvx_picker_t* picker = (lvx_picker_t*)obj;
    lv_obj_refr_size(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    picker->rec = lv_obj_create(obj);
    lv_obj_clear_flag(picker->rec, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(picker->rec, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(picker->rec, LV_OBJ_FLAG_IGNORE_LAYOUT);
    set_rec_default_style(picker->rec);
    lv_obj_set_size(picker->rec, lv_obj_get_content_width(obj),
                    lv_obj_get_content_height(obj) / 3);
    lv_obj_center(picker->rec);

    picker->colume_array = NULL;
    picker->colume_cnt = 0;

    LV_TRACE_OBJ_CREATE("finished");
}

static void lvx_picker_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    if (picker->colume_array) {
        lv_mem_free(picker->colume_array);
        picker->colume_array = NULL;
        picker->colume_cnt = 0;
    }
}

static void lvx_picker_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_current_target(e);
    if (code == LV_EVENT_SIZE_CHANGED || code == LV_EVENT_LAYOUT_CHANGED) {
        refr_layout(obj);
    } else if (code == LV_EVENT_STYLE_CHANGED) {
        refr_obj_text_styles(obj);
    }
}

static void refr_rec_size(lv_obj_t* obj)
{
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    if (picker->rec == NULL)
        return;
    lv_obj_set_width(picker->rec, lv_obj_get_content_width(obj));
    if (picker->colume_array == NULL || picker->colume_cnt < 1)
        return;
    const lv_font_t* font
        = lv_obj_get_style_text_font(picker->colume_array[0], LV_PART_MAIN);
    lv_coord_t line_space = lv_obj_get_style_text_line_space(
        picker->colume_array[0], LV_PART_MAIN);
    lv_coord_t font_h = lv_font_get_line_height(font);
    lv_obj_set_height(picker->rec, font_h + line_space - 1);
}

static void refr_self_size(lv_obj_t* obj)
{
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    if (picker->rec == NULL)
        return;
    if (picker->colume_array == NULL || picker->colume_cnt < 1)
        return;
    lv_obj_set_content_height(obj, lv_obj_get_height(picker->colume_array[0]));
}

static void refr_roller_pos(lv_obj_t* obj)
{
    lvx_picker_t* picker = (lvx_picker_t*)obj;

    if (picker->colume_array == NULL || picker->colume_cnt < 1)
        return;
    lv_obj_set_content_height(obj, lv_obj_get_height(picker->colume_array[0]));

    lv_coord_t gap = lv_obj_get_style_pad_row(obj, 0);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    lv_obj_init_draw_label_dsc(obj, LV_PART_SELECTED, &label_dsc);

    /* repos picker */
    for (lv_coord_t i = 0; i < picker->colume_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(picker->rec, i);
        if (child) {
            lv_point_t size;
            lv_txt_get_size(&size, lv_label_get_text(child), label_dsc.font,
                            label_dsc.letter_space, label_dsc.line_space,
                            LV_COORD_MAX, LV_TEXT_FLAG_NONE);
            lv_coord_t x_ofs = (gap - size.x) >> 1;
            lv_coord_t h = lv_obj_get_height(picker->rec);
            lv_obj_align_to(child, picker->colume_array[i],
                            LV_ALIGN_OUT_RIGHT_MID, x_ofs,
                            -(h - label_dsc.font->line_height) / 4);
        }
    }
}

static void refr_layout(lv_obj_t* obj)
{
    refr_self_size(obj);
    refr_rec_size(obj);
    refr_roller_pos(obj);
}

static void set_rec_default_style(lv_obj_t* rec)
{
    static lv_style_t style;
    static bool init_flag = false;

    if (!init_flag) {
        init_flag = true;
        lv_style_init(&style);
        lv_style_set_bg_color(&style, lv_color_white());
        lv_style_set_bg_opa(&style, LV_OPA_0);
        lv_style_set_border_width(&style, 0);
        lv_style_set_radius(&style, LV_RADIUS_CIRCLE);
    }
    lv_obj_add_style(rec, &style, 0);
}

static void set_roller_local_style(lv_obj_t* picker)
{
    lv_obj_set_style_bg_opa(picker, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_opa(picker, LV_OPA_TRANSP, LV_PART_SELECTED);

    lv_obj_set_style_border_width(picker, 0, 0);
    lv_obj_set_style_border_width(picker, 0, LV_PART_SELECTED);
}

static void refr_obj_text_styles(lv_obj_t* obj)
{
    lvx_picker_t* picker = (lvx_picker_t*)obj;
    text_styles_t main, sel;
    lvx_obj_get_text_styles(obj, &main, LV_PART_MAIN);
    lvx_obj_get_text_styles(obj, &sel, LV_PART_SELECTED);
    lv_obj_t* column = NULL;
    for (lv_coord_t i = 0; i < picker->colume_cnt; i++) {
        column = picker->colume_array[i];
        lvx_obj_set_text_styles(column, &main, LV_PART_MAIN);
        lvx_obj_set_text_styles(column, &sel, LV_PART_SELECTED);
    }
}

#endif
