/**
 * @file lv_create_obj.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_create_obj.h"
#include "lvx_font_manager.h"

/*********************
 *      DEFINES
 *********************/
#define TEST_CASE_FONTS_COUNT (sizeof(test_case_fonts) / sizeof(test_case_fonts[0]))

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* name;
    lv_obj_t* (*create_func)(lv_obj_t*);
} Test_Case;

typedef struct {
    const char* name;
    const char* text;
    uint16_t size;
    uint16_t style;

} Test_Case_Font;

/**********************
 *  STATIC VARIABLES
 **********************/
static const Test_Case_Font test_case_fonts[] = {
    { "MiSans-Demibold", "音乐", 30, LV_FREETYPE_FONT_STYLE_BOLD },
    { "MiSans-Demibold", "每日推荐", 28, LV_FREETYPE_FONT_STYLE_BOLD },
    { "MiSans-Demibold", "红心歌单", 28, LV_FREETYPE_FONT_STYLE_BOLD },
    { "MiSans-Demibold", "最近播放", 28, LV_FREETYPE_FONT_STYLE_BOLD },
    { "MiSans-Demibold", "收藏歌单", 28, LV_FREETYPE_FONT_STYLE_BOLD },
    { "MiSans-Regular", "今日推荐歌曲", 18, LV_FREETYPE_FONT_STYLE_NORMAL },
    { "MiSans-Regular", "BFRND音乐特辑", 18, LV_FREETYPE_FONT_STYLE_NORMAL },
    { "MiSans-Regular", "躁就完了!抖音最热DJ...", 18, LV_FREETYPE_FONT_STYLE_NORMAL },
    { "MiSans-Regular", "Easy Run·轻松节奏助...", 18, LV_FREETYPE_FONT_STYLE_NORMAL },
    { "MiSans-Regular", "那些年—属于周杰伦的R&B时代", 18, LV_FREETYPE_FONT_STYLE_NORMAL },
    { "MiSans-Regular", "那些年让人怀念的经典粤语歌", 18, LV_FREETYPE_FONT_STYLE_NORMAL },

};

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline void get_used_time(lv_obj_t* parent, const char* obj, lv_obj_t* (*create_func)(lv_obj_t*))
{
    uint32_t tv_start, tv_used;

    lv_obj_t* obj_parent = lv_obj_create(parent);

    tv_start = lv_tick_get();
    for (int i = 0; i < 1000; i++) {
        create_func(obj_parent);
    }
    tv_used = lv_tick_elaps(tv_start);

    lv_obj_delete(obj_parent);

    LV_LOG("%s create_time: %.4f ms\n", obj, tv_used / 1000.0);
}

static inline void get_used_time_render_label(const int num, lv_obj_t* parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    LV_PROFILER_BEGIN;
    for (int i = 0; i < num; i++) {
        const lv_font_t* font = lvx_font_create(test_case_fonts[i].name, test_case_fonts[i].size, test_case_fonts[i].style);

        lv_obj_t* obj_parent = lv_obj_create(parent);
        lv_obj_set_size(obj_parent, 300, 80);
        lv_obj_clear_flag(obj_parent, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);

        lv_obj_t* label = lv_label_create(obj_parent);
        lv_label_set_text(label, test_case_fonts[i].text);
        lv_obj_set_style_text_font(label, font, 0);
    }
    LV_PROFILER_END;
}

static inline void get_used_time_render_spangroup(const int num, lv_obj_t* parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    LV_PROFILER_BEGIN;
    for (int i = 0; i < num; i++) {
        const lv_font_t* font = lvx_font_create(test_case_fonts[i].name, test_case_fonts[i].size, test_case_fonts[i].style);

        lv_obj_t* obj_parent = lv_obj_create(parent);
        lv_obj_set_size(obj_parent, 300, 80);
        lv_obj_clear_flag(obj_parent, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);

        lv_obj_t* spans = lv_spangroup_create(obj_parent);
        lv_span_t* span = lv_spangroup_new_span(spans);
        lv_span_set_text(span, test_case_fonts[i].text);
        lv_style_set_text_font(&span->style, font);
    }
    LV_PROFILER_END;
}

/****************************************************************************
 * lvx_time_obj_creation
 ****************************************************************************/

void lvx_time_obj_creation(char* info[], int size, void* param)
{

    LV_UNUSED(info);
    LV_UNUSED(size);
    LV_UNUSED(param);

    lv_obj_t* parent = lv_screen_active();

    Test_Case test_cases[] = {
        { "lv_obj_create", lv_obj_create },
        { "lv_image_create", lv_image_create },
        { "lv_spangroup_create", lv_spangroup_create },
        { "lv_slider_create", lv_slider_create },
        { "lv_chart_create", lv_chart_create },
        { "lv_switch_create", lv_switch_create },
        { "lv_qrcode_create", lv_qrcode_create },
    };

    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; i++) {
        get_used_time(parent, test_cases[i].name, test_cases[i].create_func);
    }
}

/****************************************************************************
 * lvx_time_render_label
 ****************************************************************************/
void lvx_time_render_label(char* info[], int size, void* param)
{

    LV_UNUSED(info);
    LV_UNUSED(size);
    LV_UNUSED(param);

    const int test_case_fonts_count = TEST_CASE_FONTS_COUNT;

    lv_obj_t* parent = lv_screen_active();

    get_used_time_render_label(test_case_fonts_count, parent);
}

/****************************************************************************
 * lvx_time_render_spandgroup
 ****************************************************************************/
void lvx_time_render_spangroup(char* info[], int size, void* param)
{

    LV_UNUSED(info);
    LV_UNUSED(size);
    LV_UNUSED(param);

    const int test_case_fonts_count = TEST_CASE_FONTS_COUNT;

    lv_obj_t* parent = lv_screen_active();

    get_used_time_render_spangroup(test_case_fonts_count, parent);
}