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
#define LVX_USE_BTN 0
#endif
#endif

#ifndef LVX_USE_RADIO
#ifdef CONFIG_LVX_USE_RADIO
#define LVX_USE_RADIO CONFIG_LVX_USE_RADIO
#else
#define LVX_USE_RADIO 0
#endif
#endif

#ifndef LVX_USE_SWITCH
#ifdef CONFIG_LVX_USE_SWITCH
#define LVX_USE_SWITCH CONFIG_LVX_USE_SWITCH
#else
#define LVX_USE_SWITCH 0
#endif
#endif

#ifndef LVX_USE_PICKER
#ifdef CONFIG_LVX_USE_PICKER
#define LVX_USE_PICKER CONFIG_LVX_USE_PICKER
#else
#define LVX_USE_PICKER 0
#endif
#endif

#ifndef LVX_USE_MSGBOX
#ifdef CONFIG_LVX_USE_MSGBOX
#define LVX_USE_MSGBOX CONFIG_LVX_USE_MSGBOX
#else
#define LVX_USE_MSGBOX 0
#endif
#endif

#ifndef LVX_USE_ARC_SCROLLBAR
#ifdef CONFIG_LVX_USE_ARC_SCROLLBAR
#define LVX_USE_ARC_SCROLLBAR CONFIG_LVX_USE_ARC_SCROLLBAR
#else
#define LVX_USE_ARC_SCROLLBAR 0
#endif
#endif

#ifndef LVX_USE_CHART
#ifdef CONFIG_LVX_USE_CHART
#define LVX_USE_CHART CONFIG_LVX_USE_CHART
#else
#define LVX_USE_CHART 0
#endif
#endif

#ifndef LVX_USE_FONT_MANAGER
#ifdef CONFIG_LVX_USE_FONT_MANAGER
#define LVX_USE_FONT_MANAGER CONFIG_LVX_USE_FONT_MANAGER
#else
#define LVX_USE_FONT_MANAGER 0
#endif
#endif

#if LVX_USE_FONT_MANAGER
#ifndef FONT_LIB_PATH
#ifdef CONFIG_FONT_LIB_PATH
#define FONT_LIB_PATH CONFIG_FONT_LIB_PATH
#else
#define FONT_LIB_PATH "./"
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



#endif

/*==================
 *  WIDGET STYLE
 *================*/
#define WIDGET_BG_OPA_NORMOL           LV_OPA_100
#define WIDGET_BG_OPA_PRESSED          LV_OPA_70
#define WIDGET_BG_OPA_DISABLE          LV_OPA_40

#define WIDGET_BG_COLOR_NORMOL(color)     color
#define WIDGET_BG_COLOR_PRESSED(color)    lv_color_mix(color, lv_color_black(), WIDGET_BG_OPA_PRESSED)
#define WIDGET_BG_COLOR_DISABLE(color)    lv_color_mix(color, lv_color_black(), WIDGET_BG_OPA_DISABLE)

#define SYSTEM_COLOR_BLUE                 lv_color_hex(0x0D84FF)
#define SYSTEM_COLOR_TEXT_BLUE            lv_color_hex(0x1A8AFF)
#define SYSTEM_COLOR_RED                  lv_color_hex(0xFF2626)
#define SYSTEM_COLOR_DEFAULT              lv_color_hex(0x000000)
#define SYSTEM_COLOR_BTN_GREY             lv_color_hex(0x333333)
#define SYSTEM_COLOR_WHITE                lv_color_hex(0xFFFFFF)

#if LVX_USE_BTN
#define BTN_DEFAULT_WIDTH                 110
#define BTN_DEFAULT_HEIGHT                110
#endif

