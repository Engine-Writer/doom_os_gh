#include <stdbool.h>
#include <stdint.h>
#include "terminal.h"
#include "keyboard.h"
#include "idt.h"
#include "isr.h"
#include "pic_irq.h"
#include "apic_irq.h"
#include "pic.h"
#include "apic.h"
#include "io.h"

#define KEYBOARD_IRQ_VECTOR         1
#define KEYBOARD_INTERRUPT_VECTOR   0x21
// IOAPIC base address and redirection entry for IRQ1 (keyboard)

#define IOAPIC_IRQ1_ENTRY     0x12  // Entry for IRQ1 in the Redirection Table
#define IOAPIC_IRQ1_VECTOR    0x21  // IRQ1 uses vector 0x21 on x86 systems (keyboard)
#define IOAPIC_DELIVERY_MODE  0x00000100  // Fixed delivery mode (normal interrupt)
#define IOAPIC_DESTINATION    0x00000000  // Send to all CPUs (this is usually specific for SMP systems)

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
void keyboard_handler(Registers *regs) {
    // terminal_writestring("IT WOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOORKKKKKS!!!!!!!!!!!!!!!!!");
    
    uint8_t scancode = keyboard_read_response();

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

void keyboard_pic_init() {
    PIC_IRQ_RegisterHandler(1, (IRQHandler)keyboard_handler);
    PIC_Unmask(1);
    terminal_printf("PIC Keyboard IRQ Initialized\n");
}

void keyboard_apic_init() {
    APIC_IRQ_RegisterHandler(1, (IRQHandler)keyboard_handler);
    IOAPIC_ConfigureKeyboard();
    terminal_printf("Keyboard Interrupt configuration complete!\n");
}

// Function to configure IOAPIC for keyboard interrupt (IRQ1)
void IOAPIC_ConfigureKeyboard() {
    uint32_t entry_low, entry_high;

    // Read current IRQ1 entry
    entry_low = APIC_ReadIO(IOAPIC_IRQ1_ENTRY);       // Lower 32 bits
    entry_high = APIC_ReadIO(IOAPIC_IRQ1_ENTRY + 1);  // Upper 32 bits

    // Configure lower 32-bit redirection entry
    entry_low &= ~0x000000FF;  // Reset APICINT
    entry_low = IOAPIC_IRQ1_VECTOR;  // Set interrupt vector (0x21)
    entry_low &= ~0x00000700;  // Reset trig flag
    entry_low |= APIC_DELIVERY_MODE_FIXED;  // Fixed delivery mode
    entry_low &= ~(1 << 15);  // Edge-triggered (bit 15 = 0 for edge)
    entry_low &= ~(1 << 13);  // Active high polarity (bit 13 = 0 for high)
    entry_low &= ~(1 << 16); // Enable interrupt (bit 16 = 0)

    // Configure upper 32-bit redirection entry (destination field)
    entry_high &= ~0xFF000000;  // Clear the destination field
    entry_high |= (0 << 24);   // Route interrupt to CPU 0 (bit 24 = 1)

    // Write the updated values back
    APIC_WriteIO(IOAPIC_IRQ1_ENTRY, entry_low);       // Write lower 32 bits
    APIC_WriteIO(IOAPIC_IRQ1_ENTRY + 1, entry_high);  // Write upper 32 bits

    terminal_printf("Configured IOAPIC for IRQ1 (keyboard) at 0x%x\n", apic_io_base);
}

// Function to mask the keyboard interrupt (IRQ1)
void IOAPIC_MaskIRQ1() {
    uint32_t entry_low, entry_high;

    // Step 1: Read the current entry for IRQ1 in the redirection table
    entry_low = APIC_ReadIO(IOAPIC_IRQ1_ENTRY);  // Lower 32 bits
    entry_high = APIC_ReadIO(IOAPIC_IRQ1_ENTRY + 1);  // Upper 32 bits

    // Step 2: Mask the interrupt (set the mask bit, bit 16 in the low entry)
    entry_low |= 0x00010000;  // Set the mask bit (bit 16)

    // Step 3: Write the updated entry back to the IOAPIC redirection table
    APIC_WriteIO(IOAPIC_IRQ1_ENTRY, entry_low);  // Write lower 32 bits
    APIC_WriteIO(IOAPIC_IRQ1_ENTRY + 1, entry_high);  // Write upper 32 bits

    terminal_printf("Masked IRQ1 (keyboard)\n");
}

