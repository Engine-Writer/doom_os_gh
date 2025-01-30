#ifndef TIMER_H
#define TIMER_H
#include "pic_irq.h"
#include "apic_irq.h"
#include "pic.h"
#include "apic.h"
#include "io.h"
#include <stdint.h>

// PIT functions
void timer_pic_set_pit_frequency(uint32_t frequency);
void pit_prepare_sleep(uint32_t microseconds);
uint16_t pit_read_counter();
void pit_perform_sleep();

// Timer Init functions
void timer_apic_init();
void timer_pic_init();

// Timer interrupt handler
void timer_handler(Registers* regs);

extern uint32_t timer_ticks;
extern uint32_t timer_frequency;
extern uint32_t timer_divisor;

#endif // TIMER_H