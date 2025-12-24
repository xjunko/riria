#include <riria/cpu/idt.h>
#include <riria/cpu/io.h>
#include <riria/cpu/irq.h>
#include <riria/types.h>
#include <stdio.h>

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_OFFSET 0x20
#define PIC1_DATA (PIC1 + 1)

#define PIC2 0xA0
#define PIC2_COMMAND PIC2
#define PIC2_OFFSET 0x28
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01

// interrups
#define INTERRUPT_STOP() asm volatile("cli")
#define INTERRUPT_START() asm volatile("sti")

static volatile int sync_depth = 0;

void int_disable(void) {
  INTERRUPT_STOP();

  uint64_t flags;
  asm volatile(
      "pushfq\n\t"
      "popq %0\n\t"
      : "=r"(flags)
      :
      : "memory");

  if (flags & (1ULL << 9)) {
    sync_depth = 1;
  } else {
    sync_depth++;
  }
}

void int_resume(void) {
  if (sync_depth == 0 || sync_depth == 1) {
    INTERRUPT_START();
  } else {
    sync_depth--;
  }
}

void int_enable(void) {
  sync_depth = 0;
  INTERRUPT_START();
}

// requests
#define IRQ_CHAIN_SIZE 16
#define IRQ_CHAIN_DEPTH 4

static void (*irqs[IRQ_CHAIN_SIZE])(void);
static irq_handler_chain_t irq_routines[IRQ_CHAIN_SIZE * IRQ_CHAIN_DEPTH] = {0};
static const char* _irq_description[IRQ_CHAIN_SIZE * IRQ_CHAIN_DEPTH] = {0};

const char* irq_get_handler(int irq, int chain) {
  if (irq >= IRQ_CHAIN_SIZE) return 0;
  if (chain >= IRQ_CHAIN_DEPTH) return 0;
  return _irq_description[IRQ_CHAIN_SIZE * chain + irq];
}

void irq_install_handler(int irq, irq_handler_chain_t handler,
                         const char* description) {
  kprintf("[irq] installing '%s' for irq %d \n", description, irq);
  INTERRUPT_STOP();
  for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
    if (irq_routines[i * IRQ_CHAIN_SIZE + irq]) {
      continue;
    }
    irq_routines[i * IRQ_CHAIN_SIZE + irq] = handler;
    _irq_description[i * IRQ_CHAIN_SIZE + irq] = description;
    break;
  }
  INTERRUPT_START();
}

void irq_uninstall_handler(int irq) {
  INTERRUPT_STOP();
  for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
    irq_routines[i * IRQ_CHAIN_SIZE + irq] = 0;
  }
  INTERRUPT_START();
}

static void irq_remap(void) {
  // init
  outb_p(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  outb_p(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

  // remap
  outb_p(PIC1_DATA, PIC1_OFFSET);
  outb_p(PIC2_DATA, PIC2_OFFSET);

  // ident
  outb_p(PIC1_DATA, 0x04);
  outb_p(PIC2_DATA, 0x02);

  // 8086 mode
  outb_p(PIC1_DATA, 0x01);
  outb_p(PIC2_DATA, 0x01);

  kprintf("[irq] PIC remapped to 0x%x and 0x%x\n", PIC1_OFFSET, PIC2_OFFSET);
}

static void irq_setup_gates(void) {
  for (size_t i = 0; i < IRQ_CHAIN_SIZE; i++) {
    idt_set_gate(32 + i, irqs[i], 0x08, 0x8E);
  }
}

void irq_install(void) {
  irq_remap();

  kprintf("[irq] IRQ INIT...");
  IRQ_SET(0);
  IRQ_SET(1);
  IRQ_SET(2);
  IRQ_SET(3);
  IRQ_SET(4);
  IRQ_SET(5);
  IRQ_SET(6);
  IRQ_SET(7);
  IRQ_SET(8);
  IRQ_SET(9);
  IRQ_SET(10);
  IRQ_SET(11);
  IRQ_SET(12);
  IRQ_SET(13);
  IRQ_SET(14);
  IRQ_SET(15);
  irq_setup_gates();
  kprintf(" OK!\n");
}

void irq_ack(int irq_no) {
  if (irq_no >= 8) {
    outb(PIC2_COMMAND, PIC_EOI);
  }
  outb(PIC1_COMMAND, PIC_EOI);
}

void print_regs(regs_t* r) {
  kprintf("[irq] err_code=0x%x int_no=0x%x\n", r->err_code, r->int_no);
  kprintf("[irq] rax=0x%x rbx=0x%x rcx=0x%x rdx=0x%x\n", r->rax, r->rbx, r->rcx,
          r->rdx);
  kprintf("[irq] rsi=0x%x rdi=0x%x rbp=0x%x\n", r->rsi, r->rdi, r->rbp);
  kprintf("[irq] r8=0x%x r9=0x%x r10=0x%x r11=0x%x\n", r->r8, r->r9, r->r10,
          r->r11);
  kprintf("[irq] r12=0x%x r13=0x%x r14=0x%x r15=0x%x\n", r->r12, r->r13, r->r14,
          r->r15);
  kprintf("[irq] rflags=0x%x\n ", r->rflags);
}

void irq_handler(regs_t* r) {
  int_disable();
  if (r->int_no <= 47 && r->int_no >= 32) {
    for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
      irq_handler_chain_t handler =
          irq_routines[i * IRQ_CHAIN_SIZE + (r->int_no - 32)];
      if (!handler) break;
      if (handler(r)) {
        goto done;
      }
    }
    irq_ack(r->int_no - 32);
  }
done:
  int_resume();
}