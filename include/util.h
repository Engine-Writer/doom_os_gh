#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
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


char* itoa(uint32_t value, char* str, uint32_t base);
int8_t strncmp(const char *str1, const char *str2, size_t n);

#endif // UTIL_H