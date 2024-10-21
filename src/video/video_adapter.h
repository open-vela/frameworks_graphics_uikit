/****************************************************************************
 * frameworks/graphics/uikit/video/video_adapter.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef UIKIT_VIDEO_ADAPTER_H
#define UIKIT_VIDEO_ADAPTER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

#if defined(CONFIG_UIKIT_VIDEO_ADAPTER)

/****************************************************************************
 * Type Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: vg_video_adapter_init
 *
 * Description:
 *   Video adapter init.
 *
 ****************************************************************************/

void vg_video_adapter_init(void);

/****************************************************************************
 * Name: vg_video_adapter_uninit
 *
 * Description:
 *   Video adapter deinit.
 *
 ****************************************************************************/

void vg_video_adapter_uninit(void);

/****************************************************************************
 * Name: vg_video_adapter_loop_init
 *
 * Description:
 *   Video adapter init ui uv loop.
 *
 ****************************************************************************/

void vg_video_adapter_loop_init(void* loop);

/****************************************************************************
 * Name: vg_video_adapter_loop_deinit
 *
 * Description:
 *   Video adapter deinit ui uv loop.
 *
 ****************************************************************************/

void vg_video_adapter_loop_deinit(void);

#endif /* CONFIG_UIKIT_VIDEO_ADAPTER */

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* UIKIT_VIDEO_ADAPTER_H */
