/**
 * @file font_emoji.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "font_emoji.h"
#include <limits.h>
#include <sys/types.h>

#if FONT_USE_EMOJI

#include "font_log.h"
#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _font_emoji_manager_t {
    font_emoji_config_t* config;
} font_emoji_manager_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static bool generate_path(font_emoji_t* emoji, uint32_t unicode, char* path,
    uint16_t len);
static const void* get_imgfont_path(const lv_font_t* font, uint32_t unicode,
    uint32_t unicode_next, lv_coord_t* offset_y,
    void* user_data);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

font_emoji_manager_t*
font_emoji_manager_create(font_utils_json_obj_t* json_obj)
{
    LV_ASSERT_NULL(json_obj);

    font_emoji_config_t* config = font_utils_json_get_emoji_config(json_obj);
    if (!config) {
        FONT_LOG_WARN("emoji config parse failed");
        return NULL;
    }

    font_emoji_manager_t* manager = lv_malloc(sizeof(font_emoji_manager_t));
    LV_ASSERT_MALLOC(manager);
    if (!manager) {
        font_utils_json_emoji_config_free(config);
        FONT_LOG_ERROR("malloc failed for font_emoji_manager_t");
        return NULL;
    }
    lv_memzero(manager, sizeof(font_emoji_manager_t));
    manager->config = config;

    FONT_LOG_INFO("success");
    return manager;
}

void font_emoji_manager_delete(font_emoji_manager_t* manager)
{
    LV_ASSERT_NULL(manager);

    font_utils_json_emoji_config_free(manager->config);
    lv_free(manager);
}

lv_font_t* font_emoji_manager_create_font(font_emoji_manager_t* manager,
    const char* name, uint16_t height)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(name);

    font_emoji_config_t* config = manager->config;

    for (int i = 0; i < config->emoji_arr_size; i++) {
        font_emoji_t* emoji = &config->emoji_arr[i];

        /* match name and height */
        if (strcmp(name, emoji->font_name) == 0 && height >= emoji->match_size.min && height <= emoji->match_size.max) {
            lv_font_t* imgfont = lv_imgfont_create(height, get_imgfont_path, NULL);
            LV_ASSERT_NULL(imgfont);
            if (!imgfont) {
                FONT_LOG_ERROR("emoji create failed");
                return NULL;
            }

            imgfont->user_data = emoji;

            FONT_LOG_INFO("emoji: %s(%d) create OK", name, height);

            return imgfont;
        }
    }
    FONT_LOG_INFO("No emoji match: %s(%d)", name, height);

    return NULL;
}

void font_emoji_manager_delete_font(font_emoji_manager_t* manager,
    lv_font_t* font)
{
    LV_ASSERT_NULL(manager);
    LV_ASSERT_NULL(font);

    lv_imgfont_destroy(font);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static bool generate_path(font_emoji_t* emoji, uint32_t unicode, char* path,
    uint16_t len)
{
    /* Using the following method is more than
     * 10 times faster than the 'snprintf' method
     */

    /* int32_t range: -2147483648~2147483647 */
    const int max_size = emoji->path_len + emoji->ext_len + 16;
    if (len < max_size) {
        FONT_LOG_ERROR("buf len(%d) < max_size(%d), "
                       "not enough buffers to generate path string",
            len, max_size);
        return false;
    }

    /* Copy base path, no terminator required */
    lv_memcpy(path, emoji->path, emoji->path_len);
    path += emoji->path_len;

    /* Generate number string (unicode-range: 0 ~ 0x10FFFF) */
    itoa(unicode, path, 10);
    path += strlen(path);

    /* Copy the extension, including the terminator */
    lv_memcpy(path, emoji->ext, emoji->ext_len + 1);

    return true;
}

static const void* get_imgfont_path(const lv_font_t* font, uint32_t unicode,
    uint32_t unicode_next, lv_coord_t* offset_y,
    void* user_data)
{
    LV_UNUSED(unicode_next);

    font_emoji_t* emoji = font->user_data;
    LV_ASSERT_NULL(emoji);

    if (unicode >= emoji->unicode_range.begin && unicode <= emoji->unicode_range.end) {
        static char path[PATH_MAX];
        generate_path(emoji, unicode, path, sizeof(path));
        return path;
    }

    return NULL;
}

#endif /* FONT_USE_EMOJI */
