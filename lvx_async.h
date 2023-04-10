/**
 * @file lvx_async.h
 *
 */

#ifndef LVX_ASYNC_H
#define LVX_ASYNC_H

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Call an asynchronous function before next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
lv_res_t lvx_async_before_refr_call(lv_async_cb_t async_xcb, void * user_data);

/**
 * Cancel calling the async function before the next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
lv_res_t lvx_async_before_refr_call_cancel(lv_async_cb_t async_xcb, void * user_data);

/**
 * Call an asynchronous function after next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
lv_res_t lvx_async_after_refr_call(lv_async_cb_t async_xcb, void * user_data);

/**
 * Cancel calling the async function after the next refresh.
 * @param async_xcb a callback which is the task itself.
 * @param user_data custom parameter
 */
lv_res_t lvx_async_after_refr_call_cancel(lv_async_cb_t async_xcb, void * user_data);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVX_ASYNC_H*/
