// whole implementation is derived from
// https://github.com/elttil/sbOS/blob/master/kernel/drivers/pci.c#L14

#include <riria/cpu/io.h>
#include <riria/drivers/pci.h>
#include <riria/types.h>

uint32_t pci_read(const pci_device_t* dev, uint8_t func, uint8_t offset) {
  uint32_t addr;
  uint32_t lbus = (uint32_t)dev->bus;
  uint32_t lslot = (uint32_t)dev->slot;
  uint32_t lfunc = (uint32_t)func;
  addr = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                    (offset & 0xFC) | ((uint32_t)0x80000000));
  outb_l(CMD_PORT, addr);
  return inb_l(DATA_PORT);
}

void pci_write(const pci_device_t* dev, uint8_t func, uint8_t offset,
               uint32_t val) {
  uint32_t addr;
  uint32_t lbus = (uint32_t)dev->bus;
  uint32_t lslot = (uint32_t)dev->slot;
  uint32_t lfunc = (uint32_t)func;
  addr = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                    (offset & 0xFC) | ((uint32_t)0x80000000));

  outb_l(CMD_PORT, addr);
  outb_l(DATA_PORT, val);
}

uint8_t pci_get_bar(const pci_device_t* dev, uint8_t bar_idx, pci_bar_t* bar) {
  if (bar_idx > 5) return 0;

  uint8_t offset = 0x10 + bar_idx * sizeof(uint32_t);
  uint32_t phys_bar = pci_read(dev, 0, offset);
  uint8_t type;

  if (phys_bar & 0x1) {
    type = PCI_BAR_IO;
  } else {
    type = PCI_BAR_MEM;
  }

  uint32_t original_bar = phys_bar;
  phys_bar &= 0xFFFFFFF0;

  pci_write(dev, 0, offset, 0xFFFFFFFF);
  uint32_t bar_result = pci_read(dev, 0, offset);

  bar_result &= ~(0xF);
  bar_result = ~bar_result;
  bar_result++;

  pci_write(dev, 0, offset, original_bar);

  bar->addr = phys_bar;
  bar->size = bar_result;
  bar->type = type;
  return 1;
}

uint8_t pci_devices_by_id(uint8_t class_id, uint8_t subclass_id,
                          pci_device_t* dev) {
  for (uint8_t bus = 0; bus < 255; bus++) {
    for (uint8_t slot = 0; slot < 255; slot++) {
      dev->bus = bus;
      dev->slot = slot;
      uint16_t class_info = pci_read(dev, 0, 0x08) >> 16;
      uint16_t h_classcode = (class_info & 0xFF00) >> 8;
      uint16_t h_subclass = class_info & 0x00FF;
      if (h_classcode != class_id) continue;
      if (h_subclass != subclass_id) continue;

      uint32_t device_vendor = pci_read(dev, 0, 0x00);
      dev->vendor = device_vendor & 0xFFFF;
      dev->device = (device_vendor >> 16);

      uint32_t function_type = pci_read(dev, 0, 0x0C);
      dev->function = function_type >> 16;
      dev->function &= 0xFF;

      return 1;
    }
  }

  return 0;
}

uint8_t pci_populate_device(uint16_t vendor, uint16_t device,
                            pci_device_t* dev) {
  dev->vendor = vendor;
  dev->device = device;

  for (uint32_t bus = 0; bus < 256; bus++) {
    for (uint32_t slot = 0; slot < 256; slot++) {
      pci_device_t tmp;
      tmp.bus = bus;
      tmp.slot = slot;
      uint32_t device_vendor = pci_read(&tmp, 0, 0x00);
      if (vendor != (device_vendor & 0xFFFF)) continue;
      if (device != (device_vendor >> 16)) continue;
      dev->bus = bus;
      dev->slot = slot;
      uint32_t bar0 = pci_read(dev, 0, 0x10);
      ASSERT(bar0 & 0x1 && "memory io only");
      dev->general.base = bar0 & (~0x3);
      return 1;
    }
  }

  return 0;
}
