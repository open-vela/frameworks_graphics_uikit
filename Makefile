############################################################################
# frameworks/graphics/uikit/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

CXXEXT := .cpp

ifneq ($(CONFIG_UIKIT),)

CSRCS += $(wildcard src/*.c)
CSRCS += $(wildcard src/draw/shapes/*.c)

ifeq ($(CONFIG_UIKIT_FONT_MANAGER), y)
CSRCS += $(wildcard src/font_manager/*.c)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/netutils/cjson/cJSON
endif

ifeq ($(CONFIG_UIKIT_VIDEO_ADAPTER), y)
CSRCS += $(wildcard src/video/*.c)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/frameworks/multimedia/media/include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/netutils/cjson/cJSON
endif

ifneq ($(CONFIG_UIKIT_QRSCAN),)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/external/quirc/quirc/lib
CSRCS += $(wildcard src/qrscan/*.c)
endif

ifneq ($(CONFIG_UIKIT_MARKDOWN),)
CSRCS += $(wildcard src/markdown/*.c)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/external/cmark-gfm/
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/external/cmark-gfm/cmark-gfm/src
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/external/cmark-gfm/cmark-gfm/extensions
endif

ifneq ($(CONFIG_UIKIT_ANIMATION_VIEW_PLAYER),)
CSRCS += $(wildcard src/draw/player/uikit_anim_view_player.c)
endif

ifneq ($(CONFIG_UIKIT_KEYBOARD),)
CSRCS += $(wildcard src/keyboard/*.c)
endif

ifneq ($(CONFIG_UIKIT_INPUT_METHOD),)
CSRCS += $(wildcard src/input/*.c)
endif

endif #CONFIG_UIKIT

ASRCS := $(wildcard $(ASRCS))
CSRCS := $(wildcard $(CSRCS))
CXXSRCS := $(wildcard $(CXXSRCS))
MAINSRC := $(wildcard $(MAINSRC))
NOEXPORTSRCS = $(ASRCS)$(CSRCS)$(CXXSRCS)$(MAINSRC)

ifneq ($(NOEXPORTSRCS),)
BIN := $(APPDIR)/staging/libuikit.a
endif

EXPORT_FILES := include test/Kconfig test/CMakeLists.txt test/Make.defs test/Makefile

include $(APPDIR)/Application.mk
