/*
 * ext/animengine/lvx_animengine_adapter.h
 *
 * Copyright (C) 2023 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LVX_ANIMATION_ENGINE_ADAPTER_H__
#define __LVX_ANIMATION_ENGINE_ADAPTER_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "lv_ext_conf.h"
#include <lvgl/lvgl.h>

#if LVX_USE_ANIMENGINE_ADAPTER
#include <anim_api.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

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
 * Name: lvx_anim_adapter_init
 *
 * Description:
 *   animation engine adapter init.
 * return:
 *      animation engine handle
 ****************************************************************************/

anim_engine_handle_t lvx_anim_adapter_init(void);

/****************************************************************************
 * Name: lvx_anim_adapter_uninit
 *
 * Description:
 *   animation engine adapter uninit.
 *
 * Param:
 *      animation engine handle
 ****************************************************************************/

void lvx_anim_adapter_uninit(anim_engine_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* LVX_USE_ANIMENGINE_ADAPTER */

#endif /* __LVX_ANIMATION_ENGINE_ADAPTER_H__ */