#if LVX_USE_RADIO
#define RADIO_BG_COLOR_PRESSED(color)    WIDGET_BG_COLOR_PRESSED(color)
#define RADIO_BG_COLOR_DISABLE(color)    WIDGET_BG_COLOR_DISABLE(color)
#define RADIO_DEFAULT_WIDTH              56
#define RADIO_DEFAULT_HEIGHT             56
/** unchecked */
#define RADIO_BG_COLOR_UNCHECKED    lv_color_hex(0x1F1F1F)
#define RADIO_BORDER_COLOR          lv_color_hex(0xFFFFFF)
#define RADIO_BORDER_WIDTH          3
#define RADIO_BORDER_OPA            LV_OPA_60
#define RADIO_BORDER_OPA_PRESSED    LV_OPA_40
#define RADIO_BORDER_OPA_DISABLE    LV_OPA_20

/** checked */
#define RADIO_BG_COLOR_CHECKED      SYSTEM_COLOR_BLUE
#define RADIO_BORDER_WIDTH_CHECKED  0
#define RADIO_BORDER_OPA_CHECKED    LV_OPA_0

/** checked and pressed */
#define RADIO_BG_COLOR_CHECKED_PRESSED      RADIO_BG_COLOR_PRESSED(RADIO_BG_COLOR_CHECKED)

/** checked and disable */
#define RADIO_BG_COLOR_CHECKED_DISABLE      RADIO_BG_COLOR_DISABLE(RADIO_BG_COLOR_CHECKED)
#endif

#if LVX_USE_SWITCH
#define SWITCH_BG_COLOR_DEFAULT           lv_color_hex(0x4C4C4C)
#define SWITCH_BG_COLOR_CHECKED           SYSTEM_COLOR_BLUE
#define SWITCH_DEFAULT_WIDTH              92
#define SWITCH_DEFAULT_HEIGHT             52
#define SWITCH_KNOB_OFFSET                13
#define SWITCH_KNOB_COLOR                 lv_color_hex(0xFFFFFF)
#endif

#if LVX_USE_PICKER
#define PICKER_PAD_ALL                     10
#define PICKER_PAD_GAP                     10

#define PICKER_TEXT_FONT_DEFAULT           LV_FONT_DEFAULT
#define PICKER_TEXT_COLOR_DEFAULT          SYSTEM_COLOR_WHITE
#define PICKER_TEXT_OPA_DEFAULT            LV_OPA_40
#define PICKER_TEXT_ALIGN_DEFAULT          LV_TEXT_ALIGN_CENTER

#define PICKER_TEXT_FONT_SELECTED          LV_FONT_DEFAULT
#define PICKER_TEXT_COLOR_SELECTED         SYSTEM_COLOR_WHITE
#define PICKER_TEXT_OPA_SELECTED           LV_OPA_COVER
#define PICKER_TEXT_ALIGN_SELECTED         LV_TEXT_ALIGN_CENTER
#endif

#if LVX_USE_MSGBOX
#define MSGBOX_TEXT_FONT_DEFAULT           LV_FONT_DEFAULT
#define MSGBOX_TEXT_COLOR_DEFAULT          SYSTEM_COLOR_WHITE
#define MSGBOX_TEXT_OPA_DEFAULT            LV_OPA_COVER
#define MSGBOX_TEXT_ALIGN_DEFAULT          LV_TEXT_ALIGN_CENTER

#define MSGBOX_TEXT_FONT_ITEMS             LV_FONT_DEFAULT
#define MSGBOX_TEXT_COLOR_ITEMS            SYSTEM_COLOR_WHITE
#define MSGBOX_TEXT_OPA_ITEMS              LV_OPA_COVER
#define MSGBOX_TEXT_ALIGN_ITEMS            LV_TEXT_ALIGN_LEFT
#endif

#if LVX_USE_ARC_SCROLLBAR
#define ARC_SCROLLBAR_COLOR(color)          lv_color_mix(color, lv_color_black(), LV_OPA_20)
#define ARC_SCROLLBAR_INDIC_COLOR(color)    lv_color_mix(color, lv_color_black(), LV_OPA_80)
#define ARC_SCROLLBAR_WITH                  10
#endif

#endif // LV_EXT_CONF_H
