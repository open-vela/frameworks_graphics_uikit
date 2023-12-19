include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_VIDEO_EXAMPLE), y)
CSRCS += $(wildcard video/*.c)
endif

ifeq ($(CONFIG_ANIMATION_ENGINE_EXAMPLES), y)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/frameworks/graphics/animengine/examples
endif

PRIORITY = $(CONFIG_EXAMPLES_LVEXTDEMO_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_LVEXTDEMO_STACKSIZE)
MODULE = $(CONFIG_EXAMPLES_LVEXTDEMO)

PROGNAME += lvextdemo
MAINSRC += lv_ext_demos.c

include $(APPDIR)/Application.mk
