#ifndef TIMER_H
#define TIMER_H
#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stdint.h>

void timer_set_pit_frequency(uint32_t frequency);
void timer(Registers* regs);

extern uint32_t timer_ticks;

#endif