/**
 * @file lvx_font_manager.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lvx_font_manager.h"

#if (LVX_USE_FONT_MANAGER != 0)

#include "font_manager/font_log.h"
#include "font_manager/font_manager.h"
#include "font_manager/font_utils.h"

#ifndef __NuttX__
#include <uv.h>
#endif /* __NuttX__ */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static lv_font_t* lvx_font_create_core(lv_freetype_info_t* newfont);

/**********************
 *  STATIC VARIABLES
 **********************/

static font_manager_t* g_font_manager = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvx_font_init(void)
{
    /* Avoid repeated initialization */
    if (g_font_manager) {
        return;
    }

    /* Create font manager */
    g_font_manager = font_manager_create();

    /* Set font base path */
    font_manager_set_base_path(g_font_manager, FONT_LIB_PATH "/font");
}

void lvx_font_deinit(void)
{
    LV_ASSERT_NULL(g_font_manager);
    if (g_font_manager) {
        if (font_manager_delete(g_font_manager)) {
            g_font_manager = NULL;
        }
    }
}

void lvx_font_set_base_path(const char* path)
{
    LV_ASSERT_NULL(path);
    font_manager_set_base_path(g_font_manager, path);
}

lvx_font_path_handle_t lvx_font_add_path(const char* name, const char* path)
{
    return font_manager_add_path(g_font_manager, name, path);
}

bool lvx_font_remove_path(lvx_font_path_handle_t handle)
{
    return font_manager_remove_path(g_font_manager, handle);
}

lv_font_t* lvx_font_create(const char* name, uint16_t size, uint16_t style)
{
    lv_freetype_info_t newfont;
    lv_memset_00(&newfont, sizeof(newfont));
    newfont.name = name;
    newfont.style = style;
    newfont.size = size;

    lv_font_t* font = lvx_font_create_core(&newfont);

    if (!font) {
#ifdef CONFIG_FONT_USE_LV_FONT_DEFAULT
        FONT_LOG_WARN("Use LV_FONT_DEFAULT(%p)", LV_FONT_DEFAULT);
        font = (lv_font_t*)LV_FONT_DEFAULT;
#endif /* CONFIG_FONT_USE_LV_FONT_DEFAULT */
    }

    return font;
}

void lvx_font_destroy(lv_font_t* delfont)
{
    if (delfont == NULL) {
        FONT_LOG_WARN("delfont is NULL");
        return;
    }

#ifdef CONFIG_FONT_USE_LV_FONT_DEFAULT
    if (delfont == LV_FONT_DEFAULT) {
        FONT_LOG_INFO("delfont == LV_FONT_DEFAULT, no need to delete");
        return;
    }
#endif /* CONFIG_FONT_USE_LV_FONT_DEFAULT */

#if FONT_USE_FONT_FAMILY
    lv_font_t* font_family = (lv_font_t*)delfont->fallback;
    if (font_family) {
        font_manager_delete_font_family(g_font_manager, font_family);
        delfont->fallback = NULL;
    } else {
        FONT_LOG_INFO("No fallback font detected");
    }
#endif /* FONT_USE_FONT_FAMILY */

    font_manager_delete_font(g_font_manager, delfont);

    FONT_LOG_INFO("font[%p] destroy success", delfont);
}
/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_font_t* lvx_font_create_core(lv_freetype_info_t* newfont)
{
    LV_ASSERT_NULL(newfont);

    lvx_font_init();
    if (newfont == NULL || newfont->name == NULL || newfont->size == 0) {
        FONT_LOG_ERROR("param error");
        return NULL;
    }

    uint32_t start = lv_tick_get();
    LV_UNUSED(start);

    lv_font_t* font = font_manager_create_font(g_font_manager, newfont);
    if (!font) {
        return NULL;
    }

#if FONT_USE_FONT_FAMILY
    lv_font_t* font_family = font_manager_create_font_family(g_font_manager, newfont);
    if (font_family) {
        font->fallback = font_family;
        FONT_LOG_INFO("%s(%d) add font-family success",
            newfont->name, newfont->size);
    }
#endif /* FONT_USE_FONT_FAMILY */

    FONT_LOG_INFO("font[%p]: %s(%d) create success, cost %" LV_PRIu32 "ms",
        font, newfont->name, newfont->size, lv_tick_elaps(start));

    return font;
}

#endif /* LVX_USE_FONT_MANAGER */
