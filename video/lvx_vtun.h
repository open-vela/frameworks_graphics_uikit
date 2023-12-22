/**
 * @file lvx_vtun.h
 *
 */

#ifndef LVX_VTUN_H
#define LVX_VTUN_H

#define VTUN_FRAME_PLANE_NUM 3

typedef enum {
    VTUN_FRAME_FORMAT_BGRA8888,
    VTUN_FRAME_FORMAT_RGB565,
    VTUN_FRAME_FORMAT_NV12,
    VTUN_FRAME_FORMAT_INVALID
} lvx_vtun_frame_format;

typedef enum {
    VTUN_CTRL_EVT_NONE,
    VTUN_CTRL_EVT_FRAME_REQ,
    VTUN_CTRL_EVT_PLAY,
    VTUN_CTRL_EVT_STOP,
} lvx_vtun_ctrl_evt_type;

typedef struct {
    size_t x1;
    size_t x2;
    size_t y1;
    size_t y2;
} lvx_vtun_crop_info;

typedef struct {
    void * addr;
    int stride;
} lvx_vtun_plane_info;
typedef struct {
    lvx_vtun_frame_format format;
    lvx_vtun_crop_info crop_info;
    lvx_vtun_plane_info plane[VTUN_FRAME_PLANE_NUM];
    unsigned current_ms;
    int w;
    int h;
} lvx_vtun_frame;

#endif
