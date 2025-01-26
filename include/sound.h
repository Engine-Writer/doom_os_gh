#ifndef TIMER_H
#define TIMER_H
#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stdint.h>


void sound_play_frequency(uint32_t frequency);
void sound_stop();

#endif