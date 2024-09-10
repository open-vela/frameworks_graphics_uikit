/****************************************************************************
 * frameworks/graphics/uikit/video/video_adapter.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "video_adapter.h"
#include "uikit/uikit.h"

#include <cJSON.h>
#ifdef CONFIG_NET_RPMSG
#include <netpacket/rpmsg.h>
#endif /* CONFIG_NET_RPMSG */
#include "media_player.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#ifdef CONFIG_UIKIT_VIDEO_ADAPTER

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define VIDEO_GET_FRAME_TIMEOUT 100

#ifdef CONFIG_NET_RPMSG
#define RPMSG_SERVER_MAX_LEN 128
#endif

#define CAMERA_SRC_HEADER "Camera"

#define VIDEO_SRC_HEADER "Video"

#define VTUN_HEADER "Vtun_"

/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/

struct vg_video_format_s {
    lv_color_format_t img_cf;
    vg_vtun_frame_format video_cf;
};

struct vg_video_ctx_config_s {
    char* vtun_name;
};

struct vg_video_ctx_s {
    int fd;
    void* handle;
    lv_yuv_buf_t yuv;
    struct vg_video_ctx_config_s cfg;
    void* ui_obj;
    void (*started_cb)(void* obj);
    void (*prepared_cb)(void* obj);
    void (*paused_cb)(void* obj);
    void (*completed_cb)(void* obj);
    void (*stopped_cb)(void* obj);
};

struct vg_video_ctx_map_s {
    struct vg_video_ctx_s* ctx;
    int count;
};

