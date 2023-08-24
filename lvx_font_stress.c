/**
 * @file lvx_font_stress.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lvx_font_stress.h"
#include "lvgl/lvgl.h"
#include "lvx_font_manager.h"

#if (LVX_USE_FONT_MANAGER != 0)

#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/

#if LV_FREETYPE_CACHE_TYPE == LV_FREETYPE_OUTLINE_CACHE && defined(CONFIG_ARCH_SIM)
#define STRESS_TEST_OUTLINE 1
#else
#define STRESS_TEST_OUTLINE 0
#endif

#define VLC_OP_END 0x00
#define VLC_OP_CLOSE 0x01
#define VLC_OP_MOVE 0x02
#define VLC_OP_LINE 0x04
#define VLC_OP_QUAD 0x06
#define VLC_OP_CUBIC 0x08

#define VLC_SET_OP_CODE(data, op_code) (*((uint16_t*)(&data)) = (op_code))

#define FT_F26DOT6_TO_PATH_DATA(x) ((path_data_t)(x))

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t* par;
    lv_obj_t** label_arr;
    lvx_font_stress_config_t config;
} stress_test_ctx_t;

typedef int16_t path_data_t;

typedef struct {
    path_data_t* data;
    path_data_t* cur_ptr;
    int len;
} my_outline_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void stress_timer_cb(lv_timer_t* timer);

#if STRESS_TEST_OUTLINE
static void freetype_outline_event_cb(lv_event_t* e);
static void draw_letter(
    lv_draw_ctx_t* draw_ctx,
    const lv_draw_label_dsc_t* dsc,
    const lv_point_t* pos_p,
    uint32_t letter);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvx_font_stres_config_init(lvx_font_stress_config_t* config)
{
    lv_memset_00(config, sizeof(lvx_font_stress_config_t));
    config->loop_cnt = 10000;
    config->label_cnt = 128;
}

void lvx_font_stres_test(const lvx_font_stress_config_t* config)
{
    static stress_test_ctx_t ctx = { 0 };

    LV_ASSERT_NULL(config);
    LV_ASSERT_NULL(config->font_name_arr);
    LV_ASSERT(config->font_cnt > 0);
    LV_ASSERT(config->label_cnt > 0);

    ctx.config = *config;
    ctx.par = lv_scr_act();

    size_t arr_size = sizeof(lv_obj_t*) * ctx.config.label_cnt;
    ctx.label_arr = lv_mem_alloc(arr_size);
    LV_ASSERT_MALLOC(ctx.label_arr);
    lv_memset_00(ctx.label_arr, arr_size);

#if STRESS_TEST_OUTLINE
    lv_freetype_outline_set_ref_size(128);
    lv_freetype_outline_add_event(freetype_outline_event_cb, LV_EVENT_ALL, NULL);
    lv_disp_get_default()->driver->draw_ctx->draw_letter = draw_letter;
#endif

    lv_timer_create(stress_timer_cb, 10, &ctx);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if STRESS_TEST_OUTLINE

static int16_t outline_check_sum(const int16_t* buf, uint16_t len)
{
    int16_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return sum;
}

static void draw_letter(
    lv_draw_ctx_t* draw_ctx,
    const lv_draw_label_dsc_t* dsc,
    const lv_point_t* pos_p,
    uint32_t letter)
{
    lv_font_glyph_dsc_t g;
    bool g_ret = lv_font_get_glyph_dsc(dsc->font, &g, letter, '\0');
    if (!g_ret) {
        /*Add warning if the dsc is not found
         *but do not print warning for non printable ASCII chars (e.g. '\n')*/
        if (letter >= 0x20 && letter != 0xf8ff && /*LV_SYMBOL_DUMMY*/
            letter != 0x200c) { /*ZERO WIDTH NON-JOINER*/
            LV_LOG_INFO("lv_draw_letter: glyph dsc. not found for U+%" LV_PRIX32, letter);
#if LV_USE_FONT_PLACEHOLDER
            /* draw placeholder */
            lv_area_t glyph_coords;
            lv_draw_rect_dsc_t glyph_dsc;
            lv_coord_t begin_x = pos_p->x + g.ofs_x;
            lv_coord_t begin_y = pos_p->y + g.ofs_y;
            lv_area_set(&glyph_coords, begin_x, begin_y, begin_x + g.box_w, begin_y + g.box_h);
            lv_draw_rect_dsc_init(&glyph_dsc);
            glyph_dsc.bg_opa = LV_OPA_MIN;
            glyph_dsc.outline_opa = LV_OPA_MIN;
            glyph_dsc.shadow_opa = LV_OPA_MIN;
            glyph_dsc.bg_img_opa = LV_OPA_MIN;
            glyph_dsc.border_color = dsc->color;
            glyph_dsc.border_width = 1;
            draw_ctx->draw_rect(draw_ctx, &glyph_dsc, &glyph_coords);
#endif
        }
        return;
    }
    /*Don't draw anything if the character is empty. E.g. space*/
    if ((g.box_h == 0) || (g.box_w == 0))
        return;
    lv_coord_t real_h;
