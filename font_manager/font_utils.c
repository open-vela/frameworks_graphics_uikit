/**
 * @file font_utils.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "font_utils.h"
#include "font_log.h"
#include <cJSON.h>
#include <lvgl/lvgl.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#define JSON_GET_VALUE_STRING(cjson_obj, value_dest, name)          \
    do {                                                            \
        cJSON* it = cJSON_GetObjectItem((cjson_obj), name);         \
        if (!it) {                                                  \
            FONT_LOG_WARN("can't get valuestring item: " name);     \
            goto failed;                                            \
        }                                                           \
        strncpy((value_dest), it->valuestring, sizeof(value_dest)); \
        (value_dest)[sizeof(value_dest) - 1] = '\0';                \
    } while (0)

#define JSON_GET_VALUE_INT(cjson_obj, value_dest, name)      \
    do {                                                     \
        cJSON* it = cJSON_GetObjectItem((cjson_obj), name);  \
        if (!it) {                                           \
            FONT_LOG_WARN("can't get valueint item: " name); \
            goto failed;                                     \
        }                                                    \
        (value_dest) = it->valueint;                         \
    } while (0)

/* JSON item string define */

#define JSON_ITEM_STR_EMOJI_LIST "emoji-list"
#define JSON_ITEM_STR_FONT_NAME "font-name"
#define JSON_ITEM_STR_PATH "path"
#define JSON_ITEM_STR_EXT "ext"
#define JSON_ITEM_STR_MATCH_SIZE "match-size"
#define JSON_ITEM_STR_MIN "min"
#define JSON_ITEM_STR_MAX "max"
#define JSON_ITEM_STR_UNICODE_RANGE "unicode-range"
#define JSON_ITEM_STR_BEGIN "begin"
#define JSON_ITEM_STR_END "end"

#define JSON_ITEM_STR_FONT_FAMILY "font-family"
#define JSON_ITEM_STR_FALLBACK "fallback"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _font_utils_json_obj_t {
    cJSON* cjson;
    char* json_buffer;
} font_utils_json_obj_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

font_utils_json_obj_t* font_utils_json_obj_create(const char* file_path)
{
    lv_fs_file_t file;
    char* json_buf = NULL;
    cJSON* cjson = NULL;
    font_utils_json_obj_t* json_obj = NULL;

    /* open json file */
    lv_fs_res_t res = lv_fs_open(&file, file_path, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        FONT_LOG_ERROR("faild to open file: %s", file_path);
        return NULL;
    }

    /* get file size */
    lv_fs_seek(&file, 0, LV_FS_SEEK_END);
    uint32_t size;
    res = lv_fs_tell(&file, &size);
    if (res != LV_FS_RES_OK) {
        FONT_LOG_ERROR("can't get file size");
        goto failed;
    }
    lv_fs_seek(&file, 0, LV_FS_SEEK_SET);

    /* alloc string buffer */
    json_buf = lv_mem_alloc(size + 1);
    LV_ASSERT_MALLOC(json_buf);
    if (!json_buf) {
        FONT_LOG_ERROR("malloc failed for json_buf");
        goto failed;
    }

    /* read json sting */
    uint32_t br;
    res = lv_fs_read(&file, json_buf, size, &br);
    lv_fs_close(&file);
    if (res != LV_FS_RES_OK || br != size) {
        FONT_LOG_ERROR("read file failed");
        goto failed;
    }
    json_buf[size] = '\0';

    /* create json parser */
    cjson = cJSON_Parse(json_buf);
    if (!cjson) {
        FONT_LOG_ERROR("cJSON_Parse failed");
        goto failed;
    }

    /* create json_obj */
    json_obj = lv_mem_alloc(sizeof(font_utils_json_obj_t));
    LV_ASSERT_MALLOC(json_obj);
    if (!json_obj) {
        FONT_LOG_ERROR("malloc failed for font_utils_json_obj_t");
        goto failed;
    }
    lv_memset_00(json_obj, sizeof(font_utils_json_obj_t));

    json_obj->json_buffer = json_buf;
    json_obj->cjson = cjson;

    return json_obj;

failed:
    lv_fs_close(&file);

    if (json_buf) {
        lv_mem_free(json_buf);
    }

    if (cjson) {
        cJSON_Delete(cjson);
    }

    if (json_obj) {
        lv_mem_free(json_obj);
    }

    return NULL;
}

