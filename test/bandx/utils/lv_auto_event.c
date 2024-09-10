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
#include "lv_auto_event.h"

static void lv_auto_event_task_handler(lv_timer_t* task)
{
    lv_auto_event_t* ae = (lv_auto_event_t*)(task->user_data);

    lv_obj_send_event(
        *(ae->auto_event_data[ae->run_index].obj_p),
        ae->auto_event_data[ae->run_index].event,
        ae->auto_event_data[ae->run_index].user_data);

    ae->run_index++;

    if (ae->run_index < ae->len) {
        lv_timer_set_period(task, ae->auto_event_data[ae->run_index].delay);
    } else {
        lv_timer_del(task);
        lv_free(ae);
    }
}

lv_auto_event_t* lv_auto_event_create(lv_auto_event_data_t* auto_event_data, uint32_t len)
{
    lv_auto_event_t* ae = lv_malloc(sizeof(lv_auto_event_t));

    if (ae == NULL) {
        return NULL;
    }

    ae->auto_event_data = auto_event_data;
    ae->len = len;
    ae->run_index = 0;
    ae->task = lv_timer_create(lv_auto_event_task_handler, auto_event_data[0].delay, ae);
    return ae;
}

void lv_auto_event_del(lv_auto_event_t* ae)
{
    if (ae == NULL) {
        return;
    }

    lv_timer_del(ae->task);
    lv_free(ae);
}
