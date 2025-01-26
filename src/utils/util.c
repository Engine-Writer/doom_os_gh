#include <stddef.h>
#include <stdint.h>
#include "util.h"

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