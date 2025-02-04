#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>
#include "glm.h"

typedef struct {
    uint8_t x;
    uint8_t y;
} uint8_Vector2_t;

typedef struct {
    uint16_t x;
    uint16_t y;
} uint16_Vector2_t;
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} uint8_Vector3_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} uint16_Vector3_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;



static inline color_t make_svga_color(uint8_t r, uint8_t g, uint8_t b) { 
    color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    return color;
}

static inline uint8_Vector2_t make_uint8_vector2(uint8_t x, uint8_t y) { 
    uint8_Vector2_t vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

static inline uint16_Vector2_t make_uint16_vector2(uint16_t x, uint16_t y) { 
    uint16_Vector2_t vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

static inline uint8_Vector3_t make_uint8_vector3(uint8_t x, uint8_t y, uint8_t z) { 
    uint8_Vector3_t vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

static inline uint16_Vector3_t make_uint16_vector3(uint16_t x, uint16_t y, uint8_t z) { 
    uint16_Vector3_t vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    return vec;
}


char* itoa(uint32_t value, char* str, uint32_t base);

int8_t strncmp(const char *str1, const char *str2, size_t n);
size_t strlen(const char *str);
size_t strlcat(char *dst, const char *src, size_t size);
size_t strlcpy(char *dst, const char *src, size_t n);

uint16_Vector3_t rotate_point_x(uint16_Vector3_t point, float angle);
uint16_Vector3_t rotate_point_y(uint16_Vector3_t point, float angle);

#endif // UTIL_H