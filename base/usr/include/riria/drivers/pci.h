#pragma once
#include <riria/types.h>

#define PCI_BAR_MEM 1
#define PCI_BAR_IO 2

#define DATA_PORT 0xCFC
#define CMD_PORT 0xCF8

typedef struct pci_bar {
  uint32_t addr;
  uint32_t size;
  uint8_t type;
} pci_bar_t;

typedef struct general_device {
  uint32_t base;
  uint8_t irq;
} general_device_t;

typedef struct pci_device {
  uint16_t vendor;
  uint16_t device;
  uint8_t bus;
  uint8_t slot;
  uint8_t function;

  union {
    general_device_t general;
  };
} pci_device_t;

uint32_t pci_read(const pci_device_t*, uint8_t, uint8_t);
void pci_write(const pci_device_t*, uint8_t, uint8_t, uint32_t);
uint8_t pci_get_bar(const pci_device_t*, uint8_t, pci_bar_t*);
uint8_t pci_devices_by_id(uint8_t, uint8_t, pci_device_t*);
uint8_t pci_populate_device(uint16_t, uint16_t, pci_device_t*);