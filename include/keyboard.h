#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>
#include "abomination.h"
//#include "idt.h"
//#include "irq.h"
#include "isr.h"
//#include "pic.h"
#include "io.h"
#include "util.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CMD_PORT 0x64

// Define Command Bytes
#define CMD_LED_STATE 0xED
#define CMD_ECHO 0xEE
#define CMD_SUB_COMMAND 0xF0
#define CMD_IDENTIFY_KEYBOARD 0xF2
#define CMD_TYPEMATIC_BYTE 0xF3
#define CMD_ENABLE_SCANNING 0xF4
#define CMD_DISABLE_SCANNING 0xF5
#define CMD_SET_DEFAULT_PARAMS 0xF6
#define CMD_SET_TYPMATIC_AUTOREPEAT_ONLY 0xF7
#define CMD_SET_MAKE_RELEASE 0xF8
#define CMD_SET_MAKE_ONLY 0xF9
#define CMD_SET_TYPMATIC_AUTOREPEAT_MAKE_RELEASE 0xFA
#define CMD_SET_SPECIFIC_KEY_TYPMATIC 0xFB
#define CMD_SET_SPECIFIC_KEY_MAKE_RELEASE 0xFC
#define CMD_SET_SPECIFIC_KEY_MAKE_ONLY 0xFD
#define CMD_RESEND 0xFE
#define CMD_RESET 0xFF

// Define Response Bytes
#define RES_ACK 0xFA
#define RES_SELF_TEST_PASSED 0xAA
#define RES_SELF_TEST_FAILED 0xFC
#define RES_RESEND 0xFE

// Typematic byte bit fields
#define REPEAT_RATE_MASK 0x1F   // Bits 0-4 (Repeat Rate: 30 Hz to 2 Hz)
#define DELAY_MASK 0x60         // Bits 5-6 (Delay before key repeat: 250 ms, 500 ms, etc.)
#define UNUSED_BIT_MASK 0x80    // Bit 7 must be 0 (Unused)
#define REPEAT_RATE_SHIFT 0     // Shift for repeat rate (bits 0-4)
#define DELAY_SHIFT 5           // Shift for delay (bits 5-6)
#define UNUSED_BIT_SHIFT 7      // Shift for unused bit (bit 7)

// Typematic repeat rates (in Hz)
typedef enum {
    REPEAT_RATE_30HZ = 0x00,
    REPEAT_RATE_24HZ = 0x01,
    REPEAT_RATE_20HZ = 0x02,
    REPEAT_RATE_15HZ = 0x03,
    REPEAT_RATE_12HZ = 0x04,
    REPEAT_RATE_10HZ = 0x05,
    REPEAT_RATE_8HZ  = 0x06,
    REPEAT_RATE_6HZ  = 0x07,
    REPEAT_RATE_5HZ  = 0x08,
    REPEAT_RATE_4HZ  = 0x09,
    REPEAT_RATE_3HZ  = 0x0A,
    REPEAT_RATE_2HZ  = 0x0B
} keyboard_repeat_rate_t;

// Typematic delay options (in ms)
typedef enum {
    DELAY_250MS  = 0x00,
    DELAY_500MS  = 0x20,
    DELAY_750MS  = 0x40,
    DELAY_1000MS = 0x60
} keyboard_delay_time_t;

typedef enum {
    LED_NONE        = 0x00,
    LED_SCROLL_LOCK = 0x01,  // Scroll Lock LED
    LED_NUM_LOCK    = 0x02,  // Num Lock LED
    LED_CAPS_LOCK   = 0x04,  // Caps Lock LED
    LED_KANA_MODE   = 0x10   // Kana Mode LED (optional for international keyboards)
} keyboard_LEDState_t;

typedef struct {
    unsigned int scan_code;
    char key_char; // Use char for printable characters, or control characters for others
} keyboard_scancode_t;

// TODO: some of this it 100% wrong lmao
#define KEY_NULL 0
#define KEY_ESC 27
#define KEY_BACKSPACE '\b'
#define KEY_TAB '\t'
#define KEY_ENTER '\n'
#define KEY_RETURN '\r'

#define KEY_INSERT 0x90
#define KEY_DELETE 0x91
#define KEY_HOME 0x92
#define KEY_END 0x93
#define KEY_PAGE_UP 0x94
#define KEY_PAGE_DOWN 0x95
#define KEY_LEFT 0x4B
#define KEY_UP 0x48
#define KEY_RIGHT 0x4D
#define KEY_DOWN 0x50

#define KEY_F1 0x80
#define KEY_F2 (KEY_F1 + 1)
#define KEY_F3 (KEY_F1 + 2)
#define KEY_F4 (KEY_F1 + 3)
#define KEY_F5 (KEY_F1 + 4)
#define KEY_F6 (KEY_F1 + 5)
#define KEY_F7 (KEY_F1 + 6)
#define KEY_F8 (KEY_F1 + 7)
#define KEY_F9 (KEY_F1 + 8)
#define KEY_F10 (KEY_F1 + 9)
#define KEY_F11 (KEY_F1 + 10)
#define KEY_F12 (KEY_F1 + 11)

#define KEY_LCTRL 0x1D
#define KEY_RCTRL 0x1D

#define KEY_LALT 0x38
#define KEY_RALT 0x38

#define KEY_LSHIFT 0x2A
#define KEY_RSHIFT 0x36

#define KEY_CAPS_LOCK 0x3A
#define KEY_SCROLL_LOCK 0x46
#define KEY_NUM_LOCK 0x45

#define KEY_MOD_ALT 0x0200
#define KEY_MOD_CTRL 0x0400
#define KEY_MOD_SHIFT 0x0800
#define KEY_MOD_CAPS_LOCK 0x1000
#define KEY_MOD_NUM_LOCK 0x2000
#define KEY_MOD_SCROLL_LOCK 0x4000

#define KEYBOARD_RELEASE 0x80

#define KEYBOARD_BUFFER_SIZE 256


typedef struct {
    uint16_t mods;
    bool keys[128];
    bool chars[128];
} keyboard_t;


extern keyboard_t    keyboard; // Declare variable as extern
extern const uint8_t keyboard_layout_us[2][128]; // Declare array as extern


#define KEY_IS_PRESS(_s) (!((_s) & KEYBOARD_RELEASE))
#define KEY_IS_RELEASE(_s) (!!((_s) & KEYBOARD_RELEASE))
#define KEY_SCANCODE(_s) ((_s) & 0x7F)
#define KEY_MOD(_s, _m) (!!((_s) & (_m)))
#define KEY_CHAR(_s) __extension__({\
        __typeof__(_s) __s = (_s);\
        KEY_SCANCODE(__s) < 128 ?\
            keyboard_layout_us[KEY_MOD(__s, KEY_MOD_SHIFT) ? 1 : 0][KEY_SCANCODE(__s)] :\
            0;\
    })

#define keyboard_key(_s) (keyboard.keys[(_s)])
#define keyboard_char(_c) (keyboard.chars[(uint8_t) (_c)])


void keyboard_send_command(uint8_t cmd);
void keyboard_send_data(uint8_t data);
uint8_t keyboard_read_response();

uint8_t keyboard_set_typematic(keyboard_repeat_rate_t repeat_rate, keyboard_delay_time_t delay_time);
uint8_t keyboard_set_leds(keyboard_LEDState_t led_state);

uint8_t keyboard_enable_scanning();
uint8_t keyboard_reset();
uint8_t keyboard_identify();

void keyboard_handler(Registers *regs);
void keyboard_init();

#endif // KEYBOARD_H