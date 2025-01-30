#ifndef SOUND_H
#define SOUND_H
#include "io.h"
#include <stdint.h>


void sound_play_frequency(uint32_t frequency);
void sound_stop();

#endif // SOUND_H