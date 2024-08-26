/*
 * Copyright (C) 2024 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_markdown.h"

#ifdef CONFIG_LVX_USE_MARKDOWN

#include <cmark/cmark-gfm-extension_api.h>
#include <cmark/cmark-gfm-core-extensions.h>
#include <cmark/strikethrough.h>
#include <cmark/table.h>
/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_markdown_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_markdown_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_markdown_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_markdown_event(const lv_obj_class_t * class_p, lv_event_t * e);

static cmark_node * parser_document(const char * data, uint32_t data_len, int options);
static void render_contents(lv_obj_t * obj, cmark_node *root, int options);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_markdown_class = {
    .constructor_cb = lv_markdown_constructor,
    .destructor_cb = lv_markdown_destructor,
    .event_cb = lv_markdown_event,
    .instance_size = sizeof(lv_markdown_t),
    .base_class = &lv_obj_class,
    .name = "lv_markdown",
};

static cmark_mem mem_allocator = {
    .calloc = lv_calloc,
    .realloc = lv_realloc,
    .free = lv_free,
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * lv_markdown_create(lv_obj_t * parent)
{
    cmark_gfm_core_extensions_ensure_registered();

    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_markdown_set_data(lv_obj_t * obj, const char * data, uint32_t data_len)
{
    if (!data || !data_len) {
        return;
    }

    int opts = 0;
    opts |= CMARK_OPT_FOOTNOTES;
    opts |= CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE;
    opts |= CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES;
    opts |= CMARK_OPT_LIBERAL_HTML_TAG;


    lv_markdown_t * mark = (lv_markdown_t *)obj;
    if (mark->doc) {
        cmark_node_free(mark->doc);
        mark->doc = NULL;
    }
    // FIXME:release objs

    mark->doc = parser_document(data, data_len, opts);
    render_contents(obj, mark->doc, opts);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void lv_markdown_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_markdown_t * mark = (lv_markdown_t *)obj;
    mark->content = NULL;
    mark->doc = NULL;
}

static void lv_markdown_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_markdown_t * mark = (lv_markdown_t *)obj;
    if (mark->doc) {
        cmark_node_free(mark->doc);
        mark->doc = NULL;
    }
}

static void lv_markdown_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);
    LV_UNUSED(e);
}

static cmark_node * parser_document(const char * data, uint32_t data_len, int options)
{
    cmark_parser * parser = cmark_parser_new_with_mem(options, &mem_allocator);

    cmark_syntax_extension *table_extension = cmark_find_syntax_extension("table");
    cmark_parser_attach_syntax_extension(parser, table_extension);

    cmark_syntax_extension *strikethrough_extension = cmark_find_syntax_extension("strikethrough");
    cmark_parser_attach_syntax_extension(parser, strikethrough_extension);

    cmark_syntax_extension *autolink_extension = cmark_find_syntax_extension("autolink");
    cmark_parser_attach_syntax_extension(parser, autolink_extension);

    cmark_syntax_extension *tasklink_extension = cmark_find_syntax_extension("tasklist");
    cmark_parser_attach_syntax_extension(parser, tasklink_extension);

    cmark_syntax_extension *tagfilter_extension = cmark_find_syntax_extension("tagfilter");
    cmark_parser_attach_syntax_extension(parser, tagfilter_extension);

    cmark_parser_feed(parser, data, data_len);
    cmark_node * doc = cmark_parser_finish(parser);
    cmark_parser_free(parser);
    return doc;
}

static void render_node(cmark_node * node, cmark_event_type ev_type, lv_markdown_t * obj, int32_t options)
{
    bool entering = (ev_type == CMARK_EVENT_ENTER);

    int type = (int)cmark_node_get_type(node);

    switch (type) {
        // inline
        case CMARK_NODE_TEXT:
        case CMARK_NODE_CODE:
        case CMARK_NODE_SOFTBREAK:
        case CMARK_NODE_LINEBREAK:
        case CMARK_NODE_HTML_INLINE:
        case CMARK_NODE_CUSTOM_INLINE:
        case CMARK_NODE_IMAGE:
            printf("inline:  %s [%s]\n", cmark_node_get_type_string(node), cmark_node_get_literal(node));
            break;
        // block
        case CMARK_NODE_LINK:
        case CMARK_NODE_EMPH:
        case CMARK_NODE_STRONG:
        case CMARK_NODE_DOCUMENT:
        case CMARK_NODE_BLOCK_QUOTE:
        case CMARK_NODE_LIST:
        case CMARK_NODE_ITEM:
        case CMARK_NODE_CODE_BLOCK:
        case CMARK_NODE_HTML_BLOCK:
        case CMARK_NODE_CUSTOM_BLOCK:
        case CMARK_NODE_PARAGRAPH:
        case CMARK_NODE_HEADING:
        case CMARK_NODE_THEMATIC_BREAK:
        default:
            if(entering) {
                printf("enter block:  %s \n", cmark_node_get_type_string(node));
            } else {
                printf("leave block:  %s \n", cmark_node_get_type_string(node));
            }
            break;
        // extensine
            //CMARK_NODE_TABLE:
            //CMARK_NODE_TABLE_ROW:
            //CMARK_NODE_TABLE_CELL:
            //CMARK_NODE_STRIKETHROUGH:
    }
}

static void render_contents(lv_obj_t * obj, cmark_node *root, int options)
{
    lv_markdown_t * mark = (lv_markdown_t *)obj;
    cmark_event_type ev_type;
    cmark_node *cur = NULL;
    cmark_iter *iter = cmark_iter_new(root);

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cur = cmark_iter_get_node(iter);
        render_node(cur, ev_type, mark, options);
    }
    cmark_iter_free(iter);
}
#endif /*CONFIGLVX_USE_MARKWOWN*/
