#ifndef TIMER_H
#define TIMER_H
#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stdint.h>

void timer_set_pit_frequency(uint32_t frequency);
void timer_handler(Registers* regs);
void timer_init();

extern uint32_t timer_ticks;
extern uint32_t timer_frequency;
extern uint32_t timer_divisor;

#endif // TIMER_H