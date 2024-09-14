/**
 * @file uikit_font_manager.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "uikit_internal.h"

#if UIKIT_FONT_MANAGER
#include "font_manager/font_manager.h"
#include "font_manager/font_utils.h"
#include <string.h>

#ifndef __NuttX__
#include <uv.h>
#endif /* __NuttX__ */

/*********************
 *      DEFINES
 *********************/
#define g_font_manager VG_GLOBAL_DEFAULT()->font_manager

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static lv_font_t* vg_font_create_core(lv_freetype_info_t* newfont);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void vg_font_init(void)
{
    /* Avoid repeated initialization */
    if (g_font_manager) {
        return;
    }

    /* Create font manager */
    g_font_manager = font_manager_create();

    /* check old config temporary for miwear: CONFIG_FONT_LIB_PATH */
#ifdef CONFIG_FONT_LIB_PATH
    /* Set font base path */
    if (strcmp(CONFIG_FONT_LIB_PATH, "/data/") != 0) {
        LV_LOG_WARN("You are using obsolete config FONT_LIB_PATH (%s). Please switch to UIKIT_FONT_LIB_PATH", CONFIG_FONT_LIB_PATH);
        font_manager_set_base_path(g_font_manager, CONFIG_FONT_LIB_PATH "/font");
        return;
    }
#endif

    /* Set font base path */
    font_manager_set_base_path(g_font_manager, UIKIT_FONT_LIB_PATH "/font");
}

void vg_font_deinit(void)
{
    LV_ASSERT_NULL(g_font_manager);
    if (g_font_manager) {
        if (font_manager_delete(g_font_manager)) {
            g_font_manager = NULL;
        }
    }
}

void vg_font_set_base_path(const char* path)
{
    LV_ASSERT_NULL(path);
    font_manager_set_base_path(g_font_manager, path);
}

vg_font_path_handle_t vg_font_add_path(const char* name, const char* path)
{
    return font_manager_add_path(g_font_manager, name, path);
}

bool vg_font_remove_path(vg_font_path_handle_t handle)
{
    return font_manager_remove_path(g_font_manager, handle);
}

lv_font_t* vg_font_create(const char* name, uint16_t size, uint16_t style)
{
    lv_freetype_info_t newfont;
    lv_memzero(&newfont, sizeof(newfont));
    newfont.name = name;
    newfont.style = style;
    newfont.size = size;

    lv_font_t* font = vg_font_create_core(&newfont);

    if (!font) {
#ifdef CONFIG_UIKIT_FONT_USE_LV_FONT_DEFAULT
        LV_LOG_WARN("Use LV_FONT_DEFAULT(%p)", LV_FONT_DEFAULT);
        font = (lv_font_t*)LV_FONT_DEFAULT;
#endif /* CONFIG_UIKIT_FONT_USE_LV_FONT_DEFAULT */
    }

    return font;
}

void vg_font_destroy(lv_font_t* delfont)
{
    if (delfont == NULL) {
        LV_LOG_WARN("delfont is NULL");
        return;
    }

#ifdef CONFIG_UIKIT_FONT_USE_LV_FONT_DEFAULT
    if (delfont == LV_FONT_DEFAULT) {
        LV_LOG_INFO("delfont == LV_FONT_DEFAULT, no need to delete");
        return;
    }
#endif /* CONFIG_UIKIT_FONT_USE_LV_FONT_DEFAULT */

#if UIKIT_FONT_USE_FONT_FAMILY
    lv_font_t* font_family = (lv_font_t*)delfont->fallback;
    if (font_family) {
        font_manager_delete_font_family(g_font_manager, font_family);
        delfont->fallback = NULL;
    } else {
        LV_LOG_INFO("No fallback font detected");
    }
#endif /* UIKIT_FONT_USE_FONT_FAMILY */

    font_manager_delete_font(g_font_manager, delfont);

    LV_LOG_INFO("font[%p] destroy success", delfont);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_font_t* vg_font_create_core(lv_freetype_info_t* newfont)
{
    LV_ASSERT_NULL(newfont);

    vg_font_init();
    if (newfont == NULL || newfont->name == NULL || newfont->size == 0) {
        LV_LOG_ERROR("param error");
        return NULL;
    }

    uint32_t start = lv_tick_get();
    LV_UNUSED(start);

    lv_font_t* font = font_manager_create_font(g_font_manager, newfont);
    if (!font) {
        return NULL;
    }

#if UIKIT_FONT_USE_FONT_FAMILY
    lv_font_t* font_family = font_manager_create_font_family(g_font_manager, newfont);
    if (font_family) {
        font->fallback = font_family;
        LV_LOG_INFO("%s(%d) add font-family success",
            newfont->name, newfont->size);
    }
#endif /* UIKIT_FONT_USE_FONT_FAMILY */

    LV_LOG_INFO("font[%p]: %s(%d) create success, cost %" LV_PRIu32 "ms",
        font, newfont->name, newfont->size, lv_tick_elaps(start));

    return font;
}

#endif /* UIKIT_FONT_MANAGER */
