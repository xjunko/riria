#include <riria/cpu/io.h>
#include <riria/drivers/ac97.h>
#include <riria/drivers/pci.h>
#include <riria/mem.h>
#include <riria/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AC97_BUFFERS 32
#define AC97_BUFFER_SIZE 128000

pci_device_t ac97;
pci_bar_t nabm;
pci_bar_t nam;

uint16_t* ptr;
uint16_t* buf_desc;
void* phys_buf_desc;

audio_buffer_t buffers[AC97_BUFFERS];
int current_volume = 100;
int entry = 0;

void ac97_flush(void) {
  uint8_t s = inb(nabm.addr + 0x10 + 0xB);
  if (s & (1 << 0)) return;
  s |= (1 << 0);
  outb(nabm.addr + 0x10 + 0xB, s);
}

int ac97_can_write(void) {
  uint8_t read_ptr = inb(nabm.addr + 0x10 + 0x4);

  uint8_t buffer_left;
  if (entry >= read_ptr) {
    buffer_left = 32 - entry;
    if (0 == read_ptr && buffer_left > 0) {
      buffer_left--;
    }
  } else {
    buffer_left = read_ptr - entry - 1;
  }

  return (buffer_left > 0);
}

void ac97_setup_buffer(void) {
  // NOTE: certain line will cause /dev/fb0 bugs, though, since we dont have
  // anything that uses audio yet, we can leave it as is for now.

  uint16_t* pointer = buf_desc;

  for (int i = 0; i < AC97_BUFFERS; i++) {
    void* physical_audio_data;
    uint8_t* audio_data =
        kmalloc_phys(AC97_BUFFER_SIZE, (uintptr_t*)&physical_audio_data);
    ASSERT(audio_data);

    buffers[i].data = audio_data;
    buffers[i].phys = (uintptr_t)physical_audio_data;  // this line
    buffers[i].has_played = 1;
    buffers[i].data_written = 0;

    // and below
    *((uint32_t*)pointer) = (uint32_t)(uintptr_t)physical_audio_data;
    pointer += 2;
    *pointer = AC97_BUFFER_SIZE / 2;
    pointer++;
    *pointer = 0;
    pointer++;
  }
}

int ac97_write_buffer(uint8_t* buf, size_t sz) {
  ASSERT(sz <= AC97_BUFFER_SIZE);
  uint8_t read_ptr = inb(nabm.addr + 0x10 + 0x4);
  uint8_t buffer_left;

  if (entry >= read_ptr) {
    buffer_left = 32 - entry;
    if (read_ptr == 0 && buffer_left > 0) {
      buffer_left--;
    }
  } else {
    buffer_left = read_ptr - entry - 1;
  }

  if (buffer_left == 0) {
    return 0;
  }

  memset((uint8_t*)buffers[entry].data, 0x00, AC97_BUFFER_SIZE);
  memcpy((uint8_t*)buffers[entry].data, buf, sz);

  outb(nabm.addr + 0x10 + 0x5, entry);
  entry = (entry + 1) % 32;
  ac97_flush();
  return 1;
}

char temp_buffer[AC97_BUFFER_SIZE];
size_t temp_buffer_count = 0;

int ac97_write_pcm(uint8_t* buf, size_t sz) {
  size_t bytes_written = min(sz, AC97_BUFFER_SIZE - temp_buffer_count);
  memcpy(temp_buffer + temp_buffer_count, buf, bytes_written);
  temp_buffer_count += bytes_written;

  if (temp_buffer_count < AC97_BUFFER_SIZE) {
    return bytes_written;
  }

  if (!ac97_write_buffer((uint8_t*)temp_buffer, AC97_BUFFER_SIZE)) {
    return bytes_written;
  }

  temp_buffer_count = 0;
  return bytes_written;
}

void ac97_set_volume(int volume) {
  ASSERT(volume <= 100);
  current_volume = volume;
  int s;
  if (volume == 0) {
    s = 31;
  } else {
    s = (31 * volume) / 100;
  }

  uint8_t right_chan = 31 - s;
  uint8_t left_chan = 31 - s;

  outb_w(nam.addr + 0x18, right_chan | (left_chan << 8));
}

// https://github.com/elttil/sbOS/blob/master/kernel/drivers/ac97.c#L161
void ac97_install(void) {
  printf("[ac97] INIT...");

  if (!pci_populate_device(0x8086, 0x2415, &ac97)) {
    UNREACHABLE();
  }

  ptr = buf_desc = kmalloc_phys(0x1000, (uintptr_t*)&phys_buf_desc);
  if (!ptr) UNREACHABLE();

  // enable bus mastering
  uint32_t reg1 = pci_read(&ac97, 0, 0x04);
  reg1 |= (1 << 0);
  reg1 |= (1 << 2);
  pci_write(&ac97, 0, 0x04, reg1);

  ASSERT(pci_get_bar(&ac97, 1, &nabm));
  ASSERT(nabm.type == PCI_BAR_IO);

  ASSERT(pci_get_bar(&ac97, 0, &nam));
  ASSERT(nam.type == PCI_BAR_IO);

  /*
    In initalization of sound card you must resume card from cold reset
    and set power for it. It can be done by writing value 0x2 to Global
    Control register if you do not want to use interrupts, or 0x3 if you
    want to use interrupts.
  */
  printf(", POWER");
  outb_w(nabm.addr + 0x2C, (1 << 1));

  /*
    After this, you should write any value to NAM
    Reset register to reset all NAM registers.
  */
  printf(", RESET");
  outb_w(nam.addr, 0x1);

  /*
    After this, you can read
    card capability info from Global Status register to found out if 20
    bit audio samples are supported and also check out bit 4 in NAM
    Capabilites register and value in AUX Output to find out if this sound
    card support headhone output.
  */
  printf(", CAPABILITIES");
  outb_w(nam.addr + 0x2C, 48000);
  outb_w(nam.addr + 0x2E, 48000);
  outb_w(nam.addr + 0x30, 48000);
  outb_w(nam.addr + 0x32, 48000);

  /*
    As last thing, set maximal volume for
    PCM Output by writing value 0 to this register. Now sound card is
    ready to use.
  */
  printf(", VOLUME");
  ac97_set_volume(50);

  // playing sound
  outb_w(nam.addr + 0x02, 0);
  outb_w(nam.addr + 0x04, 0);

  // Set reset bit of output channel (NABM register 0x1B, value 0x2)
  // and wait for card to clear it
  uint8_t s = inb(nabm.addr + 0x10 + 0xB);
  s |= (1 << 1);
  outb(nabm.addr + 0x10 + 0xB, s);

  printf(", WAITING");
  for (; inb(nabm.addr + 0x10 + 0xB) & (1 << 1);)
    ;

  printf(", BUFFER");
  ac97_setup_buffer();

  // Write physical position of BDL to Buffer Descriptor Base Address
  // register (NABM register 0x10)
  printf(", BDL ADDR");
  outb_l(nabm.addr + 0x10, (uint32_t)(uintptr_t)phys_buf_desc);

  printf(", START");
  uint8_t control = inb(nabm.addr + 0x10 + 0xB);
  control |= (1 << 0);
  outb(nabm.addr + 0x10 + 0xB, control);

  printf(", OK!\n");
}