#pragma once
#include <cstdint>
// Caller keeps the committed guest region alive until stopGuestTimestamp().
bool startGuestTimestamp(uint8_t* base,uint32_t importSlot,uint32_t storage);
void stopGuestTimestamp();
