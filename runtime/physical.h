#pragma once
#include <cstdint>
// Reserve a guest address space with replaceable physical-memory ranges.
uint8_t* reserveGuestMemory();
// Caller must first free physical allocations and stop guest threads.
bool releaseGuestMemory(uint8_t* base);
