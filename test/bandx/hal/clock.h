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
#ifndef __CLOCK_H
#define __CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct clock_value_s {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint16_t ms;
    uint16_t year;
    uint8_t month;
    uint8_t date;
    uint8_t week;
};

void clock_get_value(struct clock_value_s* clock_value);
void clock_set_value(struct clock_value_s* clock_value);

#ifdef __cplusplus
}
#endif

#endif
