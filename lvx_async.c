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

typedef struct {
    lv_async_cb_t cb;
    void * user_data;
} async_refr_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_async_refr_init(void);

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_ll_t async_refr_ll = { 0 };

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_res_t lvx_async_after_refr_call(lv_async_cb_t async_xcb, void * user_data)
{
    if(async_refr_ll.n_size == 0) {
        lvx_async_refr_init();
    }

    async_refr_info_t * info = _lv_ll_ins_tail(&async_refr_ll);
    LV_ASSERT_MALLOC(info);

    if(info == NULL) {
        return LV_RES_INV;
    }

    info->cb = async_xcb;
    info->user_data = user_data;
    return LV_RES_OK;
}

lv_res_t lvx_async_after_refr_call_cancel(lv_async_cb_t async_xcb, void * user_data)
{
    if(async_refr_ll.n_size == 0) {
        return LV_RES_INV;
    }

    lv_res_t res = LV_RES_INV;
    async_refr_info_t * info = _lv_ll_get_head(&async_refr_ll);

    while(info != NULL) {
        async_refr_info_t * info_next = _lv_ll_get_next(&async_refr_ll, info);

        if(info->cb == async_xcb && info->user_data == user_data) {
            _lv_ll_remove(&async_refr_ll, info);
            lv_mem_free(info);
            res = LV_RES_OK;
        }

        info = info_next;
    }

    return res;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void on_refr_finish(lv_disp_drv_t * disp_drv)
{
    if(_lv_ll_is_empty(&async_refr_ll)) {
        return;
    }

    async_refr_info_t * info;
    _LV_LL_READ(&async_refr_ll, info) {
        info->cb(info->user_data);
    }

    _lv_ll_clear(&async_refr_ll);
}

static void lvx_async_refr_init(void)
{
    lv_disp_t * disp = lv_disp_get_default();
    if(!disp) {
        LV_LOG_WARN("disp is NULL");
        return;
    }

    if(disp->driver->refr_finish_cb) {
        LV_LOG_WARN("Override refr_finish_cb registered callback function: %p",
                    disp->driver->refr_finish_cb);
    }

    disp->driver->refr_finish_cb = on_refr_finish;

    _lv_ll_init(&async_refr_ll, sizeof(async_refr_info_t));
}
