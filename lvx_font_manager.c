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

static bool lvx_font_create_core(lv_ft_info_t* newfont);

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

    /* Prepare the font base directory path */
#ifdef __NuttX__
    const char* exepath = FONT_LIB_PATH;
#else
    char exepath[PATH_MAX];
    size_t max_size = sizeof(exepath);
    uv_exepath(exepath, &max_size);
#endif /* __NuttX__ */

    char base_path[PATH_MAX];
    lv_snprintf(base_path, sizeof(base_path), "%s/font", exepath);

    /* Create font manager */
    g_font_manager = font_manager_create();

    /* Set font base path */
    font_manager_set_base_path(g_font_manager, base_path);
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

lv_font_t* lvx_font_create(const char* name, uint16_t size, LV_FT_FONT_STYLE style)
{
    lv_ft_info_t newfont;
    lv_memset_00(&newfont, sizeof(newfont));
    newfont.name = name;
    newfont.style = style;
    newfont.weight = size;

    if (!lvx_font_create_core(&newfont)) {
        return NULL;
    }

    return newfont.font;
}

void lvx_font_destroy(lv_font_t* delfont)
{
    if (delfont == NULL) {
        FONT_LOG_WARN("delfont is NULL");
        return;
    }

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

static bool lvx_font_create_core(lv_ft_info_t* newfont)
{
    LV_ASSERT_NULL(newfont);

    lvx_font_init();
    if (newfont == NULL || newfont->name == NULL || newfont->weight == 0) {
        FONT_LOG_ERROR("param error");
        return false;
    }

    uint32_t start = lv_tick_get();
    LV_UNUSED(start);

    lv_font_t* font = font_manager_create_font(g_font_manager, newfont);
    if (!font) {
        return false;
    }
    newfont->font = font;

#if FONT_USE_FONT_FAMILY
    lv_font_t* font_family = font_manager_create_font_family(g_font_manager, newfont);
    if (font_family) {
        font->fallback = font_family;
        FONT_LOG_INFO("%s(%d) add font-family success",
            newfont->name, newfont->weight);
    }
#endif /* FONT_USE_FONT_FAMILY */

    FONT_LOG_INFO("font[%p]: %s(%d) create success, cost %" LV_PRIu32 "ms",
        newfont->font, newfont->name, newfont->weight, lv_tick_elaps(start));

    return true;
}

#endif /* LVX_USE_FONT_MANAGER */
