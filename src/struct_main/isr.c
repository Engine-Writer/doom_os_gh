#include "isr.h"
#include "idt.h"
#include "gdt.h"
#include "io.h"
#include "terminal.h"
#include <stddef.h>

ISRHandler g_ISRHandlers[256];

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void ISR_InitializeGates();

void ISR_Initialize() {
    ISR_InitializeGates();
    for (int i = 0; i < 256; i++)
        IDT_EnableGate(i);
}

void __attribute__((cdecl)) ISR_Handler(Registers* regs) {
    if (g_ISRHandlers[regs->interrupt] != NULL) {
        g_ISRHandlers[regs->interrupt](regs);

    } else if (regs->interrupt >= 32) {
        terminal_printf("Unhandled interrupt %d!\n", regs->interrupt);

    } else {
        terminal_printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        
        /*terminal_printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        terminal_printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        terminal_printf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);*/

        terminal_printf("KERNEL PANIC!\n");
        HALT();
    }
}

void ISR_RegisterHandler(int interrupt, ISRHandler handler) {
    g_ISRHandlers[interrupt] = handler;
    IDT_EnableGate(interrupt);
}