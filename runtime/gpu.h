#pragma once
#include <cstdint>
struct DiagnosticGpuState {bool initialized=false;uint32_t flags=0,callback=0,argument=0,pfp=0,me=0,interruptCallback=0,interruptArgument=0,systemCommandIdentifierAddress=0; uint32_t notificationCount=0; uint16_t frontWidth=0,frontHeight=0,backWidth=0,backHeight=0; bool ringConfigured=false; uint32_t processedPackets=0,microEngineWords=0; uint32_t ringPhysical=0,ringBytes=0,ringReadIndex=0,readPointerPhysical=0,readPointerUpdateFrequency=0;};
DiagnosticGpuState queryDiagnosticGpu(uint8_t* base);

// Read-only bounded snapshot for an unhandled GPU register fault.
void dumpDiagnosticGpuRing(uint8_t* base);

void diagnosticMmioStore32(uint8_t* base,uint32_t address,uint32_t value,uint32_t lr);
