#pragma once
#include <cstdint>
struct DiagnosticGpuState {bool initialized=false;uint32_t flags=0,callback=0,argument=0,pfp=0,me=0,interruptCallback=0,interruptArgument=0,systemCommandIdentifierAddress=0;};
DiagnosticGpuState queryDiagnosticGpu(uint8_t* base);