#if LV_USE_IMGFONT
    if (g.bpp == LV_IMGFONT_BPP) {
        /*Center imgfont's drawing position*/
        real_h = (dsc->font->line_height - g.box_h) / 2;
    } else
#endif
    {
        real_h = (dsc->font->line_height - dsc->font->base_line) - g.box_h;
    }
    lv_point_t gpos;
    gpos.x = pos_p->x + g.ofs_x;
    gpos.y = pos_p->y + real_h - g.ofs_y;
    /*If the letter is completely out of mask don't draw it*/
    if (gpos.x + g.box_w < draw_ctx->clip_area->x1 || gpos.x > draw_ctx->clip_area->x2
        || gpos.y + g.box_h < draw_ctx->clip_area->y1 || gpos.y > draw_ctx->clip_area->y2) {
        return;
    }

    const lv_font_t* font_p = g.resolved_font;

    /* not draw bitmap font */
    if (!lv_freetype_is_outline_font(font_p)) {
        return;
    }
    my_outline_t* outline = (my_outline_t*)lv_font_get_glyph_bitmap(font_p, letter);

    if (!outline) {
        LV_LOG_WARN("get outline failed");
        return;
    }

    int16_t sum = outline_check_sum(outline->data, outline->len - 1);
    LV_ASSERT_MSG(sum == *(outline->cur_ptr - 1), "check sum error!");
}

static lv_freetype_outline_t outline_create(const lv_freetype_outline_event_param_t* param)
{
    int path_data_len = param->counter.cmd + param->counter.vector * 2;

    if (path_data_len <= 0) {
        LV_LOG_ERROR("path_data_len = %d", path_data_len);
        return NULL;
    }

    my_outline_t* outline = lv_mem_alloc(sizeof(my_outline_t));
    LV_ASSERT_MALLOC(outline);
    if (!outline) {
        LV_LOG_WARN("outline alloc failed");
        return NULL;
    }
    lv_memset_00(outline, sizeof(my_outline_t));

    size_t path_data_size = path_data_len * sizeof(path_data_t);

    path_data_t* path_data = lv_mem_alloc(path_data_size);
    LV_ASSERT_MALLOC(path_data);
    if (!path_data) {
        LV_LOG_WARN("path_data alloc failed");
        lv_mem_free(outline);
        return NULL;
    }
    lv_memset_00(path_data, path_data_size);

    outline->data = path_data;
    outline->cur_ptr = path_data;
    outline->len = path_data_len;

    return outline;
}

static void outline_delete(const lv_freetype_outline_event_param_t* param)
{
    my_outline_t* outline = param->outline;
    LV_ASSERT_NULL(outline);
    LV_ASSERT_NULL(outline->data);
    if (outline) {
        int16_t sum = outline_check_sum(outline->data, outline->len - 1);
        LV_ASSERT_MSG(sum == *(outline->cur_ptr - 1), "check sum error!");
        lv_mem_free(outline->data);
        lv_mem_free(outline);
    }
}

