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

#ifdef CONFIG_UIKIT_INPUT_METHOD
#define UIKIT_INPUT_METHOD CONFIG_UIKIT_INPUT_METHOD
#else
#define UIKIT_INPUT_METHOD 0
#endif

#ifdef CONFIG_UIKIT_INPUT_METHOD_DEFAULT_TYPE
#define UIKIT_INPUT_METHOD_DEFAULT_TYPE CONFIG_UIKIT_INPUT_METHOD_DEFAULT_TYPE
#else
#define UIKIT_INPUT_METHOD_DEFAULT_TYPE 2
#endif

#endif

/*==================
 *  WIDGET STYLE
 *================*/

#endif // UIKIT_CONF_H
