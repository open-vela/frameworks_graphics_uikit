/**
 * @file uikit_vtun.h
 *
 */

#ifndef UIKIT_VTUN_H
#define UIKIT_VTUN_H

#define VTUN_FRAME_PLANE_NUM 3

typedef enum {
    VTUN_FRAME_FORMAT_BGRA8888,
    VTUN_FRAME_FORMAT_RGB565,
    VTUN_FRAME_FORMAT_NV12,
    VTUN_FRAME_FORMAT_INVALID
} vg_vtun_frame_format;

typedef enum {
    VTUN_CTRL_EVT_NONE,
    VTUN_CTRL_EVT_FRAME_REQ,
    VTUN_CTRL_EVT_PLAY,
    VTUN_CTRL_EVT_STOP,
} vg_vtun_ctrl_evt_type;

typedef struct {
    size_t x1;
    size_t x2;
    size_t y1;
    size_t y2;
} vg_vtun_crop_info;

typedef struct {
    void* addr;
    int stride;
} vg_vtun_plane_info;
typedef struct {
    vg_vtun_frame_format format;
    vg_vtun_crop_info crop_info;
    vg_vtun_plane_info plane[VTUN_FRAME_PLANE_NUM];
    unsigned current_ms;
    int w;
    int h;
} vg_vtun_frame;

#endif
