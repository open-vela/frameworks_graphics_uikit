/**
 * @file lvx_vector_clipper.h
 *
 */

#ifndef LVX_VECTOR_CLIPPER_H
#define LVX_VECTOR_CLIPPER_H

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
    LVX_VECTOR_CLIPPER_INTERSECT,
    LVX_VECTOR_CLIPPER_UNION,
    LVX_VECTOR_CLIPPER_DIFF,
    LVX_VECTOR_CLIPPER_XOR,
} lvx_vector_clipper_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Flatten a path to a polygon path.
 * @param result  The polygon path.
 * @param path    The input path.
 * @return return true if success.
 */
bool lvx_vector_flatten_path_to_polygon(lv_vector_path_t * result, const lv_vector_path_t * path);

/**
 * Clip two polygon path with gpc.
 * @param type  The clip type.
 * @param result_path  The clip result path.
 * @param path1  The first path.
 * @param path2  The second path.
 * @return return true if success.
 */
bool lvx_vector_polygon_clipper(lvx_vector_clipper_t type, lv_vector_path_t * result_path,
                                const lv_vector_path_t * path1, const lv_vector_path_t * path2);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_USE_VECTOR_GRAPHIC*/
#endif /* LVX_VECTOR_CLIPPER_H */
