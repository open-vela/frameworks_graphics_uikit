/****************************************************************************
 * apps/graphics/lvgl/ext/video/lvx_video_adapter.c
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

#include "lvx_video_adapter.h"
#include "lvx_video.h"
#include <cJSON.h>
#ifdef CONFIG_NET_RPMSG
#include <netpacket/rpmsg.h>
#endif /* CONFIG_NET_RPMSG */
#include "lvx_vtun.h"
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

#ifdef CONFIG_LVX_USE_VIDEO_ADAPTER

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

#define VTUN_SERVER_PATH_HEAD "vtun_server_path"

#define MEDIA_PLAYER_PREPARE_OPT "format=sasp:noheader=1"

/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/

struct lvx_video_format_s {
    lv_img_cf_t img_cf;
    lvx_vtun_frame_format video_cf;
};

struct lvx_video_ctx_config_s {
    char* vtun_name;
};

struct lvx_video_ctx_s {
    int fd;
    void* handle;
    struct lvx_video_ctx_config_s cfg;
};

struct lvx_video_ctx_map_s {
    struct lvx_video_ctx_s* ctx;
    int count;
};

struct lvx_video_adapter_ctx_s {
    lvx_video_vtable_t vtable;
    struct lvx_video_ctx_map_s map;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct lvx_video_format_s g_video_format_map[] = {
    { LV_IMG_CF_TRUE_COLOR_ALPHA, VTUN_FRAME_FORMAT_BGRA8888 },
    { LV_IMG_CF_RESERVED_17, VTUN_FRAME_FORMAT_YUV420SP},
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
 * Name: strcontain
 ****************************************************************************/

static bool strcontain(const char* str, const char* pfx)
{
    int index = 0;
    int str_len = strlen(str);
    int pfx_len = strlen(pfx);
    if (str_len < pfx_len) {
        return false;
    }
    while (index < str_len - pfx_len && *pfx) {
        while (*str && *str != *pfx) {
            str++;
        }
        while (*str && *pfx && *str == *pfx) {
            str++;
            pfx++;
        }
        index++;
    }
    return !*pfx;
}

/****************************************************************************
 * Name: lvx_video_config_item_parse
 ****************************************************************************/

static int lvx_video_config_item_parse(cJSON* cjson,
    struct lvx_video_ctx_s* ctx)
{
    if (!cJSON_IsString(cjson)) {
        LV_LOG_ERROR("parse video tunnel config error.");
        return -EINVAL;
    }

    ctx->cfg.vtun_name = strdup(cjson->valuestring);

    return 0;
}

/****************************************************************************
 * Name: lvx_video_config_parse
 ****************************************************************************/

static int lvx_video_config_parse(cJSON* cjson,
    struct lvx_video_ctx_map_s* map)
{
    cJSON* ele = NULL;
    int i = 0;
    int ret = 0;

    map->count = cJSON_GetArraySize(cjson);

    if (map->count <= 0) {
        return -EINVAL;
    }

    map->ctx = lv_mem_alloc(map->count * sizeof(struct lvx_video_ctx_s));
    LV_ASSERT_MALLOC(map->ctx);

    if (!map->ctx) {
        LV_LOG_ERROR("malloc failed for map->ctx %d", errno);
        return -errno;
    }

    lv_memset_00(map->ctx, map->count * sizeof(struct lvx_video_ctx_s));

    cJSON_ArrayForEach(ele, cjson)
    {
        if ((ret = lvx_video_config_item_parse(ele, &map->ctx[i])) < 0) {
            LV_LOG_ERROR("lvx_video_config_item_parse (%d) error!", i);
            break;
        }

        i++;
    }

    return ret;
}

/****************************************************************************
 * Name: lvx_video_config_init
 ****************************************************************************/

static int lvx_video_config_init(const char* file_path,
    struct lvx_video_ctx_map_s* map)
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

    json_buf = lv_mem_alloc(size + 1);
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

    if (lvx_video_config_parse(cjson, map) < 0) {
        LV_LOG_ERROR("lvx_video_config_parse failed");
        goto failed;
    }

    ret = 0;

failed:
    lv_fs_close(&file);

    if (json_buf) {
        lv_mem_free(json_buf);
    }

    if (cjson) {
        cJSON_Delete(cjson);
    }

