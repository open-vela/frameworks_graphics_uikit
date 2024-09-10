include $(APPDIR)/Make.defs

ifeq ($(CONFIG_UIKIT_DEMO_VIDEO), y)
CSRCS += $(wildcard video/*.c)
endif

ifeq ($(CONFIG_UIKIT_DEMO_BANDX), y)
CSRCS += $(shell find -L bandx -name "*.c")
endif

ifeq ($(CONFIG_UIKIT_DEMO_VECTOR_DRAW), y)
CSRCS += $(shell find -L draw -name "*.c")
endif

ifeq ($(CONFIG_UIKIT_DEMO_TIME_OBJCREATION), y)
CSRCS += $(shell find -L creation -name "*.c")
endif

ifeq ($(CONFIG_UIKIT_DEMO_FONT_STRESS), y)
CSRCS += $(wildcard font/*.c)
endif

ifeq ($(and $(CONFIG_SCHED_INSTRUMENTATION),$(CONFIG_LV_USE_PROFILER)), y)
CFLAGS += "-DLV_PROFILER_BEGIN=sched_note_beginex(NOTE_TAG_GRAPHICS, __func__)"
CFLAGS += "-DLV_PROFILER_END=sched_note_endex(NOTE_TAG_GRAPHICS, __func__)"

CFLAGS += "-DLV_PROFILER_BEGIN_TAG(str)=sched_note_beginex(NOTE_TAG_GRAPHICS, str)"
CFLAGS += "-DLV_PROFILER_END_TAG(str)=sched_note_endex(NOTE_TAG_GRAPHICS, str)"
endif

PRIORITY = $(CONFIG_UIKIT_DEMO_PRIORITY)
STACKSIZE = $(CONFIG_UIKIT_DEMO_STACKSIZE)
MODULE = $(CONFIG_UIKIT_DEMO)

PROGNAME += uikit_demo
MAINSRC += uikit_demo.c

include $(APPDIR)/Application.mk
