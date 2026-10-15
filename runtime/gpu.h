#pragma once
#include <cstdint>
#include <vector>
struct DiagnosticGpuState {uint64_t binMask=0,binSelect=0;uint32_t blendControl[4]={},colorControl=0,currentBinIdMin=0;bool initialized=false;uint32_t flags=0,callback=0,argument=0,pfp=0,me=0,interruptCallback=0,interruptArgument=0,systemCommandIdentifierAddress=0; uint32_t notificationCount=0; uint16_t frontWidth=0,frontHeight=0,backWidth=0,backHeight=0; bool ringConfigured=false; uint32_t vertexControl=0; uint32_t pointSize=0; uint32_t colorMask=0; uint32_t modeControl=0; uint32_t hizControl=0; uint32_t depthControl=0; uint32_t coherDestBase[8]={}; uint32_t aaMask=0; uint32_t clipTransformConfig[3]={}; uint32_t vizQueryConfig=0; uint32_t vertexIndexConfig[3]={}; uint32_t sqProgramControl=0,sqContextMisc=0; uint32_t invalidationMask=0,invalidationCount=0; uint32_t unknownE42=0,paClEnhance=0,rbBcControl=0,windowConfig[3]={}; uint32_t unknown0081=0,unknown0082=0; uint32_t sqEoRt=0; uint32_t waitUntil=0; uint32_t scratchAddress=0,scratchMask=0,scratch[8]={}; uint32_t unknownA02[4]={}; uint32_t sqGprManagement=0,sqFlowControl=0,sqInstStoreManagement=0; uint32_t coherSize=0,coherBase=0,coherStatus=0; uint32_t processedPackets=0,microEngineWords=0; uint32_t ringPhysical=0,ringBytes=0,ringReadIndex=0,readPointerPhysical=0,readPointerUpdateFrequency=0; std::vector<uint32_t> shaderWords[2]; uint32_t shaderUploadCount=0; uint32_t surfaceInfo=0,screenScissor[2]={},coverageWrittenMask=0; uint32_t diagnosticBypassedDraws=0; uint32_t shaderEventInitiator=0,completedShaderEvents=0; uint32_t noOutputDraws=0;};
DiagnosticGpuState queryDiagnosticGpu(uint8_t* base);

// Read-only bounded snapshot for an unhandled GPU register fault.
void dumpDiagnosticGpuRing(uint8_t* base);

void diagnosticMmioStore32(uint8_t* base,uint32_t address,uint32_t value,uint32_t lr);