static void outline_push(const lv_freetype_outline_event_param_t* param)
{
    my_outline_t* outline = param->outline;
    LV_ASSERT_NULL(outline);
    LV_ASSERT_NULL(outline->data);

    int offset = outline->cur_ptr - outline->data;

    if (offset >= outline->len) {
        LV_LOG_ERROR("offset = %d, path_data_len = %d, overflow!",
            offset, outline->len);
        return;
    }

    lv_freetype_outline_type_t type = param->type;
    switch (type) {
    case LV_FREETYPE_OUTLINE_END:
        VLC_SET_OP_CODE(*outline->cur_ptr++, VLC_OP_END);

        /* mark check sum */
        LV_ASSERT(outline->len > 1);
        int16_t sum = outline_check_sum(outline->data, outline->len - 1);
        *(outline->data + outline->len - 1) = sum;
        LV_LOG_INFO("outline_check_sum = %d", sum);
        break;
    case LV_FREETYPE_OUTLINE_MOVE_TO:
        VLC_SET_OP_CODE(*outline->cur_ptr++, VLC_OP_MOVE);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.x);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.y);
        break;
    case LV_FREETYPE_OUTLINE_LINE_TO:
        VLC_SET_OP_CODE(*outline->cur_ptr++, VLC_OP_LINE);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.x);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.y);
        break;
    case LV_FREETYPE_OUTLINE_CUBIC_TO:
        VLC_SET_OP_CODE(*outline->cur_ptr++, VLC_OP_CUBIC);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->control1.x);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->control1.y);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->control2.x);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->control2.y);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.x);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.y);
        break;
    case LV_FREETYPE_OUTLINE_CONIC_TO:
        VLC_SET_OP_CODE(*outline->cur_ptr++, VLC_OP_QUAD);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->control1.x);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->control1.y);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.x);
        *outline->cur_ptr++ = FT_F26DOT6_TO_PATH_DATA(param->to.y);
        break;
    default:
        LV_LOG_WARN("unknown point type: %d", type);
        break;
    }

    offset = outline->cur_ptr - outline->data;
    LV_ASSERT(offset <= outline->len);
}

static void freetype_outline_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_freetype_outline_event_param_t* param = lv_event_get_param(e);
    switch (code) {
    case LV_EVENT_CREATE:
        param->outline = outline_create(param);
        break;
    case LV_EVENT_DELETE:
        outline_delete(param);
        break;
    case LV_EVENT_INSERT:
        outline_push(param);
        break;
    default:
        LV_LOG_WARN("unknown event code: %d", code);
        break;
    }
}

#endif /*STRESS_TEST_OUTLINE*/

static int random_one_utf8_char(char* buf, int buf_len, uint32_t char_range_min, uint32_t char_range_max)
{
    if (buf_len < 1) {
        return -1;
    }

    int buf_index = 0;

    uint32_t r = rand() % (char_range_max - char_range_min + 1) + char_range_min;
    if (r < 0x80) {
        buf[buf_index++] = r;
    } else if (r < 0x800) {
        buf[buf_index++] = 0xc0 | (r >> 6);
        buf[buf_index++] = 0x80 | (r & 0x3f);
    } else if (r < 0x10000) {
        buf[buf_index++] = 0xe0 | (r >> 12);
        buf[buf_index++] = 0x80 | ((r >> 6) & 0x3f);
        buf[buf_index++] = 0x80 | (r & 0x3f);
    } else if (r < 0x200000) {
        buf[buf_index++] = 0xf0 | (r >> 18);
        buf[buf_index++] = 0x80 | ((r >> 12) & 0x3f);
        buf[buf_index++] = 0x80 | ((r >> 6) & 0x3f);
        buf[buf_index++] = 0x80 | (r & 0x3f);
    } else if (r < 0x4000000) {
        buf[buf_index++] = 0xf8 | (r >> 24);
        buf[buf_index++] = 0x80 | ((r >> 18) & 0x3f);
        buf[buf_index++] = 0x80 | ((r >> 12) & 0x3f);
        buf[buf_index++] = 0x80 | ((r >> 6) & 0x3f);
        buf[buf_index] = 0x80 | (r & 0x3f);
    } else {
        buf[buf_index++] = 0xfc | (r >> 30);
        buf[buf_index++] = 0x80 | ((r >> 24) & 0x3f);
        buf[buf_index++] = 0x80 | ((r >> 18) & 0x3f);
        buf[buf_index++] = 0x80 | ((r >> 12) & 0x3f);
        buf[buf_index++] = 0x80 | ((r >> 6) & 0x3f);
        buf[buf_index++] = 0x80 | (r & 0x3f);
    }

    return buf_index;
}
static int random_utf8_chars(char* buf, int buf_len, int char_num)
{
    uint32_t ranges[][2] = {
        { 0x4e00, 0x9fa5 }, // CJK Unified Ideographs
        { 'A', 'Z' },
        { 'a', 'z' },
        { '0', '9' },
    };

    if (buf_len < char_num) {
        return -1;
    }

    int range_num = sizeof(ranges) / sizeof(ranges[0]);
    int buf_index = 0;
    for (int i = 0; i < char_num && buf_index < buf_len; i++) {
        int range_index = rand() % range_num;
        int ret = random_one_utf8_char(buf + buf_index, buf_len - buf_index, ranges[range_index][0], ranges[range_index][1]);
        if (ret < 0) {
            return -1;
        }
        buf_index += ret;
    }

    buf[buf_index] = '\0';
    return buf_index;
}

