/**
 * @file markdown_test.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <nuttx/config.h>
#include "uikit/uikit.h"

#ifdef CONFIG_UIKIT_MARKDOWN

#include "markdown_test.h"

static const char * markdown_txt = \
            "### GTest介绍"
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
            "***this ***is ***a ***bunch*** of*** nested*** emphases***\n"

    ;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void uikit_markdown_demo(char * info[], int size, void * param)
{
    lv_obj_t * markdown_widget = uikit_markdown_create(lv_scr_act());
    lv_obj_set_size(markdown_widget, LV_PCT(100), LV_PCT(100));

    uikit_markdown_set_data(markdown_widget, markdown_txt, strlen(markdown_txt));
}

#endif
