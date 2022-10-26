#ifndef LV_EXT_CONF_H
#define LV_EXT_CONF_H

#include <nuttx/config.h>

/*==================
 *  THEME EXTENDED
 *================*/
/* All extension widgets need to rely on this theme */

/*==================
 *  WIDGET USAGE
 *================*/

#ifndef LVX_USE_FONT_MANAGER
#ifdef CONFIG_LVX_USE_FONT_MANAGER
#define LVX_USE_FONT_MANAGER CONFIG_LVX_USE_FONT_MANAGER
#else
#define LVX_USE_FONT_MANAGER 0
#endif
#endif

#ifndef LVX_USE_RLE
#ifdef CONFIG_LVX_USE_RLE
#define LVX_USE_RLE CONFIG_LVX_USE_RLE
#else
#define LVX_USE_RLE 0
#endif
#endif

#ifndef LVX_USE_BIN_DECODER
#ifdef CONFIG_LVX_USE_BIN_DECODER
#define LVX_USE_BIN_DECODER CONFIG_LVX_USE_BIN_DECODER
#else
#define LVX_USE_BIN_DECODER 0
#endif
#endif

/*==================
 *  WIDGET STYLE
 *================*/

#endif  // LV_EXT_CONF_H
