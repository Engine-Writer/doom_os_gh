#ifndef IRQ_H
#define IRQ_H
#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

extern IRQHandler g_PICIRQHandlers[16];

void PIC_IRQ_Initialize();
void PIC_IRQ_RegisterHandler(int irq, IRQHandler handler);
#endif // IRQ_H