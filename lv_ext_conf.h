#ifndef LV_EXT_CONF_H
#define LV_EXT_CONF_H

#include <nuttx/config.h>

/*==================
 *  THEME EXTENDED
 *================*/
/* All extension widgets need to rely on this theme */
#ifndef LVX_USE_THEME_EXTENDED
#ifdef CONFIG_LVX_USE_THEME_EXTENDED
#define LVX_USE_THEME_EXTENDED CONFIG_LVX_USE_THEME_EXTENDED
#else
#define LVX_USE_THEME_EXTENDED 0
#endif
#endif

/*==================
 *  WIDGET USAGE
 *================*/
#ifndef LVX_USE_BTN
#ifdef CONFIG_LVX_USE_BTN
#define LVX_USE_BTN CONFIG_LVX_USE_BTN
#else
#define CONFIG_LVX_USE_BTN 0
#endif
#endif

#endif // LV_EXT_CONF_H