void font_utils_json_obj_delete(font_utils_json_obj_t* json_obj)
{
    LV_ASSERT_NULL(json_obj);

    if (json_obj->cjson) {
        cJSON_Delete(json_obj->cjson);
        json_obj->cjson = NULL;
    }

    if (json_obj->json_buffer) {
        lv_mem_free(json_obj->json_buffer);
        json_obj->json_buffer = NULL;
    }

    lv_mem_free(json_obj);
}

font_emoji_config_t* font_utils_json_get_emoji_config(font_utils_json_obj_t* json_obj)
{
    LV_ASSERT_NULL(json_obj);
    cJSON* cjson = json_obj->cjson;

    cJSON* emoji_list_arr = cJSON_GetObjectItem(cjson, JSON_ITEM_STR_EMOJI_LIST);
    int emoji_list_arr_size = cJSON_GetArraySize(emoji_list_arr);

    /* check array */
    if (!emoji_list_arr_size) {
        FONT_LOG_WARN(JSON_ITEM_STR_EMOJI_LIST " is empty");
        return NULL;
    }

    /* create font_emoji_config */
    font_emoji_config_t* config = lv_mem_alloc(sizeof(font_emoji_config_t));
    LV_ASSERT_MALLOC(config);
    if (!config) {
        FONT_LOG_ERROR("malloc failed for font_emoji_config_t");
        return NULL;
    }
    lv_memset_00(config, sizeof(font_emoji_config_t));

    /* create emoji_arr */
    config->emoji_arr = lv_mem_alloc(sizeof(font_emoji_t) * emoji_list_arr_size);
    LV_ASSERT_MALLOC(config->emoji_arr);
    if (!config->emoji_arr) {
        FONT_LOG_ERROR("malloc failed for config->emoji_arr");
        goto failed;
    }
    lv_memset_00(config->emoji_arr, sizeof(font_emoji_t) * emoji_list_arr_size);
    config->emoji_arr_size = emoji_list_arr_size;

    /* set item */
    for (int i = 0; i < emoji_list_arr_size; i++) {
        cJSON* item = cJSON_GetArrayItem(emoji_list_arr, i);
        font_emoji_t* emoji = &config->emoji_arr[i];

        JSON_GET_VALUE_STRING(item, emoji->font_name, JSON_ITEM_STR_FONT_NAME);

        JSON_GET_VALUE_STRING(item, emoji->path, JSON_ITEM_STR_PATH);
        emoji->path_len = strlen(emoji->path);

        JSON_GET_VALUE_STRING(item, emoji->ext, JSON_ITEM_STR_EXT);
        emoji->ext_len = strlen(emoji->ext);

        cJSON* match_size = cJSON_GetObjectItem(item, JSON_ITEM_STR_MATCH_SIZE);
        JSON_GET_VALUE_INT(match_size, emoji->match_size.min, JSON_ITEM_STR_MIN);
        JSON_GET_VALUE_INT(match_size, emoji->match_size.max, JSON_ITEM_STR_MAX);

        /* check match_size */
        if (emoji->match_size.min > emoji->match_size.max) {
            FONT_LOG_WARN("match_size.min(%d) > match_size.max(%d)",
                emoji->match_size.min, emoji->match_size.max);
        }

        cJSON* unicode_range = cJSON_GetObjectItem(item, JSON_ITEM_STR_UNICODE_RANGE);
        JSON_GET_VALUE_INT(unicode_range, emoji->unicode_range.begin, JSON_ITEM_STR_BEGIN);
        JSON_GET_VALUE_INT(unicode_range, emoji->unicode_range.end, JSON_ITEM_STR_END);

        /* check unicode_range */
        if (emoji->unicode_range.begin > emoji->unicode_range.end) {
            FONT_LOG_WARN("unicode_range.begin(%" LV_PRIu32 ") > unicode_range.end(%" LV_PRIu32 ")",
                emoji->unicode_range.begin, emoji->unicode_range.end);
        }
    }

    return config;

failed:
    if (config) {
        font_utils_json_emoji_config_free(config);
    }
    return NULL;
}

