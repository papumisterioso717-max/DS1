#pragma once
#include <cstdint>
void diagnosticMmioStore32(uint8_t* base,uint32_t address,uint32_t value,uint32_t lr);
#define PPC_MM_STORE_U32(x,y) diagnosticMmioStore32(base,uint32_t(x),uint32_t(y),uint32_t(ctx.lr))
