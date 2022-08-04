/**
 * @file font_cache.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "font_cache.h"
#include "font_log.h"
#include "font_utils.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_ft_info_t ft_info;
    char name[FONT_NAME_MAX];
} font_cache_t;

typedef struct _font_cache_manager_t {
    lv_ll_t cache_ll;
    uint32_t max_size;
} font_cache_manager_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void font_cache_close(font_cache_manager_t* manager, font_cache_t* cache);
static void font_cache_manager_remove_tail(font_cache_manager_t* manager);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

font_cache_manager_t* font_cache_manager_create(uint32_t max_size)
{
    font_cache_manager_t* manager = lv_mem_alloc(sizeof(font_cache_manager_t));
    LV_ASSERT_MALLOC(manager);
    if (!manager) {
        FONT_LOG_ERROR("malloc failed for font_cache_manager_t");
        return NULL;
    }
    lv_memset_00(manager, sizeof(font_cache_manager_t));

    _lv_ll_init(&manager->cache_ll, sizeof(font_cache_t));
    manager->max_size = max_size;

    FONT_LOG_INFO("success");
    return manager;
}

void font_cache_manager_delete(font_cache_manager_t* manager)
{
    LV_ASSERT_NULL(manager);

    lv_ll_t* cache_ll = &manager->cache_ll;

    font_cache_t* cache = _lv_ll_get_head(cache_ll);

    /* clear all cache */
    while (cache != NULL) {
        font_cache_t* cache_next = _lv_ll_get_next(cache_ll, cache);
        font_cache_close(manager, cache);
        cache = cache_next;
    }

    lv_mem_free(manager);

    FONT_LOG_INFO("success");
}

bool font_cache_manager_get_reuse(font_cache_manager_t* manager, lv_ft_info_t* ft_info)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(ft_info);

    lv_ll_t* cache_ll = &manager->cache_ll;

    FONT_LOG_INFO("font: %s(%d) searching...", ft_info->name, ft_info->weight);

    font_cache_t* cache;
    _LV_LL_READ(cache_ll, cache)
    {
        /* match font */
        if (font_utils_ft_info_is_equal(ft_info, &cache->ft_info)) {
            ft_info->font = cache->ft_info.font;
            FONT_LOG_INFO("cache hit");

            /* remove reused cache */
            _lv_ll_remove(cache_ll, cache);
            lv_mem_free(cache);
            return true;
        }
    }

    FONT_LOG_INFO("cache miss");

    return false;
}

void font_cache_manager_set_reuse(font_cache_manager_t* manager, const lv_ft_info_t* ft_info)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(ft_info);

    lv_ll_t* cache_ll = &manager->cache_ll;

    /* check cached size */
    if (_lv_ll_get_len(cache_ll) >= manager->max_size) {
        FONT_LOG_INFO("cache full, remove tail cache...");
        font_cache_manager_remove_tail(manager);
    }

    /* record reuse font */
    font_cache_t* cache = _lv_ll_ins_head(cache_ll);
    LV_ASSERT_MALLOC(cache);
    lv_memset_00(cache, sizeof(font_cache_t));

    strncpy(cache->name, ft_info->name, sizeof(cache->name));
    cache->name[sizeof(cache->name) - 1] = '\0';

    cache->ft_info = *ft_info;
    cache->ft_info.name = cache->name;

    FONT_LOG_INFO("insert font: %s(%d) to reuse list", ft_info->name, ft_info->weight);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void font_cache_close(font_cache_manager_t* manager, font_cache_t* cache)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(cache);

    FONT_LOG_INFO("font: %s(%d) close", cache->ft_info.name, cache->ft_info.weight);
    lv_ft_font_destroy(cache->ft_info.font);

    _lv_ll_remove(&manager->cache_ll, cache);
    lv_mem_free(cache);
}

static void font_cache_manager_remove_tail(font_cache_manager_t* manager)
{
    font_cache_t* tail = _lv_ll_get_tail(&manager->cache_ll);
    LV_ASSERT_NULL(tail);
    font_cache_close(manager, tail);
}