font_family_config_t* font_utils_json_get_font_family_config(font_utils_json_obj_t* json_obj)
{
    LV_ASSERT_NULL(json_obj);
    cJSON* cjson = json_obj->cjson;

    cJSON* font_family_arr = cJSON_GetObjectItem(cjson, JSON_ITEM_STR_FONT_FAMILY);
    int font_family_arr_size = cJSON_GetArraySize(font_family_arr);

    /* check array */
    if (!font_family_arr_size) {
        FONT_LOG_WARN("font-family is empty");
        return NULL;
    }

    /* create font_family_config */
    font_family_config_t* config = lv_mem_alloc(sizeof(font_family_config_t));
    LV_ASSERT_MALLOC(config);
    if (!config) {
        FONT_LOG_ERROR("malloc failed for font_family_config_t");
        return NULL;
    }
    lv_memset_00(config, sizeof(font_family_config_t));

    /* create font_family_arr */
    config->font_family_arr = lv_mem_alloc(sizeof(font_family_t) * font_family_arr_size);
    LV_ASSERT_MALLOC(config->font_family_arr);
    if (!config->font_family_arr) {
        FONT_LOG_ERROR("malloc failed for config->font_family_arr");
        goto failed;
    }
    lv_memset_00(config->font_family_arr, sizeof(font_family_t) * font_family_arr_size);
    config->font_family_arr_size = font_family_arr_size;

    /* set item */
    for (int font_index = 0; font_index < font_family_arr_size; font_index++) {
        cJSON* item = cJSON_GetArrayItem(font_family_arr, font_index);
        if (!item) {
            FONT_LOG_ERROR("can't get font_family_arr item[%d]", font_index);
            goto failed;
        }
        font_family_t* font_family = &config->font_family_arr[font_index];

        JSON_GET_VALUE_STRING(item, font_family->font_name, JSON_ITEM_STR_FONT_NAME);

        cJSON* fallback_arr = cJSON_GetObjectItem(item, JSON_ITEM_STR_FALLBACK);
        int fallback_arr_size = cJSON_GetArraySize(fallback_arr);

        /* check fallback array */
        if (!fallback_arr_size) {
            FONT_LOG_ERROR(JSON_ITEM_STR_FALLBACK " is empty");
            goto failed;
        }

        /* create fallback_arr */
        font_family->fallback_arr = lv_mem_alloc(sizeof(font_family_fallback_t) * fallback_arr_size);
        LV_ASSERT_MALLOC(font_family->fallback_arr);
        if (!font_family->fallback_arr) {
            FONT_LOG_ERROR("malloc failed for font_family->fallback_arr");
            goto failed;
        }
        lv_memset_00(font_family->fallback_arr, sizeof(font_family_fallback_t) * fallback_arr_size);
        font_family->fallback_arr_size = fallback_arr_size;

        /* set fallback */
        for (int fallback_index = 0; fallback_index < fallback_arr_size; fallback_index++) {
            cJSON* fallback_item = cJSON_GetArrayItem(fallback_arr, fallback_index);
            if (!fallback_item) {
                FONT_LOG_ERROR("can't get fallback_item [%d]", fallback_index);
                goto failed;
            }

            font_family_fallback_t* fallback = &font_family->fallback_arr[fallback_index];
            strncpy(fallback->font_name, fallback_item->valuestring, sizeof(fallback->font_name));
            fallback->font_name[sizeof(fallback->font_name) - 1] = '\0';
        }
    }

    return config;

failed:
    if (config) {
        font_utils_json_font_family_free(config);
    }
    return NULL;
}

void font_utils_json_emoji_config_free(font_emoji_config_t* config)
{
    LV_ASSERT_NULL(config);
    if (config->emoji_arr) {
        lv_mem_free(config->emoji_arr);
        config->emoji_arr = NULL;
    }
    lv_mem_free(config);
}

void font_utils_json_font_family_free(font_family_config_t* config)
{
    LV_ASSERT_NULL(config);
    if (config->font_family_arr) {
        for (int i = 0; i < config->font_family_arr_size; i++) {
            font_family_t* font_family = &config->font_family_arr[i];

            if (font_family->fallback_arr) {
                lv_mem_free(font_family->fallback_arr);
                font_family->fallback_arr = NULL;
            }
        }
        lv_mem_free(config->font_family_arr);
        config->font_family_arr = NULL;
    }
    lv_mem_free(config);
}

bool font_utils_ft_info_is_equal(const lv_ft_info_t* ft_info_1, const lv_ft_info_t* ft_info_2)
{
    LV_ASSERT_NULL(ft_info_1);
    LV_ASSERT_NULL(ft_info_2);

    bool is_equal = (ft_info_1->weight == ft_info_2->weight
        && ft_info_1->style == ft_info_2->style
        && strcmp(ft_info_1->name, ft_info_2->name) == 0);

    return is_equal;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
