/**
 * @file font_stress_demo.h
 *
 */

#ifndef UIKIT_DEMO_FONT_STRESS_H
#define UIKIT_DEMO_FONT_STRESS_H

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
} uikit_demo_font_stress_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void uikit_demo_font_stress_config_init(uikit_demo_font_stress_config_t* config);

void uikit_demo_font_stress(const uikit_demo_font_stress_config_t* config);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEMPL_H*/
