/**
 * @file uikit_vector_shapes.h
 *
 */

#ifndef UIKIT_VECTOR_SHAPES_H
#define UIKIT_VECTOR_SHAPES_H

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
    VG_VECTOR_SHAPE_CLOVER,
    VG_VECTOR_SHAPE_HEART,
} vg_vector_shape_type_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a path from complex shape define
 * @param type  The shape type.
 * @param path  The path.
 * @return return true if success.
 */
bool vg_vector_shapes_factory(vg_vector_shape_type_t type, lv_vector_path_t* path);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_USE_VECTOR_GRAPHIC*/
#endif /* UIKIT_VECTOR_SHAPES_H */
