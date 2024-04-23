# ##############################################################################
# apps/graphics/lvgl/ext/CMakeLists.txt
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES or CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(CONFIG_LVGL_EXTENSION)
  set(INCDIR ${CMAKE_CURRENT_LIST_DIR})
  set(CSRCS)

  if(CONFIG_LVX_USE_FONT_MANAGER)
    file(GLOB CSRCS_FONT font_manager/*.c)
    list(APPEND CSRCS_FONT lvx_font_manager.c)
    list(APPEND CSRCS_FONT lvx_font_stress.c)
    list(APPEND CSRCS ${CSRCS_FONT})
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/netutils/cjson/cJSON)
  endif()

  if(CONFIG_LVX_USE_VIDEO_ADAPTER)
    file(GLOB CSRCS_VIDEO_ADAPTER video/*.c)
    list(APPEND CSRCS ${CSRCS_VIDEO_ADAPTER})
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/frameworks/media/include)
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/netutils/cjson/cJSON)
  endif()

  if(CONFIG_LVX_USE_ANIMENGINE_ADAPTER)
    file(GLOB CSRCS_ANIMENGINE_ADAPTER animengine/*.c)
    list(APPEND CSRCS ${CSRCS_ANIMENGINE_ADAPTER})
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/frameworks/graphics/animengine/include)
  endif()

  if(CONFIG_LVX_USE_QRSCAN)
    file(GLOB CSRCS_QRSCAN qrscan/*.c)
    list(APPEND CSRCS ${CSRCS_QRSCAN})
  endif()

  file(GLOB CSRCS_DRAW draw/shapes/*.c)
  list(APPEND CSRCS_DRAW lvx_async.c)
  list(APPEND CSRCS_DRAW lv_ext.c)
  list(APPEND CSRCS ${CSRCS_DRAW})

  # ##########################################################################
  # Library Configuration
  # ##########################################################################

  nuttx_add_library(lvglext STATIC)
  target_sources(lvglext PRIVATE ${CSRCS})
  target_link_libraries(lvglext lvgl)
  target_include_directories(lvglext PRIVATE ${INCDIR})
  nuttx_export_header(TARGET lvglext INCLUDE_DIRECTORIES ${INCDIR})

  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
endif()
