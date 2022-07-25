/**
 * @file lvx_font_manager.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_font_manager.h"

#if (LVX_USE_FONT_MANAGER != 0)

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef __NuttX__
#include <uv.h>
#endif

/*********************
 *      DEFINES
 *********************/

#define FONT_NAME_MAX_LEN 64

/**********************
 *      TYPEDEFS
 **********************/

typedef struct font_refer_t {
    lv_ft_info_t ft;        /* font info */
    int32_t cnt;            /* reference count */
} font_refer_t;

typedef struct name_refer_t {
    char * name;            /* point to font name string */
    int32_t cnt;            /* reference count */
} name_refer_t;

typedef struct fonts_info_t {
    lv_ll_t ft_ll;         /* font_refer_t list */
    lv_ll_t  name_ll;      /* name_refer_t list */
} fonts_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static char * save_name_to_names(const char * name);
static char * name_in_names_use(const char * name);
static void name_in_names_del(const char * name);
static lv_ft_info_t * save_ft_to_fts(const lv_ft_info_t * newfont);
static lv_ft_info_t * ft_in_fts_use(const lv_ft_info_t * curfont);
static void ft_in_fts_del(const lv_font_t * delfont);
static void fts_list_clear(void);
static bool font_name_combine(char * buff, uint16_t buff_len, const char * name);

#if defined(CONFIG_FONT_USE_EMOJI)
static bool emoji_get_imgfont_cb(const lv_font_t* font, void* img_src, uint16_t len,
                                 uint32_t unicode, uint32_t unicode_next);
#endif

#if defined(CONFIG_FONT_USE_FONT_FAMILY)
static lv_font_t * fm_fallback_add(const lv_ft_info_t * newfont, const char * list);
static void fm_fallback_remove(lv_font_t * font);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/* save global fonts info */
static fonts_info_t g_font_tables;

/* font path */
static char * g_font_base_path = NULL;

/* first initialization flag */
static bool g_init_flag = false;

#if defined(CONFIG_FONT_USE_EMOJI)
static lv_font_t * g_emoji_font = NULL;
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvx_font_init(void)
{
    if(g_init_flag) {
        return;
    }

    _lv_ll_init(&g_font_tables.ft_ll, sizeof(font_refer_t));
    _lv_ll_init(&g_font_tables.name_ll, sizeof(name_refer_t));

    if(!g_font_base_path) {
        const uint16_t buff_len = 256;
        char * buff = lv_mem_alloc(buff_len);
        LV_ASSERT_MALLOC(buff);

#ifdef __NuttX__
        strncpy(buff, FONT_LIB_PATH, buff_len);
#else
        size_t count = buff_len - 1;
        uv_exepath(buff, &count);
#endif
        buff[buff_len - 1] = '\0';
        char * pend = strrchr(buff, '/');
        snprintf(pend, buff_len - (pend - buff), "/%s", "font");
        lvx_font_base_path_set(buff);
        lv_mem_free(buff);
    }

#if defined(CONFIG_FONT_USE_EMOJI)
    lv_font_t * imgfont = lv_imgfont_create(CONFIG_FONT_EMOJI_SIZE_DEFAULT,
                                            emoji_get_imgfont_cb);
    LV_ASSERT_NULL(imgfont);

    if(imgfont) {
        g_emoji_font = imgfont;
    }
    else {
        LV_LOG_ERROR("emoji_font create faild");
    }
#endif

    g_init_flag = true;
}

void lvx_font_deinit(void)
{
    fts_list_clear();

    if(g_font_base_path) {
        lv_mem_free(g_font_base_path);
        g_font_base_path = NULL;
    }

#if defined(CONFIG_FONT_USE_EMOJI)
    if(g_emoji_font) {
        lv_imgfont_destroy(g_emoji_font);
        g_emoji_font = NULL;
    }
#endif

    g_init_flag = false;
}

