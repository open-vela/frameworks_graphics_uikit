/**
 * @file font_manager.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "font_manager.h"
#include "font_cache.h"
#include "font_emoji.h"
#include "font_utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

#define IS_EMOJI_NAME(name) (strstr((name), UIKIT_FONT_EMOJI_HEADER) == (name))

/**********************
 *      TYPEDEFS
 **********************/

/* freetype font reference node */
typedef struct _font_refer_node_t {
    lv_font_t* font_p; /* lv_freetype gen font */
    lv_freetype_info_t ft_info; /* freetype font info */
    char name[UIKIT_FONT_NAME_MAX]; /* name buffer */
    int ref_cnt; /* reference count */
} font_refer_node_t;

/* lvgl font record node */
typedef struct _font_rec_node_t {
    lv_font_t font; /* lvgl font info */
    font_refer_node_t* refer_node_p; /* referenced freetype resource */
} font_rec_node_t;

/* font manager object */
typedef struct vg_font_manager_t {
    lv_ll_t refer_ll; /* freetype font record list */
    lv_ll_t rec_ll; /* lvgl font record list */
    lv_ll_t path_ll; /* font path record list */
    char base_path[PATH_MAX]; /* font base path */
    char def_path[PATH_MAX];

#if UIKIT_FONT_USE_FONT_FAMILY
    font_family_config_t* font_family_config;
#endif /* UIKIT_FONT_USE_FONT_FAMILY */

#if UIKIT_FONT_USE_EMOJI
    font_emoji_manager_t* emoji_manager;
#endif /* UIKIT_FONT_USE_EMOJI */

#if (UIKIT_FONT_CACHE_SIZE > 0)
    font_cache_manager_t* cache_manager;
#endif /* UIKIT_FONT_CACHE_SIZE */
} font_manager_t;

struct _font_path_t {
    char* name;
    char* path;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

static const char* font_manager_get_path(font_manager_t* manager, const char* name);
static void font_manager_remove_path_all(font_manager_t* manager);
static bool font_manager_check_resource(font_manager_t* manager);
static font_refer_node_t* font_manager_request_font(font_manager_t* manager, const lv_freetype_info_t* ft_info);
static bool font_manager_drop_font(font_manager_t* manager, font_refer_node_t* refer_node);
static font_rec_node_t* font_manager_search_rec_node(font_manager_t* manager, lv_font_t* font);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

font_manager_t* font_manager_create(void)
{
    font_manager_t* manager = lv_malloc(sizeof(font_manager_t));
    LV_ASSERT_MALLOC(manager);
    if (!manager) {
        LV_LOG_ERROR("malloc failed for font_manager_t");
        return NULL;
    }
    lv_memzero(manager, sizeof(font_manager_t));

    _lv_ll_init(&manager->refer_ll, sizeof(font_refer_node_t));
    _lv_ll_init(&manager->rec_ll, sizeof(font_rec_node_t));
    _lv_ll_init(&manager->path_ll, sizeof(font_path_t));

#if UIKIT_FONT_USE_FONT_FAMILY
    /* Open the font configuration file */
    font_utils_json_obj_t* json_obj = font_utils_json_obj_create(UIKIT_FONT_CONFIG_FILE_PATH);
    if (json_obj) {

#if UIKIT_FONT_USE_EMOJI
        manager->emoji_manager = font_emoji_manager_create(json_obj);
#endif /* UIKIT_FONT_USE_EMOJI */

        manager->font_family_config = font_utils_json_get_font_family_config(json_obj);

        font_utils_json_obj_delete(json_obj);
    }
#endif /* UIKIT_FONT_USE_FONT_FAMILY */

#if (UIKIT_FONT_CACHE_SIZE > 0)
    manager->cache_manager = font_cache_manager_create(UIKIT_FONT_CACHE_SIZE);
#endif /* UIKIT_FONT_CACHE_SIZE */

    LV_LOG_INFO("success");
    return manager;
}

bool font_manager_delete(font_manager_t* manager)
{
    LV_ASSERT_NULL(manager);

    /* Resource leak check */
    if (font_manager_check_resource(manager)) {
        LV_LOG_ERROR("Unfreed resource detected, delete failed!");
        return false;
    }

#if UIKIT_FONT_USE_FONT_FAMILY
    if (manager->font_family_config) {
        font_utils_json_font_family_free(manager->font_family_config);
    }

#if UIKIT_FONT_USE_EMOJI
    if (manager->emoji_manager) {
        font_emoji_manager_delete(manager->emoji_manager);
    }
#endif /* UIKIT_FONT_USE_EMOJI */

#endif /* UIKIT_FONT_USE_FONT_FAMILY */

#if (UIKIT_FONT_CACHE_SIZE > 0)
    font_cache_manager_delete(manager->cache_manager);
#endif /* UIKIT_FONT_CACHE_SIZE */

    font_manager_remove_path_all(manager);

    lv_free(manager);

    LV_LOG_INFO("success");
    return true;
}

void font_manager_set_base_path(font_manager_t* manager, const char* base_path)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(base_path);
    size_t max_len = sizeof(manager->base_path);
    strncpy(manager->base_path, base_path, max_len);
    manager->base_path[max_len - 1] = '\0';
    LV_LOG_USER("%s", manager->base_path);
}

