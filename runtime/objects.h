#pragma once
#include <cstdint>
void registerCurrentGuestThread(uint8_t* base,uint32_t pcr);

uint32_t registerNativeGuestThread(uint8_t* base,uint32_t pcr,void* native);
