#include <stdbool.h>
#include <stdint.h>
#include "terminal.h"
#include "keyboard.h"
#include "idt.h"
#include "irq.h"
#include "isr.h"
#include "pic.h"
#include "io.h"

keyboard_t keyboard;
const uint8_t keyboard_layout_us[2][128] = {
    {
        KEY_NULL, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
        '-', '=', KEY_BACKSPACE, KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u',
        'i', 'o', 'p', '[', ']', KEY_ENTER, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j',
        'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
        ',', '.', '/', 0, 0, 0, ' ', 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
        KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0, 0, KEY_HOME, KEY_UP,
        KEY_PAGE_UP, '-', KEY_LEFT, '5', KEY_RIGHT, '+', KEY_END, KEY_DOWN,
        KEY_PAGE_DOWN, KEY_INSERT, KEY_DELETE, 0, 0, 0, KEY_F11, KEY_F12
    }, {
        KEY_NULL, KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
        '_', '+', KEY_BACKSPACE, KEY_TAB, 'Q', 'W',   'E', 'R', 'T', 'Y', 'U',
        'I', 'O', 'P',   '{', '}', KEY_ENTER, 0, 'A', 'S', 'D', 'F', 'G', 'H',
        'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
        'M', '<', '>', '?', 0, 0, 0, ' ', 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4,
        KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0, 0, KEY_HOME, KEY_UP,
        KEY_PAGE_UP, '-', KEY_LEFT, '5', KEY_RIGHT, '+', KEY_END, KEY_DOWN,
        KEY_PAGE_DOWN, KEY_INSERT, KEY_DELETE, 0, 0, 0, KEY_F11, KEY_F12
    }
};


// Function to send command to PS/2 keyboard
void keyboard_send_command(uint8_t cmd) {
    outb(KEYBOARD_CMD_PORT, cmd);
}

// Function to send data to PS/2 keyboard
void keyboard_send_data(uint8_t data) {
    outb(KEYBOARD_DATA_PORT, data);
}

// Function to read response from PS/2 keyboard
uint8_t keyboard_read_response() {
    return inb(KEYBOARD_DATA_PORT);
}

// Function to set the typematic rate and delay
uint8_t keyboard_set_typematic(keyboard_repeat_rate_t repeat_rate, keyboard_delay_time_t delay_time) {
    uint8_t typematic_byte = 0;

    // Set repeat rate (0x00 to 0x0B for 30 Hz to 2 Hz)
    typematic_byte |= (repeat_rate & REPEAT_RATE_MASK) << REPEAT_RATE_SHIFT;
    typematic_byte |= (delay_time & DELAY_MASK) << DELAY_SHIFT;
    typematic_byte &= ~UNUSED_BIT_MASK;

    // Send command to keyboard
    keyboard_send_command(CMD_TYPEMATIC_BYTE);
    keyboard_send_data(typematic_byte);
    iowait();

    // Read the response
    uint8_t response = keyboard_read_response();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to set LED states (ScrollLock, NumLock, CapsLock)
uint8_t keyboard_set_leds(keyboard_LEDState_t led_state) {
    keyboard_send_command(CMD_LED_STATE);
    keyboard_send_data((uint8_t)led_state);
    uint8_t response = keyboard_read_response();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to enable scanning (keyboard sends scan codes)
uint8_t keyboard_enable_scanning() {
    keyboard_send_command(CMD_ENABLE_SCANNING);
    uint8_t response = keyboard_read_response();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to reset keyboard (self-test)
uint8_t keyboard_reset() {
    keyboard_send_command(CMD_RESET);
    uint8_t response = keyboard_read_response();
    if (response == RES_ACK) {
        // Self-test passed
        response = keyboard_read_response(); // Read self-test result
        if (response == RES_SELF_TEST_PASSED) {
            return 0;
        } else if (response == RES_SELF_TEST_FAILED) {
            return 3;
        }
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to identify keyboard
uint8_t keyboard_identify() {
    keyboard_send_command(CMD_IDENTIFY_KEYBOARD);
    uint8_t response = keyboard_read_response();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Keyboard interrupt handler
void keyboard_hi(Registers *regs) {
    uint8_t scan_code = keyboard_read_response();  // Read the scan code from the keyboard input
    keyboard_handler(scan_code);
}

// Keyboard handler
void keyboard_handler(uint8_t scancode) {
    // Handle modifier keys
    if (KEY_SCANCODE(scancode) == KEY_LALT || KEY_SCANCODE(scancode) == KEY_RALT) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_ALT), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_LCTRL || KEY_SCANCODE(scancode) == KEY_RCTRL) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_CTRL), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_LSHIFT || KEY_SCANCODE(scancode) == KEY_RSHIFT) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_SHIFT), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_CAPS_LOCK) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_CAPS_LOCK), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_NUM_LOCK) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_NUM_LOCK), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_SCROLL_LOCK) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_SCROLL_LOCK), KEY_IS_PRESS(scancode));
    }

    bool was_on = keyboard.chars[KEY_CHAR(scancode)];

    // Update key state
    keyboard.keys[(uint8_t)(scancode & 0x7F)] = KEY_IS_PRESS(scancode);
    keyboard.chars[KEY_CHAR(scancode)] = KEY_IS_PRESS(scancode);

    if (keyboard.chars[KEY_CHAR(scancode)] && !was_on && \
    (KEY_CHAR(scancode) >= 32 && KEY_CHAR(scancode) <= 126)) {
        terminal_printf("You pressed %c \n", KEY_CHAR(scancode|keyboard.mods));
    } else if (!keyboard.chars[KEY_CHAR(scancode)] && was_on && \
    (KEY_CHAR(scancode) >= 32 && KEY_CHAR(scancode) <= 126)) {
        terminal_printf("You released %c \n", KEY_CHAR(scancode|keyboard.mods));
    }
}