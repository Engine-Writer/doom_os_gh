#ifndef MATH_H
#define MATH_H
#include <stdint.h>
#include "util.h"

#define E 2.71828
#define PI 3.14159265358979323846264338327950


static inline uint32_t abs32(uint32_t x) {
    return x < 0 ? -x : x;
}

static inline uint16_t abs16(uint16_t x) {
    return x < 0 ? -x : x;
}

static inline uint8_t abs8(uint8_t x) {
    return x < 0 ? -x : x;
}

// Helper functions for min and max.
static inline uint32_t min32(int a, int b) {
    return (a < b) ? a : b;
}
static inline uint32_t max32(int a, int b) {
    return (a > b) ? a : b;
}

// Compute the x coordinate where the horizontal line at y intersects the edge (a, b).
static inline int compute_intersection_x(uint16_Vector2_t a, uint16_Vector2_t b, int y) {
    if (a.y == b.y)
        return a.x; // Horizontal edge – return one endpoint.
    float t = (float)(y - a.y) / (float)(b.y - a.y);
    return a.x + (int)((b.x - a.x) * t);
}

double fmod(double x, double m);
double fabs(double x);
double sin(double x);
double cos(double x);
double pow(double x, double y);

#endif // MATH_H