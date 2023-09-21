/**
 * @file page.h
 *
 */

#ifndef PAGE_H
#define PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

#ifdef CONFIG_LVX_USE_DEMO_BANDX

#include "../resource/resource.h"

/*********************
 *      DEFINES
 *********************/

#define PAGE_CLASS_DEF(NAME)                         \
    const lv_fragment_class_t _page_##NAME##_cls = { \
                                                     .constructor_cb = on_page_construct,         \
                                                     .destructor_cb = on_page_destruct,           \
                                                     .attached_cb = on_page_attached,             \
                                                     .detached_cb = on_page_detached,             \
                                                     .create_obj_cb = on_page_create,             \
                                                     .obj_created_cb = on_page_created,           \
                                                     .obj_will_delete_cb = on_page_will_delete,   \
                                                     .obj_deleted_cb = on_page_deleted,           \
                                                     .event_cb = on_page_event,                   \
                                                     .instance_size = sizeof(page_ctx_t)          \
                                                   }

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void page_init(void);

lv_fragment_t * page_create(const char * name, void * arg);

bool page_push(lv_fragment_t * self, const char * name, void * arg);

void page_pop(lv_fragment_t * self);

/**********************
 *      MACROS
 **********************/
#define PAGE_VER_RES 368
#define PAGE_HOR_RES 194

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#endif /*CONFIG_LVX_USE_DEMO_BANDX*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*PAGE_H*/
