############################################################################
# system/libuv/ext/Makefile
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

ifneq ($(CONFIG_LVGL_EXTENSION),)

ifeq ($(CONFIG_LVX_USE_RLE), y)
CSRCS += ext/rle/lvx_rle_decoder.c
endif

ifeq ($(CONFIG_LVX_USE_BIN_DECODER), y)
CSRCS += ext/bindecoder/lvx_bin_decoder.c
endif

ifeq ($(CONFIG_LVX_USE_FONT_MANAGER), y)
CSRCS += ext/lvx_font_manager.c
CSRCS += $(wildcard ext/font_manager/*.c)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/netutils/cjson/cJSON
endif

ifeq ($(CONFIG_LVX_USE_VIDEO_ADAPTER), y)
CSRCS += $(wildcard ext/video/*.c)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/frameworks/media/include
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/netutils/cjson/cJSON
endif

ifeq ($(CONFIG_LVX_USE_VIDEO_EXAMPLE), y)
CSRCS += $(wildcard ext/test/video/*.c)
endif

ifeq ($(CONFIG_LVX_USE_ANIMENGINE_ADAPTER), y)
CSRCS += $(wildcard ext/animengine/*.c)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/frameworks/graphics/animengine/include
endif

ifneq ($(CONFIG_LV_USE_QRSCAN),)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/graphics/lvgl/lvgl/src/extra/libs/qrscan
endif

CSRCS += ext/lvx_async.c
CSRCS += ext/lv_ext.c

endif #CONFIG_LVGL_EXTENSION
