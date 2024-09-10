/*
 * Copyright (C) 2020 Xiaomi Corporation
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
#include "lv_obj_ext_func.h"

void lv_obj_set_opa_scale(lv_obj_t* obj, lv_opa_t opa)
{
    lv_obj_set_style_opa(obj, opa, 0);
}

lv_opa_t lv_obj_get_opa_scale(lv_obj_t* obj)
{
    return lv_obj_get_style_opa(obj, 0);
}

/**
 * @brief  Add animation to objects
 * @param  obj:Object address
 * @param  a:Animation controller address
 * @param  exec_cb:The address of the function that controls the properties of the object
 * @param  start:Start value of animation
 * @param  end:The end value of the animation
 * @param  time:Animation execution time
 * @param  delay:Delay time before the animation starts
 * @param  ready_cb:Animation end event callback
 * @param  path_cb:Animation curve
 * @retval None
 */
void lv_obj_add_anim(
    lv_obj_t* obj, lv_anim_t* a,
    lv_anim_exec_xcb_t exec_cb,
    int32_t start, int32_t end,
    uint16_t time,
    uint32_t delay,
    lv_anim_ready_cb_t ready_cb,
    lv_anim_path_cb_t path_cb)
{
    lv_anim_t anim_temp;

    if (a == NULL) {
        a = &anim_temp;

        /* INITIALIZE AN ANIMATION
         *-----------------------*/
        lv_anim_init(a);
    }

    /* MANDATORY SETTINGS
     *------------------*/

    /*Set the "animator" function*/
    lv_anim_set_exec_cb(a, exec_cb);

    /*Set the "animator" function*/
    lv_anim_set_var(a, obj);

    /*Length of the animation [ms]*/
    lv_anim_set_time(a, time);

    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(a, start, end);

    /* OPTIONAL SETTINGS
     *------------------*/

    /*Time to wait before starting the animation [ms]*/
    lv_anim_set_delay(a, delay);

    /*Set the path in an animation*/
    lv_anim_set_path_cb(a, path_cb);

    /*Set a callback to call when animation is ready.*/
    lv_anim_set_ready_cb(a, ready_cb);

    /*Set a callback to call when animation is started (after delay).*/
    lv_anim_set_start_cb(a, ready_cb);

    /* START THE ANIMATION
     *------------------*/
    lv_anim_start(a); /*Start the animation*/
}
