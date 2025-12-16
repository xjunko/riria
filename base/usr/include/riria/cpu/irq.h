#pragma once
#include <riria/cpu/regs.h>
#include <stddef.h>
#include <stdint.h>

#define IRQ_OFF int_disable()
#define IRQ_RES int_resume()
#define IRQ_ON int_enable()

#define IRQ(N) extern void _irq##N(void)
#define IRQ_SET(N) irqs[N] = _irq##N

typedef void (*irq_handler_t)(regs_t *);
typedef int (*irq_handler_chain_t)(regs_t *);

void int_disable(void);
void int_resume(void);
void int_enable(void);

char *get_irq_handler(int, int);
void irq_install(void);
void irq_install_handler(int, irq_handler_chain_t, const char *);
void irq_uninstall_handler(int);

void irq_ack(int);
void irq_handler(regs_t *);

IRQ(0);
IRQ(1);
IRQ(2);
IRQ(3);
IRQ(4);
IRQ(5);
IRQ(6);
IRQ(7);
IRQ(8);
IRQ(9);
IRQ(10);
IRQ(11);
IRQ(12);
IRQ(13);
IRQ(14);
IRQ(15);