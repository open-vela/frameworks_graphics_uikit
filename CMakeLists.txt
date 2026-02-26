# ##############################################################################
# frameworks/graphics/uikit/CMakeLists.txt
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

if(CONFIG_UIKIT)
  nuttx_add_prebuilt(BIN_TARGETS uikit EXPORT_FILES include)

  set(INCDIR ${CMAKE_CURRENT_LIST_DIR}/include)
  set(CSRCS)

  if(CONFIG_UIKIT_FONT_MANAGER)
    file(GLOB CSRCS_FONT src/font_manager/*.c)
    list(APPEND CSRCS_FONT src/uikit_font_manager.c)
    list(APPEND CSRCS ${CSRCS_FONT})
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/netutils/cjson/cJSON)
  endif()

  if(CONFIG_UIKIT_VIDEO_ADAPTER)
    file(GLOB CSRCS_VIDEO_ADAPTER src/video/*.c)
    list(APPEND CSRCS ${CSRCS_VIDEO_ADAPTER})
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/frameworks/multimedia/media/include)
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/netutils/cjson/cJSON)
  endif()

  if(CONFIG_UIKIT_QRSCAN)
    file(GLOB CSRCS_QRSCAN src/qrscan/*.c)
    list(APPEND CSRCS ${CSRCS_QRSCAN})
  endif()

  if(CONFIG_UIKIT_MARKDOWN)
    file(GLOB CSRCS_MARKDOWN src/markdown/*.c)
    list(APPEND CSRCS ${CSRCS_MARKDOWN})
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/external/cmark-gfm)
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/external/cmark-gfm/cmark-gfm/src)
    list(APPEND INCDIR ${NUTTX_APPS_DIR}/external/cmark-gfm/cmark-gfm/extensions)
  endif()

  if(CONFIG_UIKIT_ANIMATION_VIEW_PLAYER)
    file(GLOB CSRCS_PLAYER src/draw/player/uikit_anim_view_player.c)
    list(APPEND CSRCS ${CSRCS_PLAYER})
  endif()

  if(CONFIG_UIKIT_KEYBOARD)
    file(GLOB CSRCS_KEYBOARD src/keyboard/*.c)
    list(APPEND CSRCS ${CSRCS_KEYBOARD})
  endif()

  if(CONFIG_UIKIT_INPUT_METHOD)
    file(GLOB CSRCS_INPUT src/input/*.c)
    list(APPEND CSRCS ${CSRCS_INPUT})
  endif()

  file(GLOB CSRCS_DRAW src/draw/shapes/*.c)
  list(APPEND CSRCS_DRAW src/uikit_async.c)
  list(APPEND CSRCS_DRAW src/uikit.c)
  list(APPEND CSRCS ${CSRCS_DRAW})

  # ############################################################################
  # Library Configuration
  # ############################################################################

  nuttx_library_ifndef(SKIP_PREBUILT uikit)
  nuttx_sources(${CSRCS})
  nuttx_include_directories(${INCDIR} ${DEPENDS_LIBS_INCDIR})

  set_property(
    TARGET nuttx
    APPEND
    PROPERTY NUTTX_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include)

  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
endif()
