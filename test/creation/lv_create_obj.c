/**
 * @file lv_create_obj.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_create_obj.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    const char* name;
    lv_obj_t* (*create_func)(lv_obj_t*);
} Test_Case;

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline void get_used_time(lv_obj_t* parent, const char* obj, lv_obj_t* (*create_func)(lv_obj_t*))
{
    uint32_t tv_start, tv_used;

    lv_obj_t* obj_parent = lv_obj_create(parent);

    tv_start = lv_tick_get();
    for (int i = 0; i < 1000; i++) {
        create_func(obj_parent);
    }
    tv_used = lv_tick_elaps(tv_start);

    lv_obj_delete(obj_parent);

    LV_LOG("%s create_time: %.4f ms\n", obj, tv_used / 1000.0);
}

/****************************************************************************
 * lvx_time_obj_creation
 ****************************************************************************/

void lvx_time_obj_creation(char* info[], int size, void* param)
{

    LV_UNUSED(info);
    LV_UNUSED(size);
    LV_UNUSED(param);

    lv_obj_t* parent = lv_screen_active();

    Test_Case test_cases[] = {
        { "lv_obj_create", lv_obj_create },
        { "lv_image_create", lv_image_create },
        { "lv_spangroup_create", lv_spangroup_create },
        { "lv_slider_create", lv_slider_create },
        { "lv_chart_create", lv_chart_create },
        { "lv_switch_create", lv_switch_create },
        { "lv_qrcode_create", lv_qrcode_create },
    };

    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; i++) {
        get_used_time(parent, test_cases[i].name, test_cases[i].create_func);
    }

}
