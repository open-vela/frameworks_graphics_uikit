/**
 * @file uikit_async.h
 *
 */

#ifndef UIKIT_ASYNC_H
#define UIKIT_ASYNC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct _vg_async_t {
    bool inited;
    lv_ll_t refr_start_ll;
    lv_ll_t refr_finish_ll;
} vg_async_t;

typedef lv_async_cb_t vg_async_cb_t;
typedef lv_result_t vg_async_result_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Deinit asynchronous refresh module.
 */
void vg_async_refr_deinit(void);

/**
 * Call an asynchronous function before next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
vg_async_result_t vg_async_before_refr_call(vg_async_cb_t async_xcb, void* user_data);

/**
 * Cancel calling the async function before the next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
vg_async_result_t vg_async_before_refr_call_cancel(vg_async_cb_t async_xcb, void* user_data);

/**
 * Call an asynchronous function after next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
vg_async_result_t vg_async_after_refr_call(vg_async_cb_t async_xcb, void* user_data);

/**
 * Cancel calling the async function after the next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
vg_async_result_t vg_async_after_refr_call_cancel(vg_async_cb_t async_xcb, void* user_data);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*UIKIT_ASYNC_H*/
