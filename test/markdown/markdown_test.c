/**
 * @file markdown_test.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "uikit/uikit.h"

#if CONFIG_UIKIT_MARKDOWN

#include "markdown_test.h"
#include <cmark-gfm.h>
#include <latexmath.h>
#include <lvgl/src/lvgl_private.h>
#include <table.h>

static lv_font_t* fonts[6] = { 0 };

static const char* markdown_txt = "### GTest介绍"
                                  "\n"
                                  "Gtest是Google的一个开源框架，它主要用于**写单元测试**，~~检查真自己~~的程序是否符合预期行为。"
                                  "可在多个平台上使用(包括Linux, MacOSX, Windows, Cygwin)。它提供了丰富的断言、致命和非致命失败判断，能进行值*参数化测试*、类型参数化测试、“死亡测试”。"
                                  "\n"
                                  "#### Github地址"
                                  "\n"
                                  "___"
                                  "\n"
                                  "[GoogleTest - Google Testing and Mocking Framework](https://github.com/google/googletest)"
                                  "\n"
                                  "#### 编译安装"
                                  "\n"
                                  "* ubuntu 20.4 环境"
                                  "```"
                                  "shell:~$ sudo apt-get install libgtest-dev"
                                  "```"
                                  "\n"
                                  "* 从源代码安装"
                                  "\n\n"
                                  ">注释代码"
                                  "\n"
                                  "\n"
                                  "   |Header| \n"
                                  "   |------| \n"
                                  "   |Hello |"
                                  "\n"
                                  "\n"
                                  "$a+b-c$"
                                  "\n"
                                  "\n"
                                  " $leq$, $geq$  $arcsin x, arccos x, arctan x, arccot x$  $iint, iint_D$  $alpha, beta, gamma$  "
                                  "\n"
                                  "1. First item \n"
                                  "2. Second item \n"
                                  "    1. Nested 1\n"
                                  "    2. Nested 2\n"
                                  "***this ***is ***a ***bunch*** of*** nested*** emphases***\n";

static void markdown_heading_style_cb(lv_style_t* style, int32_t level);
static void markdown_unsupported_cb(char* buff, uint32_t size, lv_style_t* style, int32_t type);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void timer_cb(lv_timer_t* timer)
{
    lv_obj_t* obj = lv_timer_get_user_data(timer);

    static uint32_t ofs = 0;
    const uint32_t str_len = strlen(markdown_txt);

    uint32_t count = lv_rand(2, 10);
    while (count-- > 0) {
        if (lv_text_encoded_next(markdown_txt, &ofs) == 0) {
            lv_timer_delete(timer);
            break;
        }
    }

    ofs = (ofs > str_len) ? str_len : ofs;

    vg_markdown_set_data(obj, markdown_txt, ofs);
    const int32_t bottom = lv_obj_get_scroll_bottom(obj);
    if (bottom > 0) {
        lv_obj_scroll_by(obj, 0, -bottom, LV_ANIM_OFF);
    }
}

// MiSans-Regular.ttf
void uikit_demo_markdown(char* info[], int size, void* param)
{
    fonts[0] = vg_font_create("MiSans-Regular", 48, LV_FREETYPE_FONT_STYLE_NORMAL);
    fonts[1] = vg_font_create("MiSans-Regular", 32, LV_FREETYPE_FONT_STYLE_NORMAL);
    fonts[2] = vg_font_create("MiSans-Regular", 24, LV_FREETYPE_FONT_STYLE_NORMAL);
    fonts[3] = vg_font_create("MiSans-Regular", 22, LV_FREETYPE_FONT_STYLE_NORMAL);
    fonts[4] = vg_font_create("MiSans-Regular", 18, LV_FREETYPE_FONT_STYLE_NORMAL);
    fonts[5] = vg_font_create("MiSans-Regular", 16, LV_FREETYPE_FONT_STYLE_NORMAL);

    lv_obj_t* markdown_widget = vg_markdown_create(lv_screen_active());
    vg_markdown_set_heading_style_cb(markdown_widget, markdown_heading_style_cb);
    vg_markdown_set_unsupported_cb(markdown_widget, markdown_unsupported_cb);

    int32_t width = lv_display_get_horizontal_resolution(NULL);
    int32_t height = lv_display_get_horizontal_resolution(NULL);
    lv_obj_set_size(markdown_widget, width, height);

    lv_timer_create(timer_cb, 100, markdown_widget);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void markdown_heading_style_cb(lv_style_t* style, int32_t level)
{
    const int32_t font_cnt = sizeof(fonts) / sizeof(fonts[0]);
    if (0 < level && level <= font_cnt) {
        if (fonts[level - 1])
            lv_style_set_text_font(style, fonts[level - 1]);
        else
            lv_style_set_text_font(style, lv_font_default());
    } else {
        lv_style_set_text_font(style, fonts[font_cnt - 1]);
    }
}

static void markdown_unsupported_cb(char* buff, uint32_t size, lv_style_t* style, int32_t type)
{
    const char* type_str;
    if (type == CMARK_NODE_IMAGE) {
        type_str = "图片";
    } else if (type == CMARK_NODE_CODE) {
        type_str = "代码";
    } else if (type == CMARK_NODE_CODE_BLOCK) {
        type_str = "代码块";
    } else if (type == CMARK_NODE_HTML_BLOCK) {
        type_str = "超文本语言";
    } else if (type == CMARK_NODE_HTML_INLINE) {
        type_str = "超文本语言块";
    } else if (type == CMARK_NODE_CUSTOM_BLOCK) {
        type_str = "块";
    } else if (type == CMARK_NODE_BLOCK_QUOTE) {
        type_str = "引用块";
    } else if (type == CMARK_NODE_TABLE) {
        type_str = "表格";
    } else if (type == CMARK_NODE_LATEX_MATH) {
        type_str = "公式";
    } else {
        type_str = "块";
    }
    lv_snprintf(buff, size, "%s不支持", type_str);
    lv_style_set_text_color(style, lv_palette_main(LV_PALETTE_GREY));
}

#endif
