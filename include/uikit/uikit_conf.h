#ifndef UIKIT_CONF_H
#define UIKIT_CONF_H

#include <nuttx/config.h>

/*==================
 *  THEME EXTENDED
 *================*/
/* All extension widgets need to rely on this theme */

/*==================
 *  WIDGET USAGE
 *================*/

#ifndef UIKIT_FONT_MANAGER

#ifdef CONFIG_UIKIT_FONT_MANAGER
#define UIKIT_FONT_MANAGER CONFIG_UIKIT_FONT_MANAGER
#else
#define UIKIT_FONT_MANAGER 0
#endif

#ifdef CONFIG_UIKIT_VIDEO_ADAPTER
#define UIKIT_VIDEO_ADAPTER CONFIG_UIKIT_VIDEO_ADAPTER
#else
#define UIKIT_VIDEO_ADAPTER 0
#endif

#endif

/*==================
 *  WIDGET STYLE
 *================*/

#endif // UIKIT_CONF_H