void lvx_font_base_path_set(const char * path)
{
    uint32_t len = strlen(path) + 1;
    if(g_font_base_path) {
        g_font_base_path = lv_mem_realloc(g_font_base_path, len);
        LV_ASSERT_MALLOC(g_font_base_path);
    }
    else {
        g_font_base_path = lv_mem_alloc(len);
        LV_ASSERT_MALLOC(g_font_base_path);
    }

    memcpy(g_font_base_path, path, len);
}

bool lvx_font_create_core(lv_ft_info_t * newfont)
{
    lvx_font_init();

    if(newfont == NULL || newfont->name == NULL || newfont->weight == 0) {
        LV_LOG_ERROR("newfont param error");
        return false;
    }

    lv_ft_info_t * ft_info = save_ft_to_fts(newfont);
    if(!ft_info) {
        LV_LOG_ERROR("save_ft_to_fts() failed");
        newfont->font = NULL;
        return false;
    }

    newfont->font = ft_info->font;
    lv_font_t * cur_font = newfont->font;
    LV_UNUSED(cur_font);

#if defined(CONFIG_FONT_USE_FONT_FAMILY)
    lv_font_t * fm_font = fm_fallback_add(newfont, CONFIG_FONT_FAMILY_LIST);
    if(fm_font) {
        cur_font = fm_font;
    }
#endif

#if defined(CONFIG_FONT_USE_EMOJI)
    cur_font->fallback = g_emoji_font;
#endif

    return true;
}

lv_font_t * lvx_font_create(const char * name, uint16_t size, LV_FT_FONT_STYLE style)
{
    lv_ft_info_t newfont;
    newfont.font = NULL;
    newfont.name = name;
    newfont.style = style;
    newfont.weight = size;

    if(!lvx_font_create_core(&newfont)) {
        LV_LOG_ERROR("can't create font: %s", name);
        return NULL;
    }

    return newfont.font;
}

void lvx_font_destroy(lv_font_t * delfont)
{
    if(!delfont) {
        LV_LOG_WARN("delfont is NULL");
        return;
    }

#if defined(CONFIG_FONT_USE_FONT_FAMILY)
    fm_fallback_remove(delfont);
#endif

    ft_in_fts_del(delfont);
}

/**
 * save name string to list.
 * @param name name string
 * @return Saved string pointer
 */
static char * save_name_to_names(const char * name)
{
    char * pos = name_in_names_use(name);
    if(pos) {
        return pos;
    }

    name_refer_t * refer = _lv_ll_ins_tail(&g_font_tables.name_ll);
    if(refer) {
        uint32_t len = strlen(name) + 1;
        refer->name = lv_mem_alloc(len);
        if(refer->name) {
            memcpy(refer->name, name, len);
            refer->cnt = 1;
            return refer->name;
        }
        _lv_ll_remove(&g_font_tables.name_ll, refer);
        lv_mem_free(refer);
    }
    LV_LOG_WARN("save_name_to_names error(not memory).");
    return "";
}

/**
 * find name string in names list.name string cnt += 1 if find.
 * @param name name string
 * @return the string pointer of name.
 */
static char * name_in_names_use(const char * name)
{
    name_refer_t * refer = _lv_ll_get_head(&g_font_tables.name_ll);
    while(refer) {
        if(strcmp(refer->name, name) == 0) {
            refer->cnt += 1;
            return refer->name;
        }
        refer = _lv_ll_get_next(&g_font_tables.name_ll, refer);
    }
    return NULL;
}

/**
 * del name string from list.
 */
static void name_in_names_del(const char * name)
{
    name_refer_t * refer = _lv_ll_get_head(&g_font_tables.name_ll);
    while(refer) {
        if(strcmp(refer->name, name) == 0) {
            refer->cnt -= 1;
            if(refer->cnt <= 0) {
                _lv_ll_remove(&g_font_tables.name_ll, refer);
                lv_mem_free(refer->name);
                lv_mem_free(refer);
            }
            return;
        }
        refer = _lv_ll_get_next(&g_font_tables.name_ll, refer);
    }

    LV_LOG_WARN("name_in_names_del error, can't find:%s", name);
}

/**
 * save font info to list.
 * @param newfont font info pointer.
 * @return Saved font pointer.
 */
