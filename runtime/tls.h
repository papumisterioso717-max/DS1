#pragma once
#include <cstdint>
void registerGuestTls(uint8_t* base,uint32_t pcr,uint32_t storage,uint32_t slots,uint32_t dataSize,uint32_t rawAddress,uint32_t rawSize);

void unregisterGuestTls(uint8_t* base,uint32_t pcr);
