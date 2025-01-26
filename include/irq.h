#ifndef IRQ_H
#define IRQ_H
#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

void IRQ_Initialize();
void IRQ_RegisterHandler(int irq, IRQHandler handler);
#endif // IRQ_H