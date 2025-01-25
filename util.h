#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
typedef struct {
    uint8_t x;
    uint8_t y;
} uint8_Vector2;

static inline uint8_Vector2 make_uint8_vector2(uint8_t x, uint8_t y) { 
    uint8_Vector2 vec;
    vec.x = x;
    vec.y = y;
    return vec;
}


char* itoa(int value, char* str, int base);

#endif