font_path_t* font_manager_add_path(font_manager_t* manager, const char* name, const char* path)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(name);
    LV_ASSERT_NULL(path);

    const char* old_path = font_manager_get_path(manager, name);
    if (strcmp(path, old_path) == 0) {
        LV_LOG_WARN("name: %s, path: %s already exists", name, old_path);
        return NULL;
    }

    font_path_t* font_path = _lv_ll_ins_tail(&manager->path_ll);
    LV_ASSERT_MALLOC(font_path);

    uint32_t name_len = strlen(name) + 1;
    font_path->name = lv_malloc(name_len);
    LV_ASSERT_MALLOC(font_path->name);
    lv_memcpy(font_path->name, name, name_len);

    uint32_t path_len = strlen(path) + 1;
    font_path->path = lv_malloc(path_len);
    LV_ASSERT_MALLOC(font_path->path);
    lv_memcpy(font_path->path, path, path_len);

    LV_LOG_USER("name: %s, path: %s add success", name, path);
    return font_path;
}

bool font_manager_remove_path(font_manager_t* manager, font_path_t* handle)
{
    LV_ASSERT_NULL(manager);

    font_path_t* font_path;
    _LV_LL_READ(&manager->path_ll, font_path)
    {
        if (font_path == handle) {
            _lv_ll_remove(&manager->path_ll, font_path);
            LV_LOG_USER("remove name: %s, path: %s", font_path->name, font_path->path);
            lv_free(font_path->name);
            lv_free(font_path->path);
            lv_free(font_path);
            return true;
        }
    }

    LV_LOG_WARN("handle %p is invalid", handle);
    return false;
}

lv_font_t* font_manager_create_font(font_manager_t* manager, const lv_freetype_info_t* ft_info)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(ft_info);

    /* Request freetype font */
    font_refer_node_t* refer_node = font_manager_request_font(manager, ft_info);
    if (!refer_node) {
        return NULL;
    }

    /* Add font record node */
    font_rec_node_t* rec_node = _lv_ll_ins_head(&manager->rec_ll);
    LV_ASSERT_MALLOC(rec_node);
    lv_memzero(rec_node, sizeof(font_rec_node_t));

    /* copy freetype_font data */
    rec_node->font = *refer_node->font_p;

    /* record reference node */
    rec_node->refer_node_p = refer_node;

    LV_LOG_INFO("success");
    return &rec_node->font;
}

bool font_manager_delete_font(font_manager_t* manager, lv_font_t* font)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(font);

    /* check font is created by font manager */
    font_rec_node_t* rec_node = font_manager_search_rec_node(manager, font);
    if (!rec_node) {
        LV_LOG_WARN("No record found for font: %p(%d),"
                    " it was not created by font manager",
            font, (int)font->line_height);
        return false;
    }

    /* return freetype font resource */
    bool retval = font_manager_drop_font(manager, rec_node->refer_node_p);
    LV_ASSERT(retval);

    /* free rec_node */
    _lv_ll_remove(&manager->rec_ll, rec_node);
    lv_free(rec_node);

    LV_LOG_INFO("success");
    return retval;
}

#if UIKIT_FONT_USE_FONT_FAMILY

lv_font_t* font_manager_create_font_family(font_manager_t* manager, const lv_freetype_info_t* ft_info)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(ft_info);

    if (!manager->font_family_config) {
        LV_LOG_WARN("No font_family_config detected");
        return NULL;
    }

    lv_font_t* start_font = NULL;
    lv_font_t* cur_font = NULL;
    lv_freetype_info_t ft_info_tmp = *ft_info;

    int font_family_arr_size = manager->font_family_config->font_family_arr_size;
    for (int font_index = 0; font_index < font_family_arr_size; font_index++) {
        font_family_t* font_family = &manager->font_family_config->font_family_arr[font_index];

        /* match font */
        if (strcmp(ft_info->name, font_family->font_name) == 0) {
            LV_LOG_INFO("matched font-family: %s", ft_info->name);

            /* add fallback */
            int fallback_arr_size = font_family->fallback_arr_size;
            for (int fallback_index = 0; fallback_index < fallback_arr_size; fallback_index++) {
                font_family_fallback_t* fallback = &font_family->fallback_arr[fallback_index];

                ft_info_tmp.name = fallback->font_name;

                /* create fallback font */
                lv_font_t* font = font_manager_create_font(manager, &ft_info_tmp);
                if (!font) {
                    LV_LOG_INFO("%s(%d) <- %s(%d) create failed, continue...",
                        ft_info->name, ft_info->size,
                        ft_info_tmp.name, ft_info->size);
                    continue;
                }

                /* save start_font for reture value */
                if (!start_font) {
                    start_font = font;
                }

                if (cur_font) {
                    /*append fallback*/
                    cur_font->fallback = font;
                }
                cur_font = font;
            }

            break;
        }
    }

    return start_font;
}

