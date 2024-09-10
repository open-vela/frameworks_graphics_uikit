
/**
 * @file uikit_demo.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <nuttx/config.h>
#include <unistd.h>

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
#include <uv.h>
#endif

#include "uikit/uikit.h"
#include <lvgl/lvgl.h>

#ifdef CONFIG_UIKIT_DEMO_VIDEO
#include "video/camera_demo.h"
#include "video/video_demo.h"
#endif

#ifdef CONFIG_UIKIT_DEMO_BANDX
#include "bandx/bandx_demo.h"
#endif

#ifdef CONFIG_UIKIT_DEMO_VECTOR_DRAW
#include "draw/draw_demos.h"
#endif

#ifdef CONFIG_UIKIT_DEMO_TIME_OBJCREATION
#include "creation/object_creation_test.h"
#endif

/*********************
 *      DEFINES
 *********************/

#define LV_EXT_DEMOS_COUNT (sizeof(demos_entry_info) / sizeof(demo_entry_info_t) - 1)

/**********************
 *      TYPEDEFS
 **********************/

typedef void (*demo_method_cb)(char* info[], int size, void* param);

typedef struct {
    const char* name;
    demo_method_cb entry_cb;
} demo_entry_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static bool uikit_demo(char* info[], int size, void* param);
#ifdef CONFIG_LV_USE_NUTTX_LIBUV
static void* lv_nuttx_uv_loop_init(uv_loop_t* loop, lv_disp_t* disp,
    lv_indev_t* indev);
static void lv_nuttx_uv_loop_run(uv_loop_t* loop, void* data);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

static const demo_entry_info_t demos_entry_info[] = {

#ifdef CONFIG_UIKIT_DEMO_VIDEO
    { "video", .entry_cb = uikit_demo_video },
    { "camera", .entry_cb = uikit_demo_camera },
    { "video_call", .entry_cb = uikit_demo_video_call },
    { "video_ctl", .entry_cb = uikit_demo_video_controller },
#endif

#ifdef CONFIG_UIKIT_DEMO_BANDX
    { "bandx", .entry_cb = uikit_bandx_demo },
#endif

#ifdef CONFIG_UIKIT_DEMO_VECTOR_DRAW
    { "tiger", .entry_cb = uikit_draw_demo_tiger },
    { "hearts", .entry_cb = uikit_draw_demo_hearts },
#endif

#ifdef CONFIG_UIKIT_DEMO_TIME_OBJCREATION
    { "time_obj_creation", .entry_cb = uikit_demo_time_object_creation },
    { "time_render_label", .entry_cb = uikit_demo_time_render_label },
    { "time_render_spangroup", .entry_cb = uikit_demo_time_render_spangroup },
#endif

    { "", .entry_cb = NULL }
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, FAR char* argv[])
{
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;
    void* param = NULL;

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    uv_loop_t ui_loop = { .data = NULL };
#endif

    lv_init();

    lv_nuttx_dsc_init(&info);
    lv_nuttx_init(&info, &result);

    vg_init();

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    void* data = lv_nuttx_uv_loop_init(&ui_loop, result.disp, result.indev);
    vg_uv_init(&ui_loop);
    param = &ui_loop;
#endif

    if (result.disp == NULL) {
        LV_LOG_ERROR("uikit_demo initialization failure!");
        return 1;
    }

    if (!uikit_demo(&argv[1], argc - 1, param)) {

        /* we can add custom demos here */

        goto demo_end;
    }

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    lv_nuttx_uv_loop_run(&ui_loop, data);
#else
    while (1) {
        lv_timer_handler();
        usleep(10 * 1000);
    }
#endif

demo_end:
    vg_uv_deinit();
    lv_disp_remove(result.disp);
    vg_deinit();
    lv_deinit();

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
static void* lv_nuttx_uv_loop_init(uv_loop_t* loop, lv_disp_t* disp,
    lv_indev_t* indev)
{
    lv_nuttx_uv_t uv_info;
    lv_memset(&uv_info, 0, sizeof(uv_info));

    uv_loop_init(loop);
    uv_info.loop = loop;
    uv_info.disp = disp;
    uv_info.indev = indev;

    return lv_nuttx_uv_init(&uv_info);
}

static void lv_nuttx_uv_loop_run(uv_loop_t* loop, void* data)
{
    uv_run(loop, UV_RUN_DEFAULT);
    lv_nuttx_uv_deinit(&data);
}
#endif

static bool uikit_demo(char* info[], int size, void* param)
{
    const int demos_count = LV_EXT_DEMOS_COUNT;

    if (demos_count <= 0) {
        LV_LOG_ERROR("Please enable some uikit_demo firstly!");
        return false;
    }

    const demo_entry_info_t* entry_info = NULL;

    if (size > 0 && info) {
        const char* name = info[0];
        for (int i = 0; i < demos_count; i++) {
            if (strcmp(name, demos_entry_info[i].name) == 0) {
                entry_info = &demos_entry_info[i];
                break;
            }
        }
    }

    if (entry_info == NULL) {
        LV_LOG_ERROR("uikit_demo create(%s) failure!", (size > 0 && info) ? info[0] : "");
        goto err;
    }

    if (entry_info->entry_cb) {
        entry_info->entry_cb(info, size, param);
        return true;
    }

err:
    LV_LOG("\nUsage: uikit_demo [parameters]\n");
    LV_LOG("\ndemo list:\n");

    for (int i = 0; i < demos_count; i++) {
        LV_LOG("     %s \n", demos_entry_info[i].name);
    }

    return false;
}
