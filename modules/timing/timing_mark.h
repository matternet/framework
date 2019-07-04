/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

typedef uint64_t time_mark_t; // Generic time_mark structure for holding a time.

void nsleep(uint32_t delay_ns);

uint64_t timing_get_time_us(void);
uint64_t timing_time_mark_in_us(time_mark_t *time_mark);
void timing_set_time_mark(time_mark_t *time_mark);
uint64_t timing_time_since_mark_us(time_mark_t *time_mark);
