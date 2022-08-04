/**
 * @file font_utils.h
 *
 */
#ifndef FONT_MANAGER_FONT_UTILS_H
#define FONT_MANAGER_FONT_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "font_config.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _font_utils_json_obj_t font_utils_json_obj_t;

typedef struct {
    char font_name[FONT_NAME_MAX];
    char ext[FONT_EXT_MAX];
    char path[FONT_PATH_MAX];

    uint16_t path_len;
    uint16_t ext_len;

    struct
    {
        uint16_t min;
        uint16_t max;
    } match_size;
    struct
    {
        uint32_t begin;
        uint32_t end;
    } unicode_range;
} font_emoji_t;

typedef struct {
    font_emoji_t* emoji_arr;
    int emoji_arr_size;
} font_emoji_config_t;

typedef struct {
    char font_name[FONT_NAME_MAX];
} font_family_fallback_t;

typedef struct {
    char font_name[FONT_NAME_MAX];
    font_family_fallback_t* fallback_arr;
    int fallback_arr_size;
} font_family_t;

typedef struct {
    font_family_t* font_family_arr;
    int font_family_arr_size;
} font_family_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a json parsing object.
 * @param file_path json file path.
 * @return pointer to json parsing object.
 */
font_utils_json_obj_t* font_utils_json_obj_create(const char* file_path);

/**
 * Delete a json parsing object.
 * @param json_obj pointer to json parsing object.
 */
void font_utils_json_obj_delete(font_utils_json_obj_t* json_obj);

/**
 * Get the parsed emoji configuration data structure.
 * @param json_obj pointer to json parsing object.
 * @return pointer to emoji configuration data structure.
 */
font_emoji_config_t* font_utils_json_get_emoji_config(font_utils_json_obj_t* json_obj);

/**
 * Get the parsed font-family configuration data structure.
 * @param json_obj pointer to json parsing object.
 * @return pointer to font-family configuration data structure.
 */
font_family_config_t* font_utils_json_get_font_family_config(font_utils_json_obj_t* json_obj);

/**
 * Free the parsed emoji configuration data structure.
 * @param config pointer to emoji configuration data structure.
 */
void font_utils_json_emoji_config_free(font_emoji_config_t* config);

/**
 * Free the parsed font-family configuration data structure.
 * @param config pointer to font-family configuration data structure.
 */
void font_utils_json_font_family_free(font_family_config_t* config);

/**
 * Compare font information.
 * @param ft_info_1 font information 1.
 * @param ft_info_2 font information 2.
 * @return return true if the fonts are equal.
 */
bool font_utils_ft_info_is_equal(const lv_ft_info_t* ft_info_1, const lv_ft_info_t* ft_info_2);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* FONT_MANAGER_FONT_UTILS_H */