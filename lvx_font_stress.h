/**
 * @file lvx_font_stress.h
 *
 */

#ifndef LVX_FONT_STRESS_H
#define LVX_FONT_STRESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char** font_name_arr;
    int font_cnt;
    int label_cnt;
    int loop_cnt;
} lvx_font_stress_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lvx_font_stres_config_init(lvx_font_stress_config_t* config);

void lvx_font_stres_test(const lvx_font_stress_config_t* config);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEMPL_H*/
