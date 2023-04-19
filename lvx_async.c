/**
 * @file lvx_async.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_async.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef void (*refr_event_cb_t)(lv_disp_drv_t * disp_drv);

typedef struct {
    lv_async_cb_t cb;
    void * user_data;
} async_refr_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lvx_async_refr_init(void);
static lv_res_t lvx_async_refr_call(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data);
static lv_res_t lvx_async_refr_call_cancel(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data);

/**********************
 *  STATIC VARIABLES
 **********************/

static bool async_inited = false;
static lv_ll_t async_refr_start_ll = { 0 };
static lv_ll_t async_refr_finish_ll = { 0 };
static refr_event_cb_t ori_refr_start_cb = NULL;
static refr_event_cb_t ori_refr_finish_cb = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_res_t lvx_async_before_refr_call(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call(&async_refr_start_ll, async_xcb, user_data);
}

lv_res_t lvx_async_before_refr_call_cancel(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call_cancel(&async_refr_start_ll, async_xcb, user_data);
}

lv_res_t lvx_async_after_refr_call(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call(&async_refr_finish_ll, async_xcb, user_data);
}

lv_res_t lvx_async_after_refr_call_cancel(lv_async_cb_t async_xcb, void * user_data)
{
    return lvx_async_refr_call_cancel(&async_refr_finish_ll, async_xcb, user_data);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_res_t lvx_async_refr_call(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data)
{
    if(!async_inited) {
        lvx_async_refr_init();
    }

    async_refr_info_t * info = _lv_ll_ins_tail(ll_p);
    LV_ASSERT_MALLOC(info);

    if(info == NULL) {
        return LV_RES_INV;
    }

    info->cb = async_xcb;
    info->user_data = user_data;
    return LV_RES_OK;
}

static lv_res_t lvx_async_refr_call_cancel(lv_ll_t * ll_p, lv_async_cb_t async_xcb, void * user_data)
{
    if(!async_inited) {
        return LV_RES_INV;
    }

    lv_res_t res = LV_RES_INV;
    async_refr_info_t * info = _lv_ll_get_head(ll_p);

    while(info != NULL) {
        async_refr_info_t * info_next = _lv_ll_get_next(ll_p, info);

        if(info->cb == async_xcb && info->user_data == user_data) {
            _lv_ll_remove(ll_p, info);
            lv_mem_free(info);
            res = LV_RES_OK;
        }

        info = info_next;
    }

    return res;
}

static void on_refr_event(lv_ll_t * ll_p)
{
    if(_lv_ll_is_empty(ll_p)) {
        return;
    }

    async_refr_info_t * info;
    _LV_LL_READ(ll_p, info) {
        info->cb(info->user_data);
    }

    _lv_ll_clear(ll_p);
}

static void on_refr_start(lv_disp_drv_t * disp_drv)
{
    if (ori_refr_start_cb) {
        ori_refr_start_cb(disp_drv);
    }

    on_refr_event(&async_refr_start_ll);
}

static void on_refr_finish(lv_disp_drv_t * disp_drv)
{
    if (ori_refr_finish_cb) {
        ori_refr_finish_cb(disp_drv);
    }

    on_refr_event(&async_refr_finish_ll);
}

static void lvx_async_refr_init(void)
{
    lv_disp_t * disp = lv_disp_get_default();
    if(!disp) {
        LV_LOG_WARN("disp is NULL");
        return;
    }

    if(disp->driver->refr_start_cb) {
        LV_LOG_WARN("Override refr_start_cb registered callback function: %p",
                    disp->driver->refr_start_cb);

        ori_refr_start_cb = disp->driver->refr_start_cb;
    }

    if(disp->driver->refr_finish_cb) {
        LV_LOG_WARN("Override refr_finish_cb registered callback function: %p",
                    disp->driver->refr_finish_cb);

        ori_refr_finish_cb = disp->driver->refr_finish_cb;
    }

    disp->driver->refr_start_cb = on_refr_start;
    disp->driver->refr_finish_cb = on_refr_finish;

    _lv_ll_init(&async_refr_start_ll, sizeof(async_refr_info_t));
    _lv_ll_init(&async_refr_finish_ll, sizeof(async_refr_info_t));
    async_inited = true;
}