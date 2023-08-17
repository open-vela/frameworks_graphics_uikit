/**
 * @file flashlight.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "page.h"

#include "../utils/lv_obj_ext_func.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_fragment_t base;
} page_ctx_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void on_root_event(lv_event_t * e)
{
    lv_obj_t * root = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    page_ctx_t * ctx = lv_obj_get_user_data(root);

    if(code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_RIGHT) {
            lv_obj_send_event(root, LV_EVENT_LEAVE, NULL);
        }
    }
    else if(code == LV_EVENT_LEAVE) {
        page_pop(&ctx->base);
    }
}

static void on_page_construct(lv_fragment_t * self, void * args)
{
}

static void on_page_destruct(lv_fragment_t * self)
{
}

static void on_page_attached(lv_fragment_t * self)
{
}

static void on_page_detached(lv_fragment_t * self)
{
}

static lv_obj_t * on_page_create(lv_fragment_t * self, lv_obj_t * container)
{
    lv_obj_t * root = lv_obj_create(container);
    lv_obj_remove_style_all(root);
    lv_obj_add_style(root, resource_get_style("root_def"), 0);
    return root;
}

static void on_page_created(lv_fragment_t * self, lv_obj_t * obj)
{
    lv_obj_set_user_data(obj, self);
    lv_obj_add_event(obj, on_root_event, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);

    static const lv_style_prop_t props[] = { LV_STYLE_BG_COLOR, LV_STYLE_PROP_INV };
    static lv_style_transition_dsc_t dsc;
    lv_style_transition_dsc_init(&dsc, props, lv_anim_path_ease_out, 200, 0, NULL);

    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_mix(lv_color_white(), lv_color_black(), 200), LV_STATE_CHECKED);
    lv_obj_set_style_transition(obj, &dsc, LV_STATE_DEFAULT);
    lv_obj_set_style_transition(obj, &dsc, LV_STATE_CHECKED);
}

static void on_page_will_delete(lv_fragment_t * self, lv_obj_t * obj)
{
}

static void on_page_deleted(lv_fragment_t * self, lv_obj_t * obj)
{
}

static bool on_page_event(lv_fragment_t * self, int code, void * user_data)
{
    return false;
}

PAGE_CLASS_DEF(flashlight);
