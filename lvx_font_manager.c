/**
 * @file lvx_font_manager.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_font_manager.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

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

/**********************
 *  STATIC VARIABLES
 **********************/

/* save global fonts info */
static fonts_info_t font_tables;

/* font path */
static char * font_base_path = NULL;

/* first initialization flag */
static bool init_flag = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvx_font_init(uint16_t max_faces, uint16_t max_sizes, uint32_t max_bytes)
{
    if(init_flag) {
        return;
    }

    lv_freetype_init(max_faces, max_sizes, max_bytes);

    _lv_ll_init(&font_tables.ft_ll, sizeof(font_refer_t));
    _lv_ll_init(&font_tables.name_ll, sizeof(name_refer_t));

    if(!font_base_path) {
        const uint16_t buff_len = 256;
        char * buff = lv_mem_alloc(buff_len);
        LV_ASSERT_MALLOC(buff);

#ifdef __NuttX__
        int count = strlen(FONT_LIB_PATH);
        strncpy(buff, FONT_LIB_PATH, buff_len);
#else
        int count = readlink("/proc/self/exe", buff, buff_len);
#endif
        buff[count] = '\0';
        char * pend = strrchr(buff, '/');
        snprintf(pend, buff_len - (pend - buff), "/%s", "font");
        lvx_font_base_path_set(buff);
        lv_mem_free(buff);
    }

    init_flag = true;
}

void lvx_font_deinit(void)
{
    fts_list_clear();
    lv_freetype_destroy();
    if(font_base_path) {
        lv_mem_free(font_base_path);
    }

    init_flag = false;
}

void lvx_font_base_path_set(const char * path)
{
    uint32_t len = strlen(path) + 1;
    if(font_base_path) {
        font_base_path = lv_mem_realloc(font_base_path, len);
        LV_ASSERT_MALLOC(font_base_path);
    }
    else {
        font_base_path = lv_mem_alloc(len);
        LV_ASSERT_MALLOC(font_base_path);
    }

    memcpy(font_base_path, path, len);
}

bool lvx_font_create_core(lv_ft_info_t * newfont)
{
    lvx_font_init(FONT_MAX_FACES, FONT_MAX_SIZES, FONT_MAX_BYTES);

    if(newfont == NULL || newfont->name == NULL || newfont->weight == 0) {
        return false;
    }

    lv_ft_info_t * tmp = save_ft_to_fts(newfont);
    if(tmp) {
        newfont->font = tmp->font;
        return true;
    }

    newfont->font = NULL;
    return false;
}

lv_font_t * lvx_font_create(const char * name, uint16_t size, LV_FT_FONT_STYLE style)
{
    lv_ft_info_t newfont;
    newfont.font = NULL;
    newfont.name = name;
    newfont.style = style;
    newfont.weight = size;
    lvx_font_create_core(&newfont);
    return newfont.font;
}

void lvx_font_destroy(lv_font_t * delfont)
{
    if(delfont == NULL) {
        return;
    }

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

    name_refer_t * refer = _lv_ll_ins_tail(&font_tables.name_ll);
    if(refer) {
        uint32_t len = strlen(name) + 1;
        refer->name = lv_mem_alloc(len);
        if(refer->name) {
            memcpy(refer->name, name, len);
            refer->cnt = 1;
            return refer->name;
        }
        _lv_ll_remove(&font_tables.name_ll, refer);
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

    name_refer_t * refer = _lv_ll_get_head(&font_tables.name_ll);
    while(refer) {
        if(strcmp(refer->name, name) == 0) {
            refer->cnt += 1;
            return refer->name;
        }
        refer = _lv_ll_get_next(&font_tables.name_ll, refer);
    }
    return NULL;
}

/**
 * del name string from list.
 */
static void name_in_names_del(const char * name)
{
    name_refer_t * refer = _lv_ll_get_head(&font_tables.name_ll);
    while(refer) {
        if(strcmp(refer->name, name) == 0) {
            refer->cnt -= 1;
            if(refer->cnt <= 0) {
                _lv_ll_remove(&font_tables.name_ll, refer);
                lv_mem_free(refer->name);
                lv_mem_free(refer);
            }
            return;
        }
        refer = _lv_ll_get_next(&font_tables.name_ll, refer);
    }

    LV_LOG_WARN("name_in_names_del error(not find:%p).", name);
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
    if(!buff) {
        return false;
    }

    font_refer_t * refer = NULL;
    if(font_name_combine(buff, buff_len, newfont->name)) {
        lv_ft_info_t tmp_font;
        tmp_font.name = buff;
        tmp_font.font = NULL;
        tmp_font.style = newfont->style;
        tmp_font.weight = newfont->weight;
        if(lv_ft_font_init(&tmp_font)) {
            refer = _lv_ll_ins_tail(&font_tables.ft_ll);
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
    font_refer_t * refer = _lv_ll_get_head(&font_tables.ft_ll);
    while(refer) {
        if(refer->ft.weight == curfont->weight &&
           refer->ft.style == curfont->style &&
           strcmp(refer->ft.name, curfont->name) == 0) {
            refer->cnt += 1;
            return &refer->ft;
        }
        refer = _lv_ll_get_next(&font_tables.ft_ll, refer);
    }
    return NULL;
}

/**
 * del font from list.
 * @param delfont pointer.
 */
static void ft_in_fts_del(const lv_font_t * delfont)
{
    font_refer_t * refer = _lv_ll_get_head(&font_tables.ft_ll);
    while(refer) {
        if(refer->ft.font == delfont) {
            refer->cnt -= 1;
            if(refer->cnt <= 0) {
                lv_ft_font_destroy(refer->ft.font);
                name_in_names_del(refer->ft.name);
                _lv_ll_remove(&font_tables.ft_ll, refer);
                lv_mem_free(refer);
            }
            return;
        }
        refer = _lv_ll_get_next(&font_tables.ft_ll, refer);
    }

    LV_LOG_WARN("ft_in_fts_del error(not find:%p).", delfont);
}

/**
 * clear font list. free resource.
 */
static void fts_list_clear(void)
{
    font_refer_t * refer = _lv_ll_get_head(&font_tables.ft_ll);
    while(refer) {
        lv_ft_font_destroy(refer->ft.font);
        name_in_names_del(refer->ft.name);
        refer = _lv_ll_get_next(&font_tables.ft_ll, refer);
    }
    _lv_ll_clear(&font_tables.ft_ll);
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
    snprintf(buff, buff_len, "%s/%s.%s", font_base_path, name, "ttf");
    if(access(buff, F_OK) != 0) {
        return false;
    }
    return true;
}
