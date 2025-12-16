#pragma once
#include <stdint.h>

static uint8_t inb(uint16_t port) {
  uint8_t result = 0;
  __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

static void outb(uint16_t port, uint8_t data) {
  __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static uint8_t inb_p(uint16_t port) {
  uint8_t result = 0;
  __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

static void outb_p(uint16_t port, uint8_t data) {
  __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:"
                   :
                   : "a"(data), "Nd"(port));
}

static uint16_t inb_w(uint16_t port) {
  uint16_t result = 0;
  __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

static void outb_w(uint16_t port, uint16_t data) {
  __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

static uint32_t inb_l(uint16_t port) {
  uint32_t result = 0;
  __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

static void outb_l(uint16_t port, uint32_t data) {
  __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}
