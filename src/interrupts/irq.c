#include "terminal.h"
#include "irq.h"
#include "pic.h"
#include "io.h"
#include <stddef.h>

#define PIC_REMAP_OFFSET        0x20

IRQHandler g_IRQHandlers[16];

void IRQ_Handler(Registers* regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    uint8_t pic_isr = PIC_ReadInServiceRegister();
    uint8_t pic_irr = PIC_ReadIrqRequestRegister();

    if (g_IRQHandlers[irq] != NULL) {
        // handle IRQ
        g_IRQHandlers[irq](regs);
    } else {
        terminal_printf("Unhandled IRQ %d  ISR=%x  IRR=%x...\n", irq, pic_isr, pic_irr);
    }

    // send EOI
    PIC_SendEndOfInterrupt(irq);
}

void IRQ_Initialize() {
    PIC_Configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8);

    // register ISR handlers for each of the 16 irq lines
    for (int i = 0; i < 16; i++)
        ISR_RegisterHandler(PIC_REMAP_OFFSET + i, IRQ_Handler);
}

void IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}