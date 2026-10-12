#pragma once
#include <cstdint>
struct PPCContext;
extern thread_local PPCContext* diagnosticContext;
void configureGuestThreads(uint8_t* base,uint32_t slots,uint32_t dataSize,uint32_t raw,uint32_t rawSize,uint32_t stackSize);
