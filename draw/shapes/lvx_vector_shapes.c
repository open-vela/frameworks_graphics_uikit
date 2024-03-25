
/**
 * @file lvx_vector_shapes.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_vector_shapes.h"

#if LV_USE_VECTOR_GRAPHIC

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
static bool _create_shape_heart(lv_vector_path_t * path)
{
    lv_vector_path_clear(path);

    lv_fpoint_t ps;
    lv_fpoint_t pt[3];

    ps.x = -205.020f;
    ps.y = 22.224;
    lv_vector_path_move_to(path, &ps);

    pt[0].x = ps.x - 54.903f;
    pt[0].y = ps.y - 54.903f;
    pt[1].x = ps.x - 54.903f;
    pt[1].y = ps.y - 144.222f;
    pt[2].x = ps.x;
    pt[2].y = ps.y - 199.125f;
    ps.x = pt[2].x;
    ps.y = pt[2].y;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);

    pt[0].x = ps.x + 26.59f;
    pt[0].y = ps.y - 26.59f;
    pt[1].x = ps.x + 61.934f;
    pt[1].y = ps.y - 41.233f;
    pt[2].x = ps.x + 99.558f;
    pt[2].y = ps.y - 41.233f;
    ps.x = pt[2].x;
    ps.y = pt[2].y;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);

    pt[0].x = ps.x + 33.502f;
    pt[0].y = ps.y;
    pt[1].x = ps.x + 66.21f;
    pt[1].y = ps.y + 12.442f;
    pt[2].x = ps.x + 92.109f;
    pt[2].y = ps.y + 35.038f;
    ps.x = pt[2].x;
    ps.y = pt[2].y;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);

    pt[0].x = ps.x + 10.291f;
    pt[0].y = ps.y + 9.617f;
    ps.x = pt[0].x;
    ps.y = pt[0].y;
    lv_vector_path_line_to(path, &pt[0]);

    pt[0].x = ps.x + 9.967f;
    pt[0].y = ps.y - 9.318f;
    ps.x = pt[0].x;
    ps.y = pt[0].y;
    lv_vector_path_line_to(path, &pt[0]);

    pt[0].x = ps.x + 26.223f;
    pt[0].y = ps.y - 22.895f;
    pt[1].x = ps.x + 58.931f;
    pt[1].y = ps.y - 35.333f;
    pt[2].x = ps.x + 92.433f;
    pt[2].y = ps.y - 35.337f;
    ps.x = pt[2].x;
    ps.y = pt[2].y;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);

    pt[0].x = ps.x + 37.615f;
    pt[0].y = ps.y;
    pt[1].x = ps.x + 72.969f;
    pt[1].y = ps.y + 14.643f;
    pt[2].x = ps.x + 99.588f;
    pt[2].y = ps.y + 41.233f;
    ps.x = pt[2].x;
    ps.y = pt[2].y;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);

    pt[0].x = ps.x + 54.903f;
    pt[0].y = ps.y + 54.903f;
    pt[1].x = ps.x + 54.903f;
    pt[1].y = ps.y + 144.222f;
    pt[2].x = ps.x;
    pt[2].y = ps.y + 199.125f;
    ps.x = pt[2].x;
    ps.y = pt[2].y;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);

    pt[0].x = 0;
    pt[0].y = 224.253f;
    lv_vector_path_line_to(path, &pt[0]);

    pt[0].x = -205.020f;
    pt[0].y = 22.224f;
    lv_vector_path_line_to(path, &pt[0]);
    lv_vector_path_close(path);

    return true;
}

static bool _create_shape_clover(lv_vector_path_t * path)
{
    lv_vector_path_clear(path);

    lv_fpoint_t ps;
    lv_fpoint_t pt[3];

    ps.x = -20;
    ps.y = 20;
    lv_vector_path_move_to(path, &ps);
    pt[0].x = -200;
    pt[0].y = 170;
    pt[1].x = -200;
    pt[1].y = -170;
    pt[2].x = -20;
    pt[2].y = -20;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);
    pt[0].x = -170;
    pt[0].y = -200;
    pt[1].x = 170;
    pt[1].y = -200;
    pt[2].x = 20;
    pt[2].y = -20;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);
    pt[0].x = 200;
    pt[0].y = -170;
    pt[1].x = 200;
    pt[1].y = 170;
    pt[2].x = 20;
    pt[2].y = 20;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);
    pt[0].x = 170;
    pt[0].y = 200;
    pt[1].x = -170;
    pt[1].y = 200;
    pt[2].x = -20;
    pt[2].y = 20;
    lv_vector_path_cubic_to(path, &pt[0], &pt[1], &pt[2]);
    lv_vector_path_close(path);
    return true;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

bool lvx_vector_shapes_factory(lvx_vector_shape_type_t type, lv_vector_path_t * path)
{
    if(!path) {
        LV_LOG_ERROR("%s : path is invalid!", __func__);
        return false;
    }

    switch(type) {
        case LVX_VECTOR_SHAPE_CLOVER:
            return _create_shape_clover(path);
        case LVX_VECTOR_SHAPE_HEART:
            return _create_shape_heart(path);
        /*TODO: add more shapes */
        default:
            break;
    }
    LV_LOG_WARN("%s : shape type is not support!", __func__);
    return false;
}

#endif