struct vg_video_adapter_ctx_s {
    vg_video_vtable_t vtable;
    struct vg_video_ctx_map_s map;
    void* ui_uv_loop;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct vg_video_format_s g_video_format_map[] = {
#if LV_COLOR_DEPTH == 32
    { LV_COLOR_FORMAT_NATIVE_WITH_ALPHA, VTUN_FRAME_FORMAT_BGRA8888 },
#endif
#if LV_COLOR_DEPTH == 16
    { LV_COLOR_FORMAT_NATIVE, VTUN_FRAME_FORMAT_RGB565 },
#endif
    { LV_COLOR_FORMAT_NV12, VTUN_FRAME_FORMAT_NV12 }
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: strstart
 ****************************************************************************/

static int strstart(const char* str, const char* pfx,
    const char** ptr)
{
    while (*pfx && *pfx == *str) {
        pfx++;
        str++;
    }

    if (!*pfx && ptr) {
        *ptr = str;
    }

    return !*pfx;
}

/****************************************************************************
 * Name: vg_video_config_item_parse
 ****************************************************************************/

static int vg_video_config_item_parse(cJSON* cjson,
    struct vg_video_ctx_s* ctx)
{
    if (!cJSON_IsString(cjson)) {
        LV_LOG_ERROR("parse video tunnel config error.");
        return -EINVAL;
    }

    ctx->cfg.vtun_name = strdup(cjson->valuestring);

    return 0;
}

/****************************************************************************
 * Name: vg_video_config_parse
 ****************************************************************************/

static int vg_video_config_parse(cJSON* cjson,
    struct vg_video_ctx_map_s* map)
{
    cJSON* ele = NULL;
    int i = 0;
    int ret = 0;

    map->count = cJSON_GetArraySize(cjson);

    if (map->count <= 0) {
        return -EINVAL;
    }

    map->ctx = lv_malloc(map->count * sizeof(struct vg_video_ctx_s));
    LV_ASSERT_MALLOC(map->ctx);

    if (!map->ctx) {
        LV_LOG_ERROR("malloc failed for map->ctx %d", errno);
        return -errno;
    }

    lv_memset(map->ctx, 0, map->count * sizeof(struct vg_video_ctx_s));

    cJSON_ArrayForEach(ele, cjson)
    {
        if ((ret = vg_video_config_item_parse(ele, &map->ctx[i])) < 0) {
            LV_LOG_ERROR("vg_video_config_item_parse (%d) error!", i);
            break;
        }

        i++;
    }

    return ret;
}

/****************************************************************************
 * Name: vg_video_config_init
 ****************************************************************************/

static int vg_video_config_init(const char* file_path,
    struct vg_video_ctx_map_s* map)
{
    int ret = -1;
    lv_fs_file_t file;
    uint32_t size;
    uint32_t br;
    char* json_buf = NULL;
    cJSON* cjson = NULL;

    /* open json file */

    lv_fs_res_t res = lv_fs_open(&file, file_path, LV_FS_MODE_RD);

    if (res != LV_FS_RES_OK) {
        LV_LOG_ERROR("faild to open file: %s", file_path);
        return -1;
    }

    /* get file size */

    lv_fs_seek(&file, 0, LV_FS_SEEK_END);
    res = lv_fs_tell(&file, &size);

    if (res != LV_FS_RES_OK) {
        LV_LOG_ERROR("can't get file size");
        goto failed;
    }

    lv_fs_seek(&file, 0, LV_FS_SEEK_SET);

    /* alloc string buffer */

    json_buf = lv_malloc(size + 1);
    LV_ASSERT_MALLOC(json_buf);
    if (!json_buf) {
        LV_LOG_ERROR("malloc failed for json_buf");
        goto failed;
    }

    /* read json sting */

    res = lv_fs_read(&file, json_buf, size, &br);
    lv_fs_close(&file);
    if (res != LV_FS_RES_OK || br != size) {
        LV_LOG_ERROR("read file failed");
        goto failed;
    }

    json_buf[size] = '\0';

    cjson = cJSON_Parse((const char*)json_buf);

    if (!cjson) {
        LV_LOG_ERROR("cJSON_Parse failed");
        goto failed;
    }

    if (vg_video_config_parse(cjson, map) < 0) {
        LV_LOG_ERROR("vg_video_config_parse failed");
        goto failed;
    }

    ret = 0;

failed:
    lv_fs_close(&file);

    if (json_buf) {
        lv_free(json_buf);
    }

    if (cjson) {
        cJSON_Delete(cjson);
    }

    return ret;
}

/****************************************************************************
 * Name: vg_video_format_converter
 ****************************************************************************/

static lv_color_format_t vg_video_format_converter(vg_vtun_frame_format format)
{
    int i;
    int len = sizeof(g_video_format_map) / sizeof(struct vg_video_format_s);
    lv_color_format_t cf = LV_COLOR_FORMAT_UNKNOWN;

    for (i = 0; i < len; i++) {
        if (g_video_format_map[i].video_cf == format) {
            return g_video_format_map[i].img_cf;
        }
    }

    LV_LOG_WARN("unsupported color format : %d!", format);
    return cf;
}

/****************************************************************************
 * Name: vg_find_avail_ctx
 ****************************************************************************/

static struct vg_video_ctx_s* vg_find_avail_ctx(const char* src,
    struct vg_video_ctx_map_s* map)
{
    int i;
    struct vg_video_ctx_s* ctx = map->ctx;
    const char* src_header = VIDEO_SRC_HEADER;
    if (strstart(src, CAMERA_SRC_HEADER, NULL)) {
        src_header = CAMERA_SRC_HEADER;
    }

    for (i = 0; i < map->count; i++) {
        const char* url = NULL;
        strstart(ctx[i].cfg.vtun_name, VTUN_HEADER, &url);
        if (!ctx[i].handle && strstart(url, src_header, NULL)) {
            return &ctx[i];
        }
    }

    return NULL;
}

/****************************************************************************
 * Name: vg_connect_to_server
 ****************************************************************************/

static int vg_connect_to_server(const char* vtun_name)
{
    int fd;
    int flags = SOCK_STREAM | SOCK_CLOEXEC;

#ifdef CONFIG_NET_RPMSG
    char rpmsg_server[RPMSG_SERVER_MAX_LEN];
    const char* rpmsg_server_name;
    char* url;
    strlcpy(rpmsg_server, vtun_name, sizeof(rpmsg_server));

    if (strstart(rpmsg_server, "rpmsg@", &rpmsg_server_name)) {
        struct sockaddr_rpmsg addr;

        if ((url = strchr(rpmsg_server_name, ':')) == NULL) {
            LV_LOG_ERROR("rpmsg server format error : %s", rpmsg_server);
            return -EINVAL;
        }

        *url = '\0';
        url++;

        fd = socket(AF_RPMSG, flags, 0);
        if (fd < 0) {
            LV_LOG_ERROR("create rpmsg socket error for %s  %d", vtun_name, errno);
            return -errno;
        }

        memset(&addr, 0, sizeof(addr));
        strlcpy(addr.rp_name, url, sizeof(addr.rp_name));
        strlcpy(addr.rp_cpu, rpmsg_server_name, sizeof(addr.rp_cpu));
        addr.rp_family = AF_RPMSG;
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            LV_LOG_ERROR("connect rpmsg socket error for %s  %d", vtun_name, errno);
            close(fd);
            return -errno;
        }
    } else
#endif
    {
        struct sockaddr_un addr;
        if ((fd = socket(AF_LOCAL, flags, 0)) < 0) {
            LV_LOG_ERROR("create local socket error for %s  %d", vtun_name, errno);
            return -errno;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strlcpy(addr.sun_path, vtun_name, sizeof(addr.sun_path));
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            LV_LOG_ERROR("connect local socket error for %s  %d", vtun_name, errno);
            close(fd);
            return -errno;
        }
    }

    return fd;
}

/****************************************************************************
 * Name: video_event_cb
 ****************************************************************************/

static void video_event_cb(void* cookie, int event, int ret,
    const char* extra)
{
    LV_UNUSED(ret);
    LV_UNUSED(extra);

    struct vg_video_ctx_s* ctx = (struct vg_video_ctx_s*)cookie;
    switch (event) {
    case MEDIA_EVENT_STARTED:
        if (ctx->started_cb)
            ctx->started_cb(ctx->ui_obj);
        break;
    case MEDIA_EVENT_PREPARED:
        if (ctx->prepared_cb)
            ctx->prepared_cb(ctx->ui_obj);
        break;
    case MEDIA_EVENT_PAUSED:
        if (ctx->paused_cb)
            ctx->paused_cb(ctx->ui_obj);
        break;
    case MEDIA_EVENT_COMPLETED:
        if (ctx->completed_cb)
            ctx->completed_cb(ctx->ui_obj);
        break;
    case MEDIA_EVENT_STOPPED:
        if (ctx->stopped_cb) {
            ctx->stopped_cb(ctx->ui_obj);
        }
        ctx->ui_obj = NULL;
        ctx->started_cb = NULL;
        ctx->prepared_cb = NULL;
        ctx->paused_cb = NULL;
        ctx->completed_cb = NULL;
        ctx->stopped_cb = NULL;
        break;
    default:
        break;
    }
}

/****************************************************************************
 * Name: video_adapter_open
 ****************************************************************************/

static void* video_adapter_open(struct _vg_video_vtable_t* vtable,
    const char* src, const char* option)
{
    if (!vtable) {
        return NULL;
    }

    struct vg_video_adapter_ctx_s* adapter_ctx = (struct vg_video_adapter_ctx_s*)vtable;
    struct vg_video_ctx_s* ctx = vg_find_avail_ctx(src, &(adapter_ctx->map));

    if (!ctx) {
        LV_LOG_ERROR("cannot find available video ctx for %s", src);
        return NULL;
    }

    if ((ctx->fd = vg_connect_to_server(ctx->cfg.vtun_name)) < 0) {
        LV_LOG_ERROR("connect to vtun server %s failed!", ctx->cfg.vtun_name);
        return NULL;
    }

    if (!strstart(src, CAMERA_SRC_HEADER, NULL)) {
        const char* url = NULL;
        strstart(ctx->cfg.vtun_name, VTUN_HEADER, &url);

        ctx->handle = media_uv_player_open(adapter_ctx->ui_uv_loop, url, NULL, ctx);

        if (!ctx->handle) {
            LV_LOG_ERROR("media open : %s failed!", url);
            goto fail;
        }

        if ((media_uv_player_listen(ctx->handle, video_event_cb)) < 0) {
            LV_LOG_ERROR("media player set event callback failed!");
            goto fail;
        }

        if (media_uv_player_prepare(ctx->handle, src, option, NULL, NULL, NULL) < 0) {
            LV_LOG_ERROR("media prepare:%s %s failed!", src, option);
            goto fail;
        }
    }

    return ctx;

fail:
    if (ctx->fd > 0) {
        close(ctx->fd);
        ctx->fd = 0;
    }

    if (ctx->handle) {
        media_uv_player_close(ctx->handle, 0, NULL);
        ctx->handle = NULL;
    }

    return NULL;
}

/****************************************************************************
 * Name: video_adapter_set_callback
 ****************************************************************************/
static int video_adapter_set_callback(struct _vg_video_vtable_t* vtable,
    void* ctx, int event, void* obj, video_event_callback callback)
{
    if (!ctx) {
        return -EPERM;
    }
    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    switch (event) {
    case MEDIA_EVENT_STARTED:
        video_ctx->started_cb = callback;
        break;
    case MEDIA_EVENT_PREPARED:
        video_ctx->prepared_cb = callback;
        break;
    case MEDIA_EVENT_PAUSED:
        video_ctx->paused_cb = callback;
        break;
    case MEDIA_EVENT_STOPPED:
        video_ctx->stopped_cb = callback;
        break;
    case MEDIA_EVENT_COMPLETED:
        video_ctx->completed_cb = callback;
        break;
    default:
        break;
    }
    video_ctx->ui_obj = obj;
    return OK;
}

/****************************************************************************
 * Name: video_adapter_get_frame
 ****************************************************************************/

static int video_adapter_get_frame(struct _vg_video_vtable_t* vtable,
    void* ctx, vg_video_t* video)
{
    struct pollfd fds[1];
    char cmd = VTUN_CTRL_EVT_FRAME_REQ;
    vg_vtun_frame* frame_p = NULL;

    if (!ctx || !video) {
        return -EPERM;
    }

    lv_image_dsc_t* img_dsc = &video->img_dsc;

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (send(video_ctx->fd, &cmd, sizeof(cmd), MSG_NOSIGNAL) < 0) {
        LV_LOG_ERROR("frame request send error %d", errno);
        return -errno;
    }

    fds[0].fd = video_ctx->fd;
    fds[0].events = POLLIN;

    if (poll(fds, 1, VIDEO_GET_FRAME_TIMEOUT) <= 0) {
        LV_LOG_ERROR("frame poll error %d", errno);
        return -errno;
    }

    if (recv(video_ctx->fd, &frame_p, sizeof(frame_p), MSG_NOSIGNAL) < 0) {
        LV_LOG_ERROR("frame recv error %d", errno);
        return -errno;
    }

    if (frame_p == NULL) {
        return -ENOENT;
    }

    img_dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    img_dsc->header.w = frame_p->w;
    img_dsc->header.h = frame_p->h;
    img_dsc->header.cf = vg_video_format_converter(frame_p->format);
    img_dsc->header.stride = frame_p->plane[0].stride;
    img_dsc->data_size = img_dsc->header.stride * frame_p->h;

    if (img_dsc->header.cf == LV_COLOR_FORMAT_NV12) {
        video_ctx->yuv.semi_planar.y.buf = frame_p->plane[0].addr;
        video_ctx->yuv.semi_planar.y.stride = frame_p->plane[0].stride;
        video_ctx->yuv.semi_planar.uv.buf = frame_p->plane[1].addr;
        video_ctx->yuv.semi_planar.uv.stride = frame_p->plane[1].stride;
        img_dsc->data = (const uint8_t*)&video_ctx->yuv;
    } else {
        img_dsc->data = frame_p->plane[0].addr;
    }

    video->crop_coords.x1 = frame_p->crop_info.x1;
    video->crop_coords.x2 = frame_p->crop_info.x2;
    video->crop_coords.y1 = frame_p->crop_info.y1;
    video->crop_coords.y2 = frame_p->crop_info.y2;

    video->cur_time = frame_p->current_ms / 1000;
    return OK;
}

/****************************************************************************
 * Name: video_adapter_get_dur
 ****************************************************************************/

static int video_adapter_get_dur(struct _vg_video_vtable_t* vtable,
    void* ctx, media_uv_unsigned_callback callback, void* cookie)
{
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if (media_uv_player_get_duration(video_ctx->handle, callback, cookie) < 0) {
        LV_LOG_ERROR("media player get duration failed!");
    }

    return OK;
}

/****************************************************************************
 * Name: video_adapter_start
 ****************************************************************************/

static int video_adapter_start(struct _vg_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;
    if (video_ctx->handle) {
        if ((ret = media_uv_player_start(video_ctx->handle, NULL, NULL)) < 0) {
            LV_LOG_ERROR("media player start failed!");
            return ret;
        }
    } else {
        char cmd = VTUN_CTRL_EVT_PLAY;
        if (send(video_ctx->fd, &cmd, sizeof(cmd), MSG_NOSIGNAL) < 0) {
            LV_LOG_ERROR("preview play send error %d", errno);
            return -errno;
        }
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_seek
 ****************************************************************************/

static int video_adapter_seek(struct _vg_video_vtable_t* vtable,
    void* ctx, int pos)
{
    int ret;
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if ((ret = media_uv_player_seek(video_ctx->handle, pos * 1000, NULL, NULL)) < 0) {
        LV_LOG_ERROR("media player seek to (%d) failed!", pos);
        return ret;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_pause
 ****************************************************************************/

static int video_adapter_pause(struct _vg_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if ((ret = media_uv_player_pause(video_ctx->handle, NULL, NULL)) < 0) {
        LV_LOG_ERROR("media player pause failed!");
        return ret;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_resume
 ****************************************************************************/

static int video_adapter_resume(struct _vg_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if ((ret = media_uv_player_start(video_ctx->handle, NULL, NULL)) < 0) {
        LV_LOG_ERROR("media player resume failed!");
        return ret;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_stop
 ****************************************************************************/

static int video_adapter_stop(struct _vg_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (video_ctx->handle) {
        if ((ret = media_uv_player_stop(video_ctx->handle, NULL, NULL)) < 0) {
            LV_LOG_ERROR("media player stop failed!");
            return ret;
        }
    } else {
        char cmd = VTUN_CTRL_EVT_STOP;
        if (send(video_ctx->fd, &cmd, sizeof(cmd), MSG_NOSIGNAL) < 0) {
            LV_LOG_ERROR("preview stop send error %d", errno);
            return -errno;
        }
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_close
 ****************************************************************************/

static int video_adapter_close(struct _vg_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (video_ctx->fd > 0) {
        close(video_ctx->fd);
        video_ctx->fd = 0;
    }

    lv_memset(&video_ctx->yuv, 0, sizeof(lv_yuv_buf_t));

    if (video_ctx->handle) {
        if ((ret = media_uv_player_close(video_ctx->handle, 0, NULL)) < 0) {
            LV_LOG_ERROR("media player close failed!");
            return ret;
        }

        video_ctx->handle = NULL;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_destroy
 ****************************************************************************/

static void video_adapter_destroy(struct vg_video_adapter_ctx_s* adapter_ctx)
{
    int i = 0;

    struct vg_video_ctx_map_s* map = &adapter_ctx->map;
    struct vg_video_ctx_s* ctx = map->ctx;

    if (ctx) {
        for (i = 0; i < map->count; i++) {
            lv_free(ctx[i].cfg.vtun_name);
        }

        lv_free(ctx);
    }

    lv_free(adapter_ctx);
}

/****************************************************************************
 * Name: video_adapter_loop
 ****************************************************************************/

static int video_adapter_loop(struct _vg_video_vtable_t* vtable,
    void* ctx, int loop)
{
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    if (media_uv_player_set_looping(video_ctx->handle, loop, NULL, NULL) < 0) {
        LV_LOG_ERROR("media player set loop failed!");
        return -1;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_get_playing
 ****************************************************************************/
static int video_adapter_get_playing(struct _vg_video_vtable_t* vtable,
    void* ctx, media_uv_int_callback cb, void* cookie)
{
    if (!ctx) {
        return -EPERM;
    }

    struct vg_video_ctx_s* video_ctx = (struct vg_video_ctx_s*)ctx;

    return media_uv_player_get_playing(video_ctx->handle, cb, cookie);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: vg_video_adapter_init
 *
 * Description:
 *   Video adapter init.
 *
 ****************************************************************************/

void vg_video_adapter_init(void)
{
    struct vg_video_adapter_ctx_s* adapter_ctx;

    adapter_ctx = lv_malloc(sizeof(struct vg_video_adapter_ctx_s));
    LV_ASSERT_MALLOC(adapter_ctx);
    lv_memset(adapter_ctx, 0, sizeof(struct vg_video_adapter_ctx_s));

    if (vg_video_config_init(CONFIG_UIKIT_VIDEO_CONFIG_FILE_PATH,
            &(adapter_ctx->map))
        < 0) {
        return;
    }

    adapter_ctx->vtable.video_adapter_open = video_adapter_open;
    adapter_ctx->vtable.video_adapter_get_frame = video_adapter_get_frame;
    adapter_ctx->vtable.video_adapter_get_dur = video_adapter_get_dur;
    adapter_ctx->vtable.video_adapter_start = video_adapter_start;
    adapter_ctx->vtable.video_adapter_seek = video_adapter_seek;
    adapter_ctx->vtable.video_adapter_pause = video_adapter_pause;
    adapter_ctx->vtable.video_adapter_resume = video_adapter_resume;
    adapter_ctx->vtable.video_adapter_stop = video_adapter_stop;
    adapter_ctx->vtable.video_adapter_close = video_adapter_close;
    adapter_ctx->vtable.video_adapter_loop = video_adapter_loop;
    adapter_ctx->vtable.video_adapter_get_playing = video_adapter_get_playing;
    adapter_ctx->vtable.video_adapter_set_callback = video_adapter_set_callback;

    vg_video_vtable_set_default(&(adapter_ctx->vtable));
}

/****************************************************************************
 * Name: vg_video_adapter_uninit
 *
 * Description:
 *   Video adapter uninit.
 *
 ****************************************************************************/

void vg_video_adapter_uninit(void)
{
    struct vg_video_adapter_ctx_s* adapter_ctx = (struct vg_video_adapter_ctx_s*)vg_video_vtable_get_default();
    video_adapter_destroy(adapter_ctx);
}

/****************************************************************************
 * Name: vg_video_adapter_loop_init
 *
 * Description:
 *   Video adapter init ui uv loop.
 *
 ****************************************************************************/

void vg_video_adapter_loop_init(void* loop)
{
    struct vg_video_adapter_ctx_s* adapter_ctx = (struct vg_video_adapter_ctx_s*)vg_video_vtable_get_default();

    if (loop == NULL) {
        LV_LOG_ERROR("video adapter loop init failed for nullptr loop.");
        return;
    }

    adapter_ctx->ui_uv_loop = loop;
}

/****************************************************************************
 * Name: vg_video_adapter_loop_deinit
 *
 * Description:
 *   Video adapter deinit ui uv loop.
 *
 ****************************************************************************/

void vg_video_adapter_loop_deinit(void)
{
    struct vg_video_adapter_ctx_s* adapter_ctx = (struct vg_video_adapter_ctx_s*)vg_video_vtable_get_default();
    adapter_ctx->ui_uv_loop = NULL;
}

#endif /* CONFIG_UIKIT_VIDEO_ADAPTER */