    return ret;
}

/****************************************************************************
 * Name: lvx_video_format_converter
 ****************************************************************************/

static lv_img_cf_t lvx_video_format_converter(lvx_vtun_frame_format format)
{
    int i;
    int len = sizeof(g_video_format_map) / sizeof(struct lvx_video_format_s);
    lv_img_cf_t cf = LV_IMG_CF_UNKNOWN;

    for (i = 0; i < len; i++) {
        if (g_video_format_map[i].video_cf == format) {
            return g_video_format_map[i].img_cf;
        }
    }

    return cf;
}

/****************************************************************************
 * Name: lvx_find_avail_ctx
 ****************************************************************************/

static struct lvx_video_ctx_s* lvx_find_avail_ctx(const char* src,
    struct lvx_video_ctx_map_s* map)
{
    int i;
    struct lvx_video_ctx_s* ctx = map->ctx;
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
 * Name: lvx_connect_to_server
 ****************************************************************************/

static int lvx_connect_to_server(const char* vtun_name)
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
 * Name: video_adapter_open
 ****************************************************************************/

static void* video_adapter_open(struct _lvx_video_vtable_t* vtable,
    const char* src)
{
    struct lvx_video_adapter_ctx_s* adapter_ctx = (struct lvx_video_adapter_ctx_s*)vtable;
    struct lvx_video_ctx_s* ctx = lvx_find_avail_ctx(src, &(adapter_ctx->map));

    if (!ctx) {
        LV_LOG_ERROR("cannot find available video ctx for %s", src);
        return NULL;
    }

    if ((ctx->fd = lvx_connect_to_server(ctx->cfg.vtun_name)) < 0) {
        LV_LOG_ERROR("connect to vtun server %s failed!", ctx->cfg.vtun_name);
        return NULL;
    }

    if (!strstart(src, CAMERA_SRC_HEADER, NULL)) {
        const char* url = NULL;
        strstart(ctx->cfg.vtun_name, VTUN_HEADER, &url);

        ctx->handle = media_player_open(url);

        if (!ctx->handle) {
            LV_LOG_ERROR("media open : %s failed!", url);
            goto fail;
        }

        if (!strcontain(src, VTUN_SERVER_PATH_HEAD)) {
            if (media_player_prepare(ctx->handle, src, NULL) < 0) {
                LV_LOG_ERROR("media prepare:%s failed!", url);
                goto fail;
            }
        } else {
            if (media_player_prepare(ctx->handle, NULL, MEDIA_PLAYER_PREPARE_OPT) < 0) {
                LV_LOG_ERROR("media prepare:%s failed!", MEDIA_PLAYER_PREPARE_OPT);
                goto fail;
            }
        }
    }

    return ctx;

fail:
    if (ctx->fd > 0) {
        close(ctx->fd);
        ctx->fd = 0;
    }

    if (ctx->handle) {
        media_player_close(ctx->handle, 0);
        ctx->handle = NULL;
    }

    return NULL;
}

/****************************************************************************
 * Name: video_adapter_get_frame
 ****************************************************************************/

static int video_adapter_get_frame(struct _lvx_video_vtable_t* vtable,
    void* ctx, lvx_video_t* video)
{
    struct pollfd fds[1];
    char cmd = VTUN_CTRL_EVT_FRAME_REQ;
    lvx_vtun_frame* frame_p = NULL;
    lv_img_dsc_t* img_dsc = &video->img_dsc;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

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

    if (img_dsc->data == frame_p->addr) {
        return -ENOENT;
    }

    img_dsc->header.always_zero = 0;
    img_dsc->header.w = frame_p->w;
    img_dsc->header.h = frame_p->h;
    img_dsc->header.cf = lvx_video_format_converter(frame_p->format);
    img_dsc->data_size = frame_p->size;
    img_dsc->data = frame_p->addr;

    video->cur_time = frame_p->current_ms / 1000;
    return OK;
}

/****************************************************************************
 * Name: video_adapter_get_dur
 ****************************************************************************/

static int video_adapter_get_dur(struct _lvx_video_vtable_t* vtable,
    void* ctx)
{
    unsigned int duration = 0;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if (media_player_get_duration(video_ctx->handle, &duration) < 0) {
        LV_LOG_ERROR("media player get duration failed!");
    }

    return duration / 1000;
}

/****************************************************************************
 * Name: video_adapter_start
 ****************************************************************************/

static int video_adapter_start(struct _lvx_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (video_ctx->handle) {
        if ((ret = media_player_start(video_ctx->handle)) < 0) {
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

static int video_adapter_seek(struct _lvx_video_vtable_t* vtable,
    void* ctx, int pos)
{
    int ret;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if ((ret = media_player_seek(video_ctx->handle, pos * 1000)) < 0) {
        LV_LOG_ERROR("media player seek to (%d) failed!", pos);
        return ret;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_pause
 ****************************************************************************/

static int video_adapter_pause(struct _lvx_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if ((ret = media_player_pause(video_ctx->handle)) < 0) {
        LV_LOG_ERROR("media player pause failed!");
        return ret;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_resume
 ****************************************************************************/

static int video_adapter_resume(struct _lvx_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (!video_ctx->handle) {
        return -EINVAL;
    }

    if ((ret = media_player_start(video_ctx->handle)) < 0) {
        LV_LOG_ERROR("media player resume failed!");
        return ret;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_stop
 ****************************************************************************/

static int video_adapter_stop(struct _lvx_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (video_ctx->handle) {
        if ((ret = media_player_stop(video_ctx->handle)) < 0) {
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

static int video_adapter_close(struct _lvx_video_vtable_t* vtable,
    void* ctx)
{
    int ret;
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (video_ctx->fd > 0) {
        close(video_ctx->fd);
        video_ctx->fd = 0;
    }

    if (video_ctx->handle) {
        if ((ret = media_player_close(video_ctx->handle, 0)) < 0) {
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

static void video_adapter_destroy(struct lvx_video_adapter_ctx_s* adapter_ctx)
{
    int i = 0;

    struct lvx_video_ctx_map_s* map = &adapter_ctx->map;
    struct lvx_video_ctx_s* ctx = map->ctx;

    if (ctx) {
        for (i = 0; i < map->count; i++) {
            lv_mem_free(ctx[i].cfg.vtun_name);
        }

        lv_mem_free(ctx);
    }

    lv_mem_free(adapter_ctx);
}

/****************************************************************************
 * Name: video_adapter_loop
 ****************************************************************************/

static int video_adapter_loop(struct _lvx_video_vtable_t* vtable,
    void* ctx, int loop)
{
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (media_player_set_looping(video_ctx->handle, loop) < 0) {
        LV_LOG_ERROR("media player set loop failed!");
        return -1;
    }

    return 0;
}

/****************************************************************************
 * Name: video_adapter_play_state
 ****************************************************************************/
static int video_adapter_get_player_state(struct _lvx_video_vtable_t* vtable,
    void* ctx)
{
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    return media_player_is_playing(video_ctx->handle);
}

/****************************************************************************
 * Name: video_adapter_write_data
 ****************************************************************************/
static int video_adapter_write_data(struct _lvx_video_vtable_t* vtable,
    void* ctx, void* data, size_t len)
{
    struct lvx_video_ctx_s* video_ctx = (struct lvx_video_ctx_s*)ctx;

    if (media_player_write_data(video_ctx->handle, data, len) < 0) {
        LV_LOG_ERROR("media player write data failed!");
        return -1;
    }

    return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: lvx_video_adapter_init
 *
 * Description:
 *   Video adapter init.
 *
 ****************************************************************************/

void lvx_video_adapter_init(void)
{
    struct lvx_video_adapter_ctx_s* adapter_ctx;

    adapter_ctx = lv_mem_alloc(sizeof(struct lvx_video_adapter_ctx_s));
    LV_ASSERT_MALLOC(adapter_ctx);
    lv_memset(adapter_ctx, 0, sizeof(struct lvx_video_adapter_ctx_s));

    if (lvx_video_config_init(CONFIG_LVX_VIDEO_CONFIG_FILE_PATH,
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
    adapter_ctx->vtable.video_adapter_get_player_state = video_adapter_get_player_state;
    adapter_ctx->vtable.video_adapter_write_data = video_adapter_write_data;

    lvx_video_vtable_set_default(&(adapter_ctx->vtable));
}

/****************************************************************************
 * Name: lvx_video_adapter_uninit
 *
 * Description:
 *   Video adapter uninit.
 *
 ****************************************************************************/

void lvx_video_adapter_uninit(void)
{
    struct lvx_video_adapter_ctx_s* adapter_ctx = (struct lvx_video_adapter_ctx_s*)lvx_video_vtable_get_default();
    video_adapter_destroy(adapter_ctx);
}

#endif /* CONFIG_LVX_USE_VIDEO_ADAPTER */
