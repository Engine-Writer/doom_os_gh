#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include "stdio.h"
#include "terminal.h"
#include "memory.h"
#include "util.h"
#include "io.h"

const char g_HexChars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void clrscr() {
    terminal_clear(terminal_color);
}

void scrollback(uint8_t lines) {
    for (uint8_t l=0; l<lines; ++l) {
        terminal_scroll();
    }
}

void printf_unsigned(uint32_t number, int32_t radix) {
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    if (number!=0) {
        do {
            uint32_t rem = number % radix;
            number /= radix;
            buffer[pos++] = g_HexChars[rem];
        } while (number > 0);
    }

    // print number in reverse order
    while (--pos >= 0) {
        terminal_putchar(buffer[pos]);
    }
}

void printf_signed(int32_t number, int32_t radix) {
    if (number < 0)
    {
        terminal_putchar('-');
        printf_unsigned(-number, radix);
    }
    else printf_unsigned(number, radix);
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch (state)
        {
            case PRINTF_STATE_NORMAL:
                switch (*fmt)
                {
                    case '%': {
                        state = PRINTF_STATE_LENGTH;
                        break;
                    }
                    default:
                        terminal_putchar(*fmt);
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH: {
                switch (*fmt) {
                    case 'h': {
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        break;
                    }
                    case 'l': {
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        break;
                    }
                    default:  goto PRINTF_STATE_SPEC_;
                }
                break;
            }

            case PRINTF_STATE_LENGTH_SHORT: {
                if (*fmt == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;
            }

            case PRINTF_STATE_LENGTH_LONG: {
                if (*fmt == 'l') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;
            }

            case PRINTF_STATE_SPEC: {
            PRINTF_STATE_SPEC_:
                switch (*fmt) {
                    case 'c': {
                        terminal_putchar((char)va_arg(args, int));
                        break;
                    }
                    case 's': {  
                        terminal_writestring(va_arg(args, const char*));
                        break;
                    }
                    case '%': {
                        terminal_putchar('%');
                        break;
                    }
                    case 'd':
                    case 'i': {
                        radix = 10; sign = true; number = true;
                        break;
                    }

                    case 'u': {  
                        radix = 10; sign = false; number = true;
                        break;
                    }
                    case 'X':
                    case 'x':
                    case 'p': {
                        radix = 16; sign = false; number = true;
                        break;
                    }

                    case 'o': {
                        radix = 8; sign = false; number = true;
                        break;
                    }

                    // ignore invalid spec
                    default:    break;
                }

                if (number) {
                    if (sign) {
                        switch (length) {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     printf_signed(va_arg(args, int), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG:        printf_signed(va_arg(args, long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   printf_signed(va_arg(args, long long), radix);
                                                        break;
                        }
                    } else {
                        switch (length) {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     printf_unsigned(va_arg(args, unsigned int), radix);
                                                        break;
                                                        
                        case PRINTF_LENGTH_LONG:        printf_unsigned(va_arg(args, unsigned  long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   printf_unsigned(va_arg(args, unsigned  long long), radix);
                                                        break;
                        }
                    }
                }

                // reset state
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
            }
        }

        fmt++;
    }

    va_end(args);
}

void print_buffer(const char* msg, const void* buffer, uint32_t count)
{
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    
    terminal_writestring(msg);
    for (uint16_t i = 0; i < count; i++)
    {
        terminal_putchar(g_HexChars[u8Buffer[i] >> 4]);
        terminal_putchar(g_HexChars[u8Buffer[i] & 0xF]);
    }
    terminal_writestring("\n");
}

void putc(char c) {
    terminal_putchar(c);
}

void puts(const char *s) {
    terminal_writestring(s);
}