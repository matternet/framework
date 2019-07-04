#include <common/ctor.h>
#include <ch.h>
#include <modules/timing/timing.h>
#include "timing_mark.h"

#define STM32_INSTRUCTION_CYCLE_TIME_nS 185

static volatile uint32_t instruction_cycles;

void nsleep(uint32_t delay_ns) {
    instruction_cycles = (delay_ns/STM32_INSTRUCTION_CYCLE_TIME_nS) + 1;
    while (instruction_cycles > 0) {
        instruction_cycles--;
    }
}

//static uint64 timing_base_time_us = 0;
uint64_t timing_get_time_us(void) {
    return micros64();
}

uint64_t timing_time_mark_in_us(time_mark_t *time_mark) {
    return *time_mark;
}

void timing_set_time_mark(time_mark_t *time_mark) {
    *time_mark = timing_get_time_us();
}

uint64_t timing_time_since_mark_us(time_mark_t *time_mark) {
    return timing_get_time_us() - timing_time_mark_in_us(time_mark);
}