static lv_ft_info_t * save_ft_to_fts(const lv_ft_info_t * newfont)
{
    lv_ft_info_t * tmp = ft_in_fts_use(newfont);
    if(tmp) {
        return tmp;
    }

    const uint16_t buff_len = 256;
    char * buff = lv_mem_alloc(buff_len);
    LV_ASSERT_NULL(buff);
    if(!buff) {
        return NULL;
    }

    font_refer_t * refer = NULL;
    if(font_name_combine(buff, buff_len, newfont->name)) {
        lv_ft_info_t tmp_font;
        memset(&tmp_font, 0, sizeof(tmp_font));

        tmp_font.name = buff;
        tmp_font.font = NULL;
        tmp_font.style = newfont->style;
        tmp_font.weight = newfont->weight;
        if(lv_ft_font_init(&tmp_font)) {
            refer = _lv_ll_ins_tail(&g_font_tables.ft_ll);
            if(refer) {
                refer->ft.name = save_name_to_names(newfont->name);
                refer->ft.font = tmp_font.font;
                refer->ft.weight = newfont->weight;
                refer->ft.style = newfont->style;
                refer->cnt = 1;
            }
        }
    }

    lv_mem_free(buff);

    if(!refer) {
        return NULL;
    }
    return &refer->ft;
}

/**
 * find font in list.font refer cnt += 1 if find.
 * @param curfont pointer.
 * @return font pointer.
 */
static lv_ft_info_t * ft_in_fts_use(const lv_ft_info_t * curfont)
{
    font_refer_t * refer = _lv_ll_get_head(&g_font_tables.ft_ll);
    while(refer) {
        if(refer->ft.weight == curfont->weight &&
           refer->ft.style == curfont->style &&
           strcmp(refer->ft.name, curfont->name) == 0) {
            refer->cnt += 1;
            return &refer->ft;
        }
        refer = _lv_ll_get_next(&g_font_tables.ft_ll, refer);
    }
    return NULL;
}

/**
 * del font from list.
 * @param delfont pointer.
 */
static void ft_in_fts_del(const lv_font_t * delfont)
{
    if(!delfont) {
        LV_LOG_ERROR("delfont is NULL");
        return;
    }

    font_refer_t * refer = _lv_ll_get_head(&g_font_tables.ft_ll);
    while(refer) {
        if(refer->ft.font == delfont) {
            refer->cnt -= 1;
            if(refer->cnt <= 0) {
                lv_ft_font_destroy(refer->ft.font);
                name_in_names_del(refer->ft.name);
                _lv_ll_remove(&g_font_tables.ft_ll, refer);
                lv_mem_free(refer);
            }
            return;
        }
        refer = _lv_ll_get_next(&g_font_tables.ft_ll, refer);
    }

    LV_LOG_WARN("ft_in_fts_del error(not find:%p).", delfont);
}

/**
 * clear font list. free resource.
 */
static void fts_list_clear(void)
{
    font_refer_t * refer = _lv_ll_get_head(&g_font_tables.ft_ll);
    while(refer) {
        lv_ft_font_destroy(refer->ft.font);
        name_in_names_del(refer->ft.name);
        refer = _lv_ll_get_next(&g_font_tables.ft_ll, refer);
    }
    _lv_ll_clear(&g_font_tables.ft_ll);
}

/**
 * combine font path.
 * @param buff The combined string buff.
 * @param buff_len buff size.
 * @param name font name.
 * @return the string pointer of name.
 */
static bool font_name_combine(char * buff, uint16_t buff_len, const char * name)
{
    snprintf(buff, buff_len, "%s/%s.%s", g_font_base_path, name, "ttf");
    if(access(buff, F_OK) != 0) {
        LV_LOG_WARN("Font file not found.please check %s", buff);
        return false;
    }
    return true;
}

#if defined(CONFIG_FONT_USE_EMOJI)

#define IS_EMOJI(letter) \
    (letter >=  CONFIG_FONT_EMOJI_UNICODE_START \
  && letter <= (CONFIG_FONT_EMOJI_UNICODE_START + CONFIG_FONT_EMOJI_UNICODE_TOTAL))

