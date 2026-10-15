#pragma once
#include "ppc_recomp_shared.h"
// VdHSIOCalibrationLock is a 28-byte RTL critical section, not a pointer
// variable. Xenia RegisterVideoExports initializes it with spin count 10000.
inline bool initializeVideoCalibrationLock(uint8_t* base,uint32_t slot) {
 PPCContext init{};
 init.r3.u64=0;init.r4.u64=28;init.r5.u64=4;
 __imp__MmAllocatePhysicalMemory(init,base);
 const uint32_t lock=init.r3.u32;
 if(!lock)return false;
 init.r3.u64=lock;init.r4.u64=10000;
 __imp__RtlInitializeCriticalSectionAndSpinCount(init,base);
 base[slot]=lock>>24;base[slot+1]=lock>>16;base[slot+2]=lock>>8;base[slot+3]=lock;
 return true;
}
