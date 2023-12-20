
/**
 * @file lv_ext_demo.c
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
#include "../lv_ext.h"
#include "lv_ext_demos.h"

#ifdef CONFIG_ANIM_ENGINE_API_DEMO
#include <api_demo/api_demo.h>
#endif

#ifdef CONFIG_ANIM_ENGINE_LVGL_IMAGE_DEMO
#include <lvgl_test/ui_test.h>
#endif

/*********************
 *      DEFINES
 *********************/

#define LV_EXT_DEMOS_COUNT (sizeof(demos_entry_info) / sizeof(demo_entry_info_t) - 1)

/**********************
 *      TYPEDEFS
 **********************/

typedef void (*demo_method_cb)(char* info[], int size);

typedef struct {
    const char* name;
    demo_method_cb entry_cb;
} demo_entry_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static bool lv_ext_demos(char* info[], int size);
#ifdef CONFIG_LV_USE_NUTTX_LIBUV
static void* lv_nuttx_uv_loop_init(uv_loop_t* loop, lv_disp_t* disp,
    lv_indev_t* indev);
static void lv_nuttx_uv_loop_run(uv_loop_t* loop, void* data);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

static const demo_entry_info_t demos_entry_info[] = {

#ifdef CONFIG_LVX_USE_VIDEO_EXAMPLE
    { "video", .entry_cb = lvx_example_video },
    { "camera", .entry_cb = lvx_example_camera },
    { "video_call", .entry_cb = lvx_example_video_call },
    { "video_ctl", .entry_cb = lvx_example_video_controller },
#endif
#ifdef CONFIG_ANIM_ENGINE_API_DEMO
    { "animengine", .entry_cb = anim_api_demo },
#endif

#ifdef CONFIG_ANIM_ENGINE_LVGL_IMAGE_DEMO
    { "anim_image", .entry_cb = lvgl_anim_test },
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

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    uv_loop_t ui_loop;
#endif

    lv_init();

    lv_nuttx_dsc_init(&info);
    info.fb_path = CONFIG_LV_FBDEV_INTERFACE_DEFAULT_DEVICEPATH;
    info.input_path = CONFIG_LV_TOUCHPAD_INTERFACE_DEFAULT_DEVICEPATH;

    lv_nuttx_init(&info, &result);

    lv_ext_init();

#ifdef CONFIG_LV_USE_NUTTX_LIBUV
    void* data = lv_nuttx_uv_loop_init(&ui_loop, result.disp, result.indev);
    lv_ext_uv_init(&ui_loop);
#endif

    if (result.disp == NULL) {
        LV_LOG_ERROR("lv_demos initialization failure!");
        return 1;
    }

    if (!lv_ext_demos(&argv[1], argc - 1)) {

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
    lv_ext_uv_deinit();
    lv_disp_remove(result.disp);
    lv_ext_deinit();
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

static bool lv_ext_demos(char* info[], int size)
{
    const int demos_count = LV_EXT_DEMOS_COUNT;

    if (demos_count <= 0) {
        LV_LOG_ERROR("Please enable some lv_ext_demos firstly!");
        return false;
    }

    const demo_entry_info_t* entry_info = NULL;

    if (size > 0 && info) {
        const char* name = info[0];
        for (int i = 0; i < demos_count; i++) {
            if (strcmp(name, demos_entry_info[i].name) == 0) {
                entry_info = &demos_entry_info[i];
            }
        }
    }

    if (entry_info == NULL) {
        LV_LOG_ERROR("lv_ext_demos create(%s) failure!", size > 0 ? info[0] : "");
        goto err;
    }

    if (entry_info->entry_cb) {
        entry_info->entry_cb(&info[1], size - 1);
        return true;
    }

err:
    LV_LOG("\nUsage: lv_ext_demos demo [parameters]\n");
    LV_LOG("\ndemo list:\n");

    for (int i = 0; i < demos_count; i++) {
        LV_LOG("     %s \n", demos_entry_info[i].name);
    }

    return false;
}
