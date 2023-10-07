/**
 * @file page.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "page.h"

#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

#define PAGE_STYLE_MATCH(NAME)          \
    do {                                \
        if (strcmp(name, #NAME) == 0) { \
            return &g_style_grp.NAME;   \
        }                               \
    } while (0)

/**********************
 *      TYPEDEFS
 **********************/

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

void page_init(void)
{
}

lv_fragment_t * page_create(const char * name, void * arg)
{
    LV_LOG_USER("name: %s arg: %p", name, arg);

#define PAGE_DEF(NAME)                                           \
    do {                                                         \
        if (strcmp(name, #NAME) == 0) {                          \
            extern const lv_fragment_class_t _page_##NAME##_cls; \
            return lv_fragment_create(&_page_##NAME##_cls, arg); \
        }                                                        \
    } while (0);
#include "page.inc"
#undef PAGE_DEF

    LV_LOG_WARN("NO match for %s", name);
    return NULL;
}

bool page_push(lv_fragment_t * self, const char * name, void * arg)
{
    lv_fragment_t * page = page_create(name, NULL);

    if(!page) {
        LV_LOG_WARN("page push %s failed", name);
        return false;
    }

    lv_fragment_manager_t * manager = lv_fragment_get_manager(self);
    lv_obj_t * const * container = lv_fragment_get_container(self);
    lv_fragment_manager_push(manager, page, container);
    return true;
}

void page_pop(lv_fragment_t * self)
{
    lv_fragment_manager_t * manager = lv_fragment_get_manager(self);
    lv_fragment_manager_pop(manager);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/