#include <stddef.h>
#include <stdint.h>
#include "util.h"
#include "math.h"
#include "memory.h"

// Function to convert an integer to a string
char* itoa(uint32_t value, char* str, uint32_t base) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // Handle negative numbers for base 10
    if (value < 0 && base == 10) {
        isNegative = 1;
        value = -value;  // Make the value positive
    }

    // Process digits
    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';  // Convert to char
        value = value / base;
    }

    // Add negative sign if necessary
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';  // Null-terminate the string

    // Reverse the string to get the correct order
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}

int8_t strncmp(const char *str1, const char *str2, size_t n) {
    while (n-- > 0) {
        if (*str1 != *str2) {
            return (unsigned char)(*str1) - (unsigned char)(*str2);
        }
        if (*str1 == '\0') {
            return 0;  // Return 0 if both strings are identical up to the null terminator
        }
        str1++;
        str2++;
    }
    return 0;  // Return 0 if strings are equal up to n characters
}


size_t strlen(const char *str) {
    size_t l = 0;
    while (*str++ != 0) {
        l++;
    }
    return l;
}

size_t strlcat(char *dst, const char *src, size_t size) {
    const size_t sl = strlen(src),
          dl = strlen(dst);

    if (dl == size) {
        return size + sl;
    }

    if (sl < (size - dl)) {
        memcpy(dst + dl, src, sl + 1);
    } else {
        memcpy(dst + dl, src, size - dl - 1);
        dst[size - 1] = '\0';
    }

    return sl + dl;
}

size_t strlcpy(char *dst, const char *src, size_t n) {
    // copy as many bytes as can fit
    char *d = dst;
    const char *s = src;
    size_t size = n;

    while (--n > 0) {
        if ((*d++ = *s++) == 0) {
            break;
        }
    }

    // if we ran out of space, null terminate
    if (n == 0) {
        if (size != 0) {
            *d = 0;
        }

        // traverse the rest of s
        while (*s++);
    }

    return s - src - 1;
}