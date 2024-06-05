include $(APPDIR)/Make.defs

ifeq ($(CONFIG_LVX_USE_VIDEO_EXAMPLE), y)
CSRCS += $(wildcard video/*.c)
endif

ifeq ($(CONFIG_LVX_USE_DEMO_BANDX), y)
CSRCS += $(shell find -L bandx -name "*.c")
endif

ifeq ($(CONFIG_ANIMATION_ENGINE_EXAMPLES), y)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/frameworks/graphics/animengine/examples
endif

ifeq ($(CONFIG_LVX_USE_VECTOR_DRAW_DEMOS), y)
CSRCS += $(shell find -L draw -name "*.c")
endif

ifeq ($(CONFIG_LVX_TIME_OBJ_CREATION), y)
CSRCS += $(shell find -L creation -name "*.c")
endif

ifeq ($(and $(CONFIG_SCHED_INSTRUMENTATION),$(CONFIG_LV_USE_PROFILER)), y)
CFLAGS += "-DLV_PROFILER_BEGIN=sched_note_beginex(NOTE_TAG_GRAPHICS, __func__)"
CFLAGS += "-DLV_PROFILER_END=sched_note_endex(NOTE_TAG_GRAPHICS, __func__)"

CFLAGS += "-DLV_PROFILER_BEGIN_TAG(str)=sched_note_beginex(NOTE_TAG_GRAPHICS, str)"
CFLAGS += "-DLV_PROFILER_END_TAG(str)=sched_note_endex(NOTE_TAG_GRAPHICS, str)"
endif

PRIORITY = $(CONFIG_EXAMPLES_LVEXTDEMO_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_LVEXTDEMO_STACKSIZE)
MODULE = $(CONFIG_EXAMPLES_LVEXTDEMO)

PROGNAME += lvxdemo
MAINSRC += lv_ext_demos.c

include $(APPDIR)/Application.mk
