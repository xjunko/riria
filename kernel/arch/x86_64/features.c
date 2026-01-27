#include <riria/cpu/cpuid.h>
#include <riria/cpu/features.h>

// refer to:
// https://wiki.osdev.org/CPU_Registers_x86

bool cpuid_has_sse(void) {
  cpuid_info_t info = cpuid(1, 0);
  return (info.edx & (1 << 25)) != 0;
}

bool cpuid_has_fsgsbase(void) {
  cpuid_info_t info = cpuid(7, 0);
  return (info.ebx & (1 << 0)) != 0;
}

static void setup_cr0(void) {
  uint64_t cr0 = read_cr0();
  cr0 &= ~(1ULL << 2);  // clear EM to allow FPU instructions
  cr0 |= (1ULL << 1);   // set MP so WAIT/FWAIT checks TS bit
  write_cr0(cr0);
}

static void setup_cr4(void) {
  uint64_t cr4 = read_cr4();
  cr4 |= (1ULL << 9);   // 9th bit, enables the OSFXSR (FXSAVE, FXSTOR)
  cr4 |= (1ULL << 10);  // 10th bit, enables OSXMMEXCPT (SIMD float)
  write_cr4(cr4);
}

void cpu_features_install(void) {
  if (!cpuid_has_fsgsbase()) {
    panic("cpu does not support FSGSBASE instructions!");
  } else {
    write_cr4(read_cr4() | (1 << 16));  // FSGSBASE
    printf(INFO "[   cpu] FSGSBASE instructions enabled\n");
  }

  if (!cpuid_has_sse()) {
    panic("cpu does not support SSE!");
  } else {
    setup_cr0();
    setup_cr4();
    printf(INFO "[   cpu] SSE enabled\n");
  }
}
