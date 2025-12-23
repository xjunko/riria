#pragma once
#include <riria/cpu/regs.h>
#include <riria/types.h>

#define ISR(ISR) extern void _isr##ISR(void)
#define ISR_SET(ISR)   \
  isrs[ISR].idx = ISR; \
  isrs[ISR].stub = _isr##ISR
typedef void (*isr_callback)(struct regs*);

// bit cursed but, whatever.
ISR(0);
ISR(1);
ISR(2);
ISR(3);
ISR(4);
ISR(5);
ISR(6);
ISR(7);
ISR(8);
ISR(9);
ISR(10);
ISR(11);
ISR(12);
ISR(13);
ISR(14);
ISR(15);
ISR(16);
ISR(17);
ISR(18);
ISR(19);
ISR(20);
ISR(21);
ISR(22);
ISR(23);
ISR(24);
ISR(25);
ISR(26);
ISR(27);
ISR(28);
ISR(29);
ISR(30);
ISR(31);
ISR(128);

void isr_install(void);
void isr_install_handler(size_t, isr_callback);
void isr_uninstall_handler(size_t);