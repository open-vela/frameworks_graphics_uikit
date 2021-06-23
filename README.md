# lvgl extensions

## Usage

1. Set CONFIG_LVGL_EXTENSION=y(Application Configuration--->Graphics Support--->Light and...(LVGL)--->LVGL extension--->Enable LVGL extension), import with `#include <lv_ext.h>`.

2. If you want to use lvx_widgets, you need to set CONFIG_LVX_USE_WIDGETS=y(Enable widgets extension) and set CONFIG_LVX_USE_THEME_EXTENDED=y, then enable specific widgets. At the same time, set the theme when registering the screen, as follows.:
==========================================================================
    lv_disp_t* disp = NULL;
#if defined(CONFIG_LV_USE_LCDDEV_INTERFACE)
    disp = lv_lcddev_interface_init(NULL, 0);
#endif

#if defined(CONFIG_LV_USE_FBDEV_INTERFACE)
    disp = lv_fbdev_interface_init(NULL, 0);
#endif

#if LVX_USE_THEME_EXTENDED != 0
    /* Use theme extantions */
    if (disp != NULL) {
        /* Extantion theme */
        lv_theme_t* parent_theme = lv_theme_get_from_obj(NULL);
        if (parent_theme) {
            lv_theme_t* th = lvx_theme_extended_init(disp);
            lv_theme_set_parent(th, parent_theme);
            lv_disp_set_theme(disp, th);
        }
    }
#endif
==========================================================================
