#include <riria/cpu/cpuid.h>
#include <riria/types.h>

cpuid_info_t cpuid(uint32_t leaf, uint32_t subleaf) {
  cpuid_info_t info;
  __asm__ volatile("cpuid"
                   : "=a"(info.eax), "=b"(info.ebx), "=c"(info.ecx),
                     "=d"(info.edx)
                   : "a"(leaf), "c"(subleaf));
  return info;
}