static bool emoji_get_imgfont_cb(const lv_font_t* font, void* img_src, uint16_t len,
                                 uint32_t unicode, uint32_t unicode_next)
{
    LV_UNUSED(unicode_next);

    if (IS_EMOJI(unicode)) {
        char* path = (char*)img_src;
        lv_snprintf(path, len,
                    CONFIG_FONT_EMOJI_PATH "/%" PRIu32 "." CONFIG_FONT_EMOJI_EXT_NAME, unicode);
        return true;
    }

    return false;
}

#endif /* CONFIG_FONT_USE_EMOJI */

#if defined(CONFIG_FONT_USE_FONT_FAMILY)

static int fm_parser_get_next(const char * ori_str, int start_pos, char separator,
                              char * buf, size_t buf_size)
{
    buf[0] = '\0';
    size_t total_len = strlen(ori_str);

    if(!total_len) {
        LV_LOG_ERROR("ori_str length == 0");
        return -1;
    }

    const char * start_p = ori_str + start_pos;
    const char * end_p = strchr(start_p, separator);
    int retval;

    if(end_p) {
        retval = end_p - ori_str + 1;
    } else {
        end_p = ori_str + total_len;
        retval = -1;
    }

    size_t cpy_len = end_p - start_p;

    if(cpy_len > buf_size - 1) {
        LV_LOG_ERROR("Not enough buf space");
        return -1;
    }

    memcpy(buf, start_p, cpy_len);

    buf[cpy_len] = '\0';

    return retval;
}

static lv_font_t * fm_fallback_add(const lv_ft_info_t * newfont, const char * list)
{
    LV_ASSERT_NULL(newfont);
    LV_ASSERT_NULL(list);

    const char * name = newfont->name;
    size_t name_len = strlen(name);

    if(!name_len) {
        LV_LOG_WARN("name_len = 0");
        return NULL;
    }

    size_t list_len = strlen(list);

    if(!list_len) {
        LV_LOG_WARN("list_len = 0");
        return NULL;
    }

    lv_ft_info_t ft_info = *newfont;
    const lv_ft_info_t * cur_font = newfont;

    int family_pos = 0;
    do {
        char family_str[FONT_NAME_MAX_LEN];
        family_pos = fm_parser_get_next(list, family_pos, ';',
                                        family_str, sizeof(family_str));

        if(family_str[0] && strstr(family_str, name) == family_str) {

            LV_LOG_INFO("family: %s", family_str);
            int font_pos = name_len;
            do {
                char font_str[FONT_NAME_MAX_LEN];
                font_pos = fm_parser_get_next(family_str, font_pos, ',',
                                              font_str, sizeof(font_str));

                if(font_str[0]) {
                    LV_LOG_INFO("%s(%d) add fallback font -> %s(%d)",
                                name, newfont->weight,
                                font_str, ft_info.weight);
                    ft_info.name = font_str;

                    lv_ft_info_t * cur = save_ft_to_fts(&ft_info);

                    if(!cur) {
                        LV_LOG_ERROR("%s(%d) -> %s(%d) save_ft_to_fts() failed",
                                     name, newfont->weight,
                                     font_str, ft_info.weight);
                        continue;
                    }

                    cur_font->font->fallback = cur->font;

                    cur_font = cur;
                }
            } while(font_pos > 0);

            break;
        }

    } while(family_pos > 0);

    return cur_font->font;
}

static void fm_fallback_remove(lv_font_t * font)
{
    const lv_font_t * f = font->fallback;
    while(f) {
        const lv_font_t * fallback = f->fallback;
#if defined(CONFIG_FONT_USE_EMOJI)
        if(f != g_emoji_font) {
            ft_in_fts_del(f);
        }
#else
        ft_in_fts_del(f);
#endif
        f = fallback;
    }

    font->fallback = NULL;
}

#endif /* CONFIG_FONT_USE_FONT_FAMILY */

#endif /* LVX_USE_FONT_MANAGER */
