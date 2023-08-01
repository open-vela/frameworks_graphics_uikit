/**
 * @file lvx_vtun.h
 *
 */

#ifndef LVX_VTUN_H
#define LVX_VTUN_H

typedef enum {
    VTUN_FRAME_FORMAT_BGRA8888,
    VTUN_FRAME_FORMAT_YUV420SP,
    VTUN_FRAME_FORMAT_INVALID
} lvx_vtun_frame_format;

typedef enum {
    VTUN_CTRL_EVT_NONE,
    VTUN_CTRL_EVT_FRAME_REQ,
    VTUN_CTRL_EVT_PLAY,
    VTUN_CTRL_EVT_STOP,
} lvx_vtun_ctrl_evt_type;

typedef struct {
    lvx_vtun_frame_format format;
    unsigned current_ms;
    size_t size;
    void* addr;
    int w;
    int h;
} lvx_vtun_frame;

#endif