void font_manager_delete_font_family(font_manager_t* manager, lv_font_t* font)
{
    LV_ASSERT_NULL(font);

    const lv_font_t* f = font;
    while (f) {
        const lv_font_t* fallback = f->fallback;
        font_manager_delete_font(manager, (lv_font_t*)f);
        f = fallback;
    }
}

#endif /* UIKIT_FONT_USE_FONT_FAMILY */

/**********************
 *   STATIC FUNCTIONS
 **********************/

static const char* font_manager_generate_def_path(font_manager_t* manager, const char* name)
{
    int len = lv_snprintf(
        manager->def_path,
        sizeof(manager->def_path),
        "%s/%s." UIKIT_FONT_EXT_NAME,
        manager->base_path,
        name);

    if (len >= sizeof(manager->def_path)) {
        LV_LOG_WARN("path truncation detected, len = %d", len);
    }

    return manager->def_path;
}

static const char* font_manager_get_path(font_manager_t* manager, const char* name)
{
    font_path_t* font_path;
    _LV_LL_READ(&manager->path_ll, font_path)
    {
        if (strcmp(name, font_path->name) == 0) {
            return font_path->path;
        }
    }

    return font_manager_generate_def_path(manager, name);
}

static void font_manager_remove_path_all(font_manager_t* manager)
{
    font_path_t* font_path;
    _LV_LL_READ(&manager->path_ll, font_path)
    {
        LV_LOG_USER("remove %s", font_path->name);
        lv_free(font_path->name);
        lv_free(font_path->path);
    }

    _lv_ll_clear(&manager->path_ll);
}

static bool font_manager_check_font_file(font_manager_t* manager, const char* name)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(name);

    const char* path = font_manager_get_path(manager, name);

    /* Check if font file exists */
    int ret = access(path, F_OK);
    if (ret != 0) {
        LV_LOG_WARN("Can't access font file: %s", path);
        return false;
    }

    LV_LOG_INFO("font: %s access OK", path);
    return true;
}

static bool font_manager_check_resource(font_manager_t* manager)
{
    LV_ASSERT_NULL(manager);

    /* Check the recorded font */
    lv_ll_t* rec_ll = &manager->rec_ll;
    int rec_ll_len = _lv_ll_get_len(rec_ll);
    if (rec_ll_len) {
        LV_LOG_WARN("lvgl font resource[%d]:", rec_ll_len);

        font_rec_node_t* node;
        _LV_LL_READ(rec_ll, node)
        {
            LV_LOG_WARN("Font: %p(%d) -> ref: %s(%d)",
                node,
                (int)node->font.line_height,
                node->refer_node_p->ft_info.name,
                node->refer_node_p->ft_info.size);
        }
    }

    /* Check the recorded font resources created by freetype */
    lv_ll_t* refer_ll = &manager->refer_ll;
    int refer_ll_len = _lv_ll_get_len(refer_ll);
    if (refer_ll_len) {
        LV_LOG_WARN("freetype font resource[%d]:", refer_ll_len);

        font_refer_node_t* node;
        _LV_LL_READ(refer_ll, node)
        {
            LV_LOG_WARN("Font: %s(%d), ref cnt = %d",
                node->ft_info.name,
                node->ft_info.size,
                node->ref_cnt);
        }
    }

    /* Check resource leak */
    bool has_resource = (rec_ll_len || refer_ll_len);

    return has_resource;
}

static font_rec_node_t* font_manager_search_rec_node(font_manager_t* manager, lv_font_t* font)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(font);

    font_rec_node_t* rec_node;
    _LV_LL_READ(&manager->rec_ll, rec_node)
    {
        if (font == &rec_node->font) {
            LV_LOG_INFO("font: %p(%" LV_PRId32 ") matched", font, font->line_height);
            return rec_node;
        }
    }

    return NULL;
}

