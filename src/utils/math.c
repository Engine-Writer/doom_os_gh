#include "math.h"
#include "util.h"
#include "glm.h"
#include <stdint.h>

uint32_t roundf(float number) {
    return (number >= 0.0f) ? (int)(number + 0.5f) : (int)(number - 0.5f);
}

double fabs(double x) {
    return x < 0.0 ? -x : x;
}

double fmod(double x, double y) {
    double result;
    asm (
        "1:\n\t"       // loop label
        "fprem1;"      // perform floating-point remainder operation
        "fnstsw %%ax;" // store the FPU status word in AX
        "sahf;"        // transfer status flags from AH to CPU flags
        "jp 1b;"       // if parity flag is set, jump back to label 1
        : "=t" (result)    // output: top of FPU stack goes into result
        : "0" (x), "u" (y) // inputs: 'x' is in the top of the FPU stack and 'y' in the other register
        : "ax", "cc"       // clobbered registers: AX and condition codes
    );
    return result;
}

float fmodf(float x, float y) {
    float result;
    asm (
        "1:\n\t"       // loop label
        "fprem1;"      // perform floating-point remainder operation
        "fnstsw %%ax;" // store the FPU status word in AX
        "sahf;"        // transfer status flags from AH to CPU flags
        "jp 1b;"       // if parity flag is set, jump back to label 1
        : "=t" (result)    // output: top of FPU stack goes into result
        : "0" (x), "u" (y) // inputs: 'x' is in the top of the FPU stack and 'y' in the other register
        : "ax", "cc"       // clobbered registers: AX and condition codes
    );
    return result;
}



double sin(double x) {
    double result;
    asm (
        "fldl %1;"    // Load x onto the FPU stack
        "fsin;"      // Compute sine of ST(0)
        "fstpl %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}


double cos(double x) {
    double result;
    asm (
        "fldl %1;"    // Load x onto the FPU stack
        "fcos;"      // Compute cosine of ST(0)
        "fstpl %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}

float sinf(float x) {
    float result;
    asm (
        "flds %1;"    // Load x onto the FPU stack
        "fsin;"      // Compute sine of ST(0)
        "fstps %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}


float cosf(float x) {
    float result;
    asm (
        "flds %1;"    // Load x onto the FPU stack
        "fcos;"      // Compute cosine of ST(0)
        "fstps %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}

double tan(double x) {
    x = fmod(x + PI, 2 * PI);
    if (x < 0)
        x += 2 * PI;
    // Shift to range -PI to PI
    x = x - PI;
    double cos_x = cos(x);

    // Define a small threshold to check for values close to zero
    const double epsilon = 1e-10;

    if (fabs(cos_x) < epsilon) {
        // Handle the case where cosine is too close to zero
        if (sin(x) > 0)
            return 1000000;
        else
            return -1000000;
    }

    return sin(x) / cos_x;
}

float tanf(float x) {
    x = fmod(x + PI, 2 * PI);
    if (x < 0)
        x += 2 * PI;
    // Shift to range -PI to PI
    x = x - PI;
    float cos_x = cosf(x);

    // Define a small threshold to check for values close to zero
    const float epsilon = 1e-10;

    if (fabs(cos_x) < epsilon) {
        // Handle the case where cosine is too close to zero
        if (sinf(x) > 0)
            return 1000000;
        else
            return -1000000;
    }

    return sinf(x) / cos_x;
}


// black magic
double pow(double x, double y) {
    double out;
    asm(
            "fyl2x;"
            "fld %%st;"
            "frndint;"
            "fsub %%st,%%st(1);"
            "fxch;"
            "fchs;"
            "f2xm1;"
            "fld1;"
            "faddp;"
            "fxch;"
            "fld1;"
            "fscale;"
            "fstp %%st(1);"
            "fmulp;" : "=t"(out) : "0"(x),"u"(y) : "st(1)" );
    return out;
}

uint16_Vector2_t convert_to_uint16_Vector2(Vector2 vec) {
    uint16_Vector2_t u16v2 = {
        (uint16_t)vec.x,
        (uint16_t)vec.y,
    };
    return u16v2;
}