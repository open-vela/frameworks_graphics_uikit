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

#include "uikit/uikit_markdown.h"

#ifdef CONFIG_UIKIT_MARKDOWN

#include <cmark-gfm-core-extensions.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm.h>
#include <latexmath.h>
#include <strikethrough.h>
#include <table.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&vg_markdown_class)

#define LIST_MARKER_SIZE 8
#define UNSUPPORTED_NODE_HINT 128
#define THEMATIC_BREAK_PADDING 8

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    MARKDOWN_BLOCK_TYPE_TEXT = 0,
    MARKDOWN_BLOCK_TYPE_IMAGE,
    MARKDOWN_BLOCK_TYPE_THEMATIC_BREAK,
} markdown_block_type_t;

typedef struct {
    int32_t current_top;
    lv_obj_t* current_obj;

    bool is_inline;
    int32_t inline_block_width;
    int32_t inline_gap;
    int32_t inline_offset;

    markdown_block_type_t block_type;

    int32_t max_width;
    int32_t list_level;
    int32_t heading_level;
    int32_t emphasis_level;
    int32_t strong_level;
    bool strikethrough_enabled;
    int32_t unsupported_level;
    int32_t unsupported_node_type;

    const char* link_url;
    const char* link_title;
    const char* link_alt;
} markdown_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void vg_markdown_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void vg_markdown_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void vg_markdown_event(const lv_obj_class_t* class_p, lv_event_t* e);

static cmark_node* parser_document(const char* data, uint32_t data_len, int options);
static void render_contents(lv_obj_t* obj, cmark_node* root, int options);
static void add_text(vg_markdown_t* mark, const char* text);

static void default_markdown_heading_style_cb(lv_style_t* style, int32_t level);
static void default_markdown_list_marker_cb(char* buff, uint32_t size, int32_t level, int32_t index, bool ordered);
static void default_markdown_thematic_break_style_cb(lv_style_t* style);
static void default_markdown_paragraph_style_cb(lv_style_t* style);
static void default_markdown_text_deco_style_cb(lv_style_t* style, vg_markdown_decor_t decor);
static void default_markdown_url_create_cb(lv_span_t* span, const char* src, const char* title, const char* alt);
static lv_obj_t* default_markdown_image_create_cb(lv_obj_t* parent, const char* src, const char* title,
    const char* alt, int32_t width_hint);
static void default_markdown_unsupported_cb(char* buff, uint32_t size, lv_style_t* style, int32_t type);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t vg_markdown_class = {
    .constructor_cb = vg_markdown_constructor,
    .destructor_cb = vg_markdown_destructor,
    .event_cb = vg_markdown_event,
    .instance_size = sizeof(vg_markdown_t),
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

lv_obj_t* vg_markdown_create(lv_obj_t* parent)
{
    cmark_gfm_core_extensions_ensure_registered();

    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void vg_markdown_set_data(lv_obj_t* obj, const char* data, uint32_t data_len)
{
    if (!data || !data_len) {
        return;
    }

    int opts = 0;
    opts |= CMARK_OPT_FOOTNOTES;
    opts |= CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE;
    opts |= CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES;
    opts |= CMARK_OPT_LIBERAL_HTML_TAG;

    vg_markdown_t* mark = (vg_markdown_t*)obj;
    if (mark->doc) {
        cmark_node_free(mark->doc);
        mark->doc = NULL;
    }
    lv_obj_clean(obj);

    mark->doc = parser_document(data, data_len, opts);
    render_contents(obj, (cmark_node*)mark->doc, opts);
}

void vg_markdown_set_heading_style_cb(lv_obj_t* obj, vg_markdown_heading_style_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->heading_style_cb = cb;
}

void vg_markdown_set_list_marker_cb(lv_obj_t* obj, vg_markdown_list_marker_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->list_marker_cb = cb;
}

void vg_markdown_set_thematic_break_style_cb(lv_obj_t* obj, vg_markdown_thematic_break_style_cb_t cb)

{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->thematic_break_style_cb = cb;
}

void vg_markdown_set_paragraph_style_cb(lv_obj_t* obj, vg_markdown_paragraph_style_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->paragraph_style_cb = cb;
}

void vg_markdown_set_text_deco_style_cb(lv_obj_t* obj, vg_markdown_text_deco_style_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->text_deco_style_cb = cb;
}

void vg_markdown_set_url_create_cb(lv_obj_t* obj, vg_markdown_url_create_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->url_create_cb = cb;
}

void vg_markdown_set_image_url_process_cb(lv_obj_t* obj, vg_markdown_image_url_process_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->image_url_process_cb = cb;
}

void vg_markdown_set_image_create_cb(lv_obj_t* obj, vg_markdown_image_create_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->image_create_cb = cb;
}

void vg_markdown_set_unsupported_cb(lv_obj_t* obj, vg_markdown_unsupported_cb_t cb)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->unsupported_cb = cb;
}

void vg_markdown_set_list_marker_width(lv_obj_t* obj, int32_t width)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->list_marker_width = width;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void vg_markdown_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    mark->doc = NULL;

    lv_style_init(&mark->thematic_break_style);
    lv_style_init(&mark->paragraph_style);
    lv_style_init(&mark->url_style);

    mark->list_marker_width = 20;
}

static void vg_markdown_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    if (mark->doc) {
        cmark_node* node = (cmark_node*)mark->doc;
        cmark_node_free(node);
        mark->doc = NULL;
    }
}

static void vg_markdown_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);
    lv_obj_event_base(class_p, e);
    LV_UNUSED(e);
}

static cmark_node* parser_document(const char* data, uint32_t data_len, int options)
{
    cmark_parser* parser = cmark_parser_new_with_mem(options, &mem_allocator);

    cmark_syntax_extension* table_extension = cmark_find_syntax_extension("table");
    cmark_parser_attach_syntax_extension(parser, table_extension);

    cmark_syntax_extension* strikethrough_extension = cmark_find_syntax_extension("strikethrough");
    cmark_parser_attach_syntax_extension(parser, strikethrough_extension);

    cmark_syntax_extension* autolink_extension = cmark_find_syntax_extension("autolink");
    cmark_parser_attach_syntax_extension(parser, autolink_extension);

    cmark_syntax_extension* tasklink_extension = cmark_find_syntax_extension("tasklist");
    cmark_parser_attach_syntax_extension(parser, tasklink_extension);

    cmark_syntax_extension* tagfilter_extension = cmark_find_syntax_extension("tagfilter");
    cmark_parser_attach_syntax_extension(parser, tagfilter_extension);

    cmark_syntax_extension* latexmath_extension = cmark_find_syntax_extension("latexmath");
    cmark_parser_attach_syntax_extension(parser, latexmath_extension);

    cmark_parser_feed(parser, data, data_len);
    cmark_node* doc = cmark_parser_finish(parser);
    cmark_parser_free(parser);
    return doc;
}

