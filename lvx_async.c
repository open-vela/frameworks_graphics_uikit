/**
 * @file lvx_async.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_async.h"
#include "lvgl/src/lvgl_private.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef void (*refr_event_cb_t)(lv_display_t * disp_drv);

typedef struct {
    lv_async_cb_t cb;
    void * user_data;
    bool is_cancelled;
} async_refr_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lvx_async_refr_init(void);
static lv_result_t lvx_async_refr_call(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data);
static lv_result_t lvx_async_refr_call_cancel(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data);

/**********************
 *  STATIC VARIABLES
 **********************/

static bool async_inited = false;
static lv_ll_t async_refr_start_ll = { 0 };
static lv_ll_t async_refr_finish_ll = { 0 };

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_result_t lvx_async_before_refr_call(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call(&async_refr_start_ll, async_xcb, user_data);
}

lv_result_t lvx_async_before_refr_call_cancel(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call_cancel(&async_refr_start_ll, async_xcb, user_data);
}

lv_result_t lvx_async_after_refr_call(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call(&async_refr_finish_ll, async_xcb, user_data);
}

lv_result_t lvx_async_after_refr_call_cancel(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call_cancel(&async_refr_finish_ll, async_xcb, user_data);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_result_t lvx_async_refr_call(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data)
{
    if(!async_inited) {
        lvx_async_refr_init();
    }

    async_refr_info_t * info = _lv_ll_ins_tail(ll_p);
    LV_ASSERT_MALLOC(info);

    if(info == NULL) {
        return LV_RESULT_INVALID;
    }

    info->cb = async_xcb;
    info->user_data = user_data;
    info->is_cancelled = false;
    return LV_RESULT_OK;
}

static lv_result_t lvx_async_refr_call_cancel(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data)
{
    if(!async_inited) {
        return LV_RESULT_INVALID;
    }

    lv_res_t res = LV_RESULT_INVALID;
    async_refr_info_t * info;

    _LV_LL_READ(ll_p, info) {
        if(info->cb == async_xcb && info->user_data == user_data) {
            info->is_cancelled = true;
            res = LV_RESULT_OK;
        }
    }

    return res;
}

static void on_refr_event(lv_event_t * e)
{
    lv_ll_t * ll_p = lv_event_get_user_data(e);
    if(_lv_ll_is_empty(ll_p)) {
        return;
    }

    async_refr_info_t * info;

    _LV_LL_READ(ll_p, info) {
        if(!info->is_cancelled) {
            info->cb(info->user_data);
        }
    }

    _lv_ll_clear(ll_p);
}

static void lvx_async_refr_init(void)
{
    lv_display_t * disp = lv_display_get_default();
    if(!disp) {
        LV_LOG_WARN("disp is NULL");
        return;
    }

    lv_event_add(&disp->event_list, on_refr_event, LV_EVENT_REFR_START, &async_refr_start_ll);
    lv_event_add(&disp->event_list, on_refr_event, LV_EVENT_REFR_FINISH, &async_refr_finish_ll);

    _lv_ll_init(&async_refr_start_ll, sizeof(async_refr_info_t));
    _lv_ll_init(&async_refr_finish_ll, sizeof(async_refr_info_t));
    async_inited = true;
}