static font_refer_node_t* font_manager_search_refer_node(font_manager_t* manager, const lv_freetype_info_t* ft_info)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(ft_info);

    font_refer_node_t* refer_node;
    _LV_LL_READ(&manager->refer_ll, refer_node)
    {
        if (font_utils_ft_info_is_equal(ft_info, &refer_node->ft_info)) {
            LV_LOG_INFO("font: %s(%d) matched", ft_info->name, ft_info->size);
            return refer_node;
        }
    }

    return NULL;
}

static lv_font_t* font_manager_create_font_warpper(font_manager_t* manager, const lv_freetype_info_t* ft_info)
{
    lv_font_t* font = NULL;
#if UIKIT_FONT_USE_EMOJI
    /* match emoji */
    if (IS_EMOJI_NAME(ft_info->name)) {
        if (!manager->emoji_manager) {
            LV_LOG_WARN("emoji manager not ready");
            return NULL;
        }

        lv_font_t* emoji_font = font_emoji_manager_create_font(manager->emoji_manager, ft_info->name, ft_info->size);
        if (!emoji_font) {
            LV_LOG_INFO("emoji %s(%d) create failed",
                ft_info->name,
                ft_info->size);
            return NULL;
        }
        return emoji_font;
    }
#endif /* UIKIT_FONT_USE_EMOJI */

    /* create freetype font */
#if (UIKIT_FONT_CACHE_SIZE > 0)
    font = font_cache_manager_get_reuse(manager->cache_manager, ft_info);
    /* get reuse font from cache */
    if (font) {
        return font;
    }
    /* cache miss */
#endif /* UIKIT_FONT_CACHE_SIZE */
    if (!font_manager_check_font_file(manager, ft_info->name)) {
        return NULL;
    }

    /* generate full file path */
    const char* path = font_manager_get_path(manager, ft_info->name);

    font = lv_freetype_font_create(path, CONFIG_UIKIT_FONT_CREATE_TYPE, ft_info->size, ft_info->style);

    if (!font) {
        LV_LOG_ERROR("Freetype font init failed, name: %s, weight: %d, style: %d",
            ft_info->name, ft_info->size, ft_info->style);
        return NULL;
    }
    return font;
}

static void font_manager_delete_font_warpper(font_manager_t* manager, font_refer_node_t* refer_node)
{
#if UIKIT_FONT_USE_EMOJI
    if (IS_EMOJI_NAME(refer_node->ft_info.name)) {
        if (manager->emoji_manager) {
            font_emoji_manager_delete_font(manager->emoji_manager, refer_node->font_p);
        }
    } else
#endif /* UIKIT_FONT_USE_EMOJI */
    {
#if (UIKIT_FONT_CACHE_SIZE > 0)
        font_cache_manager_set_reuse(manager->cache_manager, refer_node->font_p, &refer_node->ft_info);
#else
        lv_freetype_font_delete(refer_node->font_p);
#endif /* UIKIT_FONT_CACHE_SIZE */
    }
}

static font_refer_node_t* font_manager_request_font(font_manager_t* manager, const lv_freetype_info_t* ft_info)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(ft_info);

    /* check refer_node is existed */
    font_refer_node_t* refer_node = font_manager_search_refer_node(manager, ft_info);
    if (refer_node) {
        refer_node->ref_cnt++;
        LV_LOG_INFO("refer_node existed, ref_cnt = %d", refer_node->ref_cnt);
        return refer_node;
    }

    lv_font_t* font = font_manager_create_font_warpper(manager, ft_info);
    if (!font) {
        return NULL;
    }

    /* add refer_node to refer_ll */
    refer_node = _lv_ll_ins_head(&manager->refer_ll);
    LV_ASSERT_MALLOC(refer_node);
    lv_memzero(refer_node, sizeof(font_refer_node_t));

    strncpy(refer_node->name, ft_info->name, sizeof(refer_node->name));
    refer_node->name[sizeof(refer_node->name) - 1] = '\0';

    /* copy font data */
    refer_node->font_p = font;
    refer_node->ft_info = *ft_info;
    refer_node->ft_info.name = refer_node->name;
    refer_node->ref_cnt = 1;

    LV_LOG_INFO("success");
    return refer_node;
}

static bool font_manager_drop_font(font_manager_t* manager, font_refer_node_t* refer_node)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(refer_node);

    refer_node->ref_cnt--;

    /* If ref_cnt is > 0, no need to delete font */
    if (refer_node->ref_cnt > 0) {
        LV_LOG_INFO("refer_node existed, ref_cnt = %d", refer_node->ref_cnt);
        return true;
    }

    /* if if ref_cnt is about to be 0, free font resource */
    font_manager_delete_font_warpper(manager, refer_node);
    refer_node->font_p = NULL;

    /* free refer_node */
    _lv_ll_remove(&manager->refer_ll, refer_node);
    lv_free(refer_node);

    LV_LOG_INFO("success");
    return true;
}