static lv_obj_t* label_create(const char* font_name, lv_obj_t* par, int size, int x, int y)
{
    static const uint16_t style[] = {
        LV_FREETYPE_FONT_STYLE_NORMAL,
        LV_FREETYPE_FONT_STYLE_ITALIC,
        LV_FREETYPE_FONT_STYLE_BOLD,
    };

    int index = lv_rand(0, 2);

    lv_font_t* font = lvx_font_create(font_name, size, style[index]);

    if (!font) {
        return NULL;
    }

    lv_obj_t* label = lv_label_create(par);
    lv_obj_set_style_text_font(label, font, 0);

    /*generate random color and opa*/
    lv_color_t color = lv_color_make(lv_rand(0, 255), lv_rand(0, 255), lv_rand(0, 255));
    lv_opa_t opa = lv_rand(0, 255);

    lv_obj_set_style_text_opa(label, opa, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_outline_color(label, color, 0);
    lv_obj_set_style_outline_width(label, 1, 0);

    char str[128];

    random_utf8_chars(str, sizeof(str), 16);

    LV_LOG_USER("create label: %s", str);
    lv_label_set_text(label, str);
    return label;
}

static void label_delete(lv_obj_t* label)
{
    const lv_font_t* font = lv_obj_get_style_text_font(label, 0);
    LV_ASSERT_NULL(font);
    lvx_font_destroy((lv_font_t*)font);
    lv_obj_del(label);
}

static void label_delete_all(stress_test_ctx_t* ctx)
{
    for (int i = 0; i < ctx->config.label_cnt; i++) {
        lv_obj_t* label = ctx->label_arr[i];
        if (label) {
            label_delete(label);
            ctx->label_arr[i] = NULL;
        }
    }
}

static void stress_timer_cb(lv_timer_t* timer)
{
    stress_test_ctx_t* ctx = timer->user_data;

    uint32_t label_index = lv_rand(0, ctx->config.label_cnt - 1);
    lv_obj_t* label = ctx->label_arr[label_index];

    if (label) {
        label_delete(label);
        ctx->label_arr[label_index] = NULL;
    } else {
        uint32_t font_index = lv_rand(0, ctx->config.font_cnt - 1);
        const char* pathname = ctx->config.font_name_arr[font_index];
        LV_ASSERT_NULL(pathname);

        label = label_create(
            pathname,
            ctx->par,
            lv_rand(0, 300),
            lv_rand(0, LV_HOR_RES) - LV_HOR_RES / 2,
            lv_rand(0, LV_VER_RES) - LV_VER_RES / 2);
        ctx->label_arr[label_index] = label;
    }

    if (ctx->config.loop_cnt < 0) {
        return;
    }

    ctx->config.loop_cnt--;
    LV_LOG_USER("loop remain cnt: %d", ctx->config.loop_cnt);

    if (ctx->config.loop_cnt == 0) {
        label_delete_all(ctx);
        lvx_font_deinit();
        lv_freetype_uninit();
        lv_timer_del(timer);
        LV_LOG_USER("stress test done");
    }
}

#endif /*LVX_USE_FONT_MANAGER*/
