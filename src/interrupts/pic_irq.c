#include "terminal.h"
#include "pic_irq.h"
#include "pic.h"
#include "io.h"
#include <stddef.h>

#define PIC_REMAP_OFFSET        0x20

IRQHandler g_PICIRQHandlers[16];

void PIC_IRQ_Handler(Registers* regs) {
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    uint8_t pic_isr = PIC_ReadInServiceRegister();
    uint8_t pic_irr = PIC_ReadIrqRequestRegister();

    if (g_PICIRQHandlers[irq] != NULL) {
        // handle IRQ
        g_PICIRQHandlers[irq](regs);
    } else {
        terminal_printf("Unhandled IRQ %d  ISR=%x  IRR=%x...\n", irq, pic_isr, pic_irr);
    }

    // send EOI
    PIC_SendEndOfInterrupt(irq);
}

void PIC_IRQ_Initialize() {
    PIC_Configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8);

    // register ISR handlers for each of the 16 irq lines
    for (int i = 0; i < 16; i++)
        ISR_RegisterHandler(PIC_REMAP_OFFSET + i, PIC_IRQ_Handler);
}

void PIC_IRQ_RegisterHandler(int irq, IRQHandler handler) {
    g_PICIRQHandlers[irq] = handler;
}