static void enter_block(vg_markdown_t* mark)
{
    markdown_ctx_t* ctx = (markdown_ctx_t*)mark->ctx;
    if (ctx->current_obj == NULL || ctx->block_type != MARKDOWN_BLOCK_TYPE_TEXT) {
        int32_t offset = 0;
        int32_t width = ctx->max_width;
        if (ctx->list_level > 0)
            offset = (ctx->list_level - 1) * mark->list_marker_width;
        if (ctx->is_inline) {
            if (ctx->inline_block_width > 0)
                width = ctx->inline_block_width;
            else {
                offset += ctx->inline_offset;
                width -= offset;
            }

            ctx->inline_offset += ctx->inline_block_width + ctx->inline_gap;
            ctx->is_inline = false;
        }

        switch (ctx->block_type) {
        case MARKDOWN_BLOCK_TYPE_TEXT: {
            ctx->current_obj = lv_spangroup_create(&mark->obj);
            lv_obj_remove_flag(ctx->current_obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_pos(ctx->current_obj, offset, ctx->current_top);
            lv_obj_set_width(ctx->current_obj, width);
            lv_spangroup_set_mode(ctx->current_obj, LV_SPAN_MODE_BREAK);

            if (mark->paragraph_style_cb)
                mark->paragraph_style_cb(&mark->paragraph_style);
            else
                default_markdown_paragraph_style_cb(&mark->paragraph_style);

            lv_obj_add_style(ctx->current_obj, &mark->paragraph_style, LV_PART_MAIN);
            break;
        }
        case MARKDOWN_BLOCK_TYPE_IMAGE: {
            const char* url = ctx->link_url;
            char new_url[LV_FS_MAX_PATH_LENGTH];
            if (mark->image_url_process_cb) {
                mark->image_url_process_cb(ctx->link_url, new_url, sizeof(new_url));
                url = new_url;
            }
            if (mark->image_create_cb)
                ctx->current_obj = mark->image_create_cb(&mark->obj, url, ctx->link_title, ctx->link_alt, width);
            else
                ctx->current_obj = default_markdown_image_create_cb(&mark->obj, url, ctx->link_title, ctx->link_alt, width);

            lv_obj_set_pos(ctx->current_obj, offset, ctx->current_top);
            break;
        }
        case MARKDOWN_BLOCK_TYPE_THEMATIC_BREAK: {
            ctx->current_obj = lv_obj_create(&mark->obj);

            lv_obj_set_pos(ctx->current_obj, offset + THEMATIC_BREAK_PADDING, ctx->current_top);
            lv_obj_set_width(ctx->current_obj, width - THEMATIC_BREAK_PADDING * 2);
            lv_obj_set_height(ctx->current_obj, 4);

            if (mark->thematic_break_style_cb)
                mark->thematic_break_style_cb(&mark->thematic_break_style);
            else
                default_markdown_thematic_break_style_cb(&mark->thematic_break_style);

            lv_obj_add_style(ctx->current_obj, &mark->thematic_break_style, LV_PART_MAIN);
            break;
        }
        default:
            break;
        }
    }
}

static void exit_block(vg_markdown_t* mark)
{
    markdown_ctx_t* ctx = (markdown_ctx_t*)mark->ctx;
    if (ctx->current_obj == NULL)
        return;
    lv_obj_update_layout(ctx->current_obj);

    int32_t offset = 0;
    if (!ctx->is_inline)
        offset = lv_obj_get_height(ctx->current_obj) + 15;
    ctx->current_top += offset;
    ctx->current_obj = NULL;
}

void add_text(vg_markdown_t* mark, const char* text)
{
    markdown_ctx_t* ctx = (markdown_ctx_t*)mark->ctx;

    lv_span_t* span = lv_spangroup_new_span(ctx->current_obj);
    if (ctx->link_url || ctx->link_title) {
        ctx->link_alt = text;
        if (mark->url_create_cb)
            mark->url_create_cb(span, ctx->link_url, ctx->link_title, ctx->link_alt);
        else
            default_markdown_url_create_cb(span, ctx->link_url, ctx->link_title, ctx->link_alt);
    } else {
        if (ctx->unsupported_level == 1) {
            char unsupported_text[UNSUPPORTED_NODE_HINT];
            if (mark->unsupported_cb)
                mark->unsupported_cb(unsupported_text, sizeof(unsupported_text), &span->style, ctx->unsupported_node_type);
            else
                default_markdown_unsupported_cb(unsupported_text, sizeof(unsupported_text), &span->style, ctx->unsupported_node_type);
            lv_span_set_text(span, unsupported_text);
        } else
            lv_span_set_text(span, text);
    }

    if (ctx->heading_level > 0) {
        if (mark->heading_style_cb)
            mark->heading_style_cb(&span->style, ctx->heading_level);
        else
            default_markdown_heading_style_cb(&span->style, ctx->heading_level);
    }
    if (ctx->emphasis_level > 0) {
        if (mark->text_deco_style_cb)
            mark->text_deco_style_cb(&span->style, VG_MARKDOWN_DECOR_EM);
        else
            default_markdown_text_deco_style_cb(&span->style, VG_MARKDOWN_DECOR_EM);
    }
    if (ctx->strong_level > 0) {
        if (mark->text_deco_style_cb)
            mark->text_deco_style_cb(&span->style, VG_MARKDOWN_DECOR_STRONG);
        else
            default_markdown_text_deco_style_cb(&span->style, VG_MARKDOWN_DECOR_STRONG);
    }
    if (ctx->strikethrough_enabled) {
        if (mark->text_deco_style_cb)
            mark->text_deco_style_cb(&span->style, VG_MARKDOWN_DECOR_STRIKETHROUGH);
        else
            default_markdown_text_deco_style_cb(&span->style, VG_MARKDOWN_DECOR_STRIKETHROUGH);
    }
}

static void render_node(cmark_node* node, cmark_event_type ev_type, lv_obj_t* obj, int32_t options)
{
    LV_UNUSED(options);

    vg_markdown_t* mark = (vg_markdown_t*)obj;
    markdown_ctx_t* ctx = (markdown_ctx_t*)mark->ctx;
    bool entering = (ev_type == CMARK_EVENT_ENTER);
    cmark_node_type type = cmark_node_get_type(node);

    LV_LOG_INFO("[EVENT]: %s | [TYPE]: %s | LITERAL: %s", (entering ? "ENTER" : "EXIT"), cmark_node_get_type_string(node),
        cmark_node_get_literal(node));

    switch (type) {
    case CMARK_NODE_DOCUMENT:
        break;

    case CMARK_NODE_LIST: {
        if (entering)
            ctx->list_level++;
        else
            ctx->list_level--;
        break;
    }

    case CMARK_NODE_ITEM: {
        cmark_node* parent = cmark_node_parent(node);
        cmark_list_type parent_type = parent ? cmark_node_get_list_type(parent) : CMARK_NO_LIST;
        char list_marker[LIST_MARKER_SIZE];
        const int32_t list_index = cmark_node_get_item_index(node);
        if (mark->list_marker_cb)
            mark->list_marker_cb(list_marker, LIST_MARKER_SIZE,
                ctx->list_level, list_index, parent_type == CMARK_ORDERED_LIST);
        else
            default_markdown_list_marker_cb(list_marker, LIST_MARKER_SIZE,
                ctx->list_level, list_index, parent_type == CMARK_ORDERED_LIST);
        if (entering) {
            ctx->inline_offset = 0;
            ctx->is_inline = true;
            ctx->inline_block_width = mark->list_marker_width;
            ctx->inline_gap = 5;

            enter_block(mark);

            lv_span_t* span = lv_spangroup_new_span(ctx->current_obj);
            lv_span_set_text(span, list_marker);
            lv_spangroup_set_align(ctx->current_obj, LV_TEXT_ALIGN_RIGHT);

            ctx->is_inline = true;
            ctx->inline_block_width = 0;

            exit_block(mark);
        } else {
            exit_block(mark);
        }

        break;
    }

    case CMARK_NODE_HEADING: {
        if (entering) {
            ctx->heading_level = cmark_node_get_heading_level(node);
            enter_block(mark);
        } else {
            ctx->heading_level = 0;
            exit_block(mark);
        }
        break;
    }

    case CMARK_NODE_THEMATIC_BREAK: {
        ctx->block_type = MARKDOWN_BLOCK_TYPE_THEMATIC_BREAK;
        enter_block(mark);
        exit_block(mark);
        ctx->block_type = MARKDOWN_BLOCK_TYPE_TEXT;
        break;
    }

    case CMARK_NODE_PARAGRAPH: {
        ctx->block_type = MARKDOWN_BLOCK_TYPE_TEXT;

        cmark_node* child = cmark_node_first_child(node);
        cmark_node_type child_type = child ? cmark_node_get_type(child) : CMARK_NODE_NONE;
        if (child_type == CMARK_NODE_IMAGE)
            break;

        if (entering)
            enter_block(mark);
        else
            exit_block(mark);
        break;
    }

    case CMARK_NODE_TEXT: {
        if (ctx->block_type == MARKDOWN_BLOCK_TYPE_IMAGE) {
            ctx->link_alt = cmark_node_get_literal(node);
            break;
        }
        if (ctx->block_type != MARKDOWN_BLOCK_TYPE_TEXT || ctx->unsupported_level > 1)
            break;
        const char* text = cmark_node_get_literal(node);
        add_text(mark, text);
        break;
    }

    case CMARK_NODE_LINEBREAK:
    case CMARK_NODE_SOFTBREAK:
        break;

    case CMARK_NODE_STRONG: {
        if (entering)
            ctx->strong_level++;
        else
            ctx->strong_level--;
        break;
    }

    case CMARK_NODE_EMPH: {
        if (entering)
            ctx->emphasis_level++;
        else
            ctx->emphasis_level--;
        break;
    }

    case CMARK_NODE_LINK:
        if (entering) {
            ctx->link_url = cmark_node_get_url(node);
            ctx->link_title = cmark_node_get_title(node);
        } else {
            ctx->link_url = NULL;
            ctx->link_title = NULL;
        }
        break;

    case CMARK_NODE_IMAGE:
        if (entering) {
            ctx->link_url = cmark_node_get_url(node);
            ctx->link_title = cmark_node_get_title(node);

            ctx->block_type = MARKDOWN_BLOCK_TYPE_IMAGE;
        } else {
            enter_block(mark);
            exit_block(mark);

            ctx->link_url = NULL;
            ctx->link_title = NULL;
            ctx->link_alt = NULL;
        }
        break;

    case CMARK_NODE_FOOTNOTE_REFERENCE:
    case CMARK_NODE_FOOTNOTE_DEFINITION:
        break;

    default: {
        if (type == CMARK_NODE_STRIKETHROUGH) {
            ctx->strikethrough_enabled = entering;
            break;
        }

        /* Dealing unsupported nodes */
        if (ctx->unsupported_level == 0) {
            ctx->unsupported_node_type = cmark_node_get_type(node);
        }
        /* Only entering without exiting */
        switch (type) {
        case CMARK_NODE_CODE:
        case CMARK_NODE_CODE_BLOCK:
        case CMARK_NODE_HTML_BLOCK:
        case CMARK_NODE_HTML_INLINE:
            ctx->block_type = MARKDOWN_BLOCK_TYPE_TEXT;
            enter_block(mark);
            ctx->unsupported_level++;
            add_text(mark, NULL);
            ctx->unsupported_level--;
            if (type == CMARK_NODE_CODE_BLOCK || type == CMARK_NODE_HTML_BLOCK)
                exit_block(mark);
            LV_LOG_INFO("[%s NOT SUPPORT YET]\n", cmark_node_get_type_string(node));
            return;
        default:
            break;
        }

        /* Entering and exiting unsupported nodes */
        if (entering)
            ctx->unsupported_level++;
        else
            ctx->unsupported_level--;

        if (ctx->unsupported_level == 0 && !entering) {
            ctx->unsupported_level = 1;
            enter_block(mark);
            if (!(type == CMARK_NODE_LATEX_MATH)) {
                add_text(mark, NULL);
                exit_block(mark);
            }
            ctx->unsupported_level = 0;
            ctx->unsupported_node_type = 0;
        }

        LV_LOG_INFO("[%s NOT SUPPORT YET]\n", cmark_node_get_type_string(node));
        break;
    }
    }
}

static void render_contents(lv_obj_t* obj, cmark_node* root, int options)
{
    vg_markdown_t* mark = (vg_markdown_t*)obj;
    cmark_event_type ev_type;
    cmark_iter* iter = cmark_iter_new(root);

    markdown_ctx_t* ctx = lv_malloc_zeroed(sizeof(markdown_ctx_t));
    ctx->max_width = lv_obj_get_style_width(obj, LV_PART_MAIN);

    mark->ctx = ctx;

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cmark_node* cur = cmark_iter_get_node(iter);

        if (cmark_node_get_type(cur) == CMARK_NODE_ITEM) {
            cmark_node* prev = cmark_node_previous(cur);
            cmark_node* parent = cmark_node_parent(cur);
            int32_t idx = prev ? 1 + cmark_node_get_item_index(prev) : cmark_node_get_list_start(parent);
            cmark_node_set_item_index(cur, idx);
        }
        render_node(cur, ev_type, obj, options);
    }

    cmark_iter_free(iter);

    lv_free(ctx);
    mark->ctx = NULL;
}

static void default_markdown_heading_style_cb(lv_style_t* style, int32_t level)
{
#if LV_FONT_MONTSERRAT_48 && LV_FONT_MONTSERRAT_32 && LV_FONT_MONTSERRAT_24 && LV_FONT_MONTSERRAT_22 && LV_FONT_MONTSERRAT_18 && LV_FONT_MONTSERRAT_16
    const lv_font_t* fonts[] = {
#if LV_FONT_MONTSERRAT_48
        &lv_font_montserrat_48,
#endif
#if LV_FONT_MONTSERRAT_32
        &lv_font_montserrat_32,
#endif
#if LV_FONT_MONTSERRAT_24
        &lv_font_montserrat_24,
#endif
#if LV_FONT_MONTSERRAT_22
        &lv_font_montserrat_22,
#endif
#if LV_FONT_MONTSERRAT_18
        &lv_font_montserrat_18,
#endif
#if LV_FONT_MONTSERRAT_16
        &lv_font_montserrat_16,
#endif
    };

    const int32_t font_cnt = sizeof(fonts) / sizeof(fonts[0]);
    if (0 < level && level <= font_cnt) {
        lv_style_set_text_font(style, fonts[level - 1]);
    } else {
        lv_style_set_text_font(style, lv_font_default());
    }
#else
    lv_style_set_text_font(style, lv_font_default());
#endif
}
static void default_markdown_list_marker_cb(char* buff, uint32_t size, int32_t level, int32_t index, bool ordered)
{
    LV_UNUSED(level);
    if (ordered)
        lv_snprintf(buff, size, "%d. ", index);
    else
        lv_snprintf(buff, size, level == 1 ? "• " : "- ");
}
static void default_markdown_thematic_break_style_cb(lv_style_t* style)
{
    lv_style_set_bg_color(style, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_radius(style, 2);
}
static void default_markdown_paragraph_style_cb(lv_style_t* style)
{
    LV_UNUSED(style);
}
static void default_markdown_text_deco_style_cb(lv_style_t* style, vg_markdown_decor_t decor)
{
    switch (decor) {
    case VG_MARKDOWN_DECOR_STRIKETHROUGH:
        lv_style_set_text_decor(style, LV_TEXT_DECOR_STRIKETHROUGH);
        break;
    case VG_MARKDOWN_DECOR_EM:
        lv_style_set_text_decor(style, LV_TEXT_DECOR_UNDERLINE);
        break;
    case VG_MARKDOWN_DECOR_STRONG:
        lv_style_set_text_color(style, lv_color_hex(0x991123));
        break;
    case VG_MARKDOWN_DECOR_NONE:
        lv_style_set_text_font(style, lv_font_default());
        lv_style_set_text_decor(style, LV_TEXT_DECOR_NONE);
        break;
    default:
        break;
    }
}
static void default_markdown_url_create_cb(lv_span_t* span, const char* src, const char* title, const char* alt)
{
    LV_UNUSED(src);
    LV_UNUSED(title);

    lv_span_set_text(span, alt);
    lv_style_set_text_color(&span->style, lv_color_hex(0x114514));
}
static lv_obj_t* default_markdown_image_create_cb(lv_obj_t* parent, const char* src, const char* title,
    const char* alt, int32_t width_hint)
{
    LV_UNUSED(title);
    LV_UNUSED(alt);
    LV_UNUSED(width_hint);

    /**
     * If you just want to display the image url, you can use this code instead of creating an image object.
     *
     * lv_obj_t * label = lv_label_create(parent);
     * lv_label_set_text(label, src);
     * lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
     * lv_obj_set_width(label, width_hint);
     *
     * return label;
     */

    lv_obj_t* image = lv_image_create(parent);
    lv_image_set_src(image, src);

    return image;
}
static void default_markdown_unsupported_cb(char* buff, uint32_t size, lv_style_t* style, int32_t type)
{
    const char* type_str;
    if (type == CMARK_NODE_IMAGE) {
        type_str = "IMAGE";
    } else if (type == CMARK_NODE_CODE) {
        type_str = "CODE";
    } else if (type == CMARK_NODE_CODE_BLOCK) {
        type_str = "CODE BLOCK";
    } else if (type == CMARK_NODE_HTML_BLOCK) {
        type_str = "HTML BLOCK";
    } else if (type == CMARK_NODE_HTML_INLINE) {
        type_str = "HTML INLINE";
    } else if (type == CMARK_NODE_CUSTOM_BLOCK) {
        type_str = "CUSTOM BLOCK";
    } else if (type == CMARK_NODE_BLOCK_QUOTE) {
        type_str = "BLOCK QUOTE";
    } else if (type == CMARK_NODE_TABLE) {
        type_str = "TABLE";
    } else if (type == CMARK_NODE_LATEX_MATH) {
        type_str = "LATEX MATH";
    } else {
        type_str = "UNKNOWN";
    }
    lv_snprintf(buff, size, "[%s IS UNSUPPORTED YET]", type_str);
    lv_style_set_text_color(style, lv_palette_main(LV_PALETTE_GREY));
}
#endif /*CONFIG_UIKIT_MARKDOWN*/
