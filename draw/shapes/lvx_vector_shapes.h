/**
 * @file lvx_vector_shapes.h
 *
 */

#ifndef LVX_VECTOR_SHAPES_H
#define LVX_VECTOR_SHAPES_H

#include <lvgl/lvgl.h>

#if LV_USE_VECTOR_GRAPHIC

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    LVX_VECTOR_SHAPE_CLOVER,
    LVX_VECTOR_SHAPE_HEART,
} lvx_vector_shape_type_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a path from complex shape define
 * @param type  The shape type.
 * @param path  The path.
 * @return return true if success.
 */
bool lvx_vector_shapes_factory(lvx_vector_shape_type_t type, lv_vector_path_t * path);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_USE_VECTOR_GRAPHIC*/
#endif /* LVX_VECTOR_SHAPES_H */
