#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "gpu.h"
#include <map>
#include <mutex>
#include <cstdio>
#include <cstdlib>
namespace {
std::mutex mutex;
std::map<uint8_t*,DiagnosticGpuState> engines;
bool readable(uint8_t*b,uint32_t a,uint32_t n){
 if(!a||uint64_t(a)+n>0x100000000ull)return false;
 for(uint32_t i=0;i<n;++i){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(b+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;}
 return true;
}
}
DiagnosticGpuState queryDiagnosticGpu(uint8_t*b){std::lock_guard<std::mutex> guard(mutex);auto it=engines.find(b);return it==engines.end()?DiagnosticGpuState{}:it->second;}
PPC_FUNC(__imp__VdInitializeEngines){
 const DiagnosticGpuState next{true,ctx.r3.u32,ctx.r4.u32,ctx.r5.u32,ctx.r6.u32,ctx.r7.u32};
 // Capture the request only. Microcode execution and callbacks need a GPU backend.
 if(!readable(base,next.pfp,4)||!readable(base,next.me,4)||
    (next.callback&&((next.callback&3)||next.callback<PPC_CODE_BASE||uint64_t(next.callback)>=uint64_t(PPC_CODE_BASE)+PPC_CODE_SIZE))){ctx.r3.u64=0;return;}
 std::lock_guard<std::mutex> guard(mutex);engines[base]=next;
 fprintf(stderr,"GPU_DIAGNOSTIC_INIT flags=%08X callback=%08X argument=%08X pfp=%08X me=%08X renderer=absent microcode=not-executed\n",next.flags,next.callback,next.argument,next.pfp,next.me);
 // Compatibility return used by Xenia's VdInitializeEngines stub, not GPU readiness.
 ctx.r3.u64=1;
}
PPC_FUNC(__imp__VdShutdownEngines){std::lock_guard<std::mutex> guard(mutex);engines.erase(base);fprintf(stderr,"GPU_DIAGNOSTIC_SHUTDOWN\n");}

PPC_FUNC(__imp__VdSetGraphicsInterruptCallback){
 uint32_t callback=ctx.r3.u32,argument=ctx.r4.u32;
 if(callback&&((callback&3)||callback<PPC_CODE_BASE||uint64_t(callback)>=uint64_t(PPC_CODE_BASE)+PPC_CODE_SIZE)){fprintf(stderr,"INVALID_GPU_INTERRUPT_CALLBACK\n");std::_Exit(22);}
 std::lock_guard<std::mutex> guard(mutex);auto it=engines.find(base);
 if(it==engines.end()){fprintf(stderr,"PENDING_GPU_MODE interrupt registration before engine initialization\n");std::_Exit(20);}
 it->second.interruptCallback=callback;it->second.interruptArgument=argument;
 fprintf(stderr,"GPU_INTERRUPT_REGISTRATION callback=%08X argument=%08X delivery=pending\n",callback,argument);
}

PPC_FUNC(__imp__VdSetSystemCommandBufferGpuIdentifierAddress){
 // Xenia's export is a void stub. Title call sites pass 0 to clear, or
 // command-buffer storage + 8 to register. Retain the opaque address only:
 // no identifier writes or GPU completion are fabricated.
 // Reference: xenia/src/xenia/kernel/xboxkrnl/xboxkrnl_video.cc.
 const uint32_t address=ctx.r3.u32;
 std::lock_guard<std::mutex> guard(mutex);
 auto it=engines.find(base);
 if(it==engines.end()){
  if(!address)return;
  fprintf(stderr,"PENDING_GPU_MODE identifier registration before engine initialization\n");std::_Exit(20);
 }
 it->second.systemCommandIdentifierAddress=address;
 fprintf(stderr,"GPU_SYSTEM_COMMAND_IDENTIFIER address=%08X processing=pending\n",address);
}

namespace {
bool physicalRange(uint8_t* base,uint32_t address,uint64_t bytes){
 if(uint64_t(address)+bytes>0x20000000ull)return false;
 for(uint64_t pos=0;pos<bytes;){
  MEMORY_BASIC_INFORMATION m{};auto* p=base+0xA0000000ull+address+pos;
  if(!VirtualQuery(p,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
  pos+=static_cast<uint8_t*>(m.BaseAddress)+m.RegionSize-p;
 }
 return true;
}
[[noreturn]] void invalidRing(){fprintf(stderr,"INVALID_GPU_RING_CONFIGURATION\n");std::_Exit(22);}
}
PPC_FUNC(__imp__VdInitializeRingBuffer){
 const uint32_t address=ctx.r3.u32,log2=ctx.r4.u32;
 // Xenia command_processor.cc: byte size is 2^(RB_BUFSZ + 3).
 // Check before shifting; the guest physical address space is 512 MiB.
 if(log2>26||(address&3))invalidRing();
 const uint32_t bytes=uint32_t(1)<<(log2+3);
 if(!physicalRange(base,address,bytes))invalidRing();
 std::lock_guard<std::mutex> guard(mutex);auto it=engines.find(base);
 if(it==engines.end()){fprintf(stderr,"PENDING_GPU_MODE ring before engine initialization\n");std::_Exit(20);}
 auto& s=it->second;s.ringConfigured=true;s.ringPhysical=address;s.ringBytes=bytes;s.ringReadIndex=0;
 fprintf(stderr,"GPU_RING_CONFIGURATION physical=%08X bytes=%08X processing=pending\n",address,bytes);
}
PPC_FUNC(__imp__VdEnableRingBufferRPtrWriteBack){
 const uint32_t address=ctx.r3.u32,log2=ctx.r4.u32;
 if(log2>31||(address&3)||!physicalRange(base,address,4))invalidRing();
 std::lock_guard<std::mutex> guard(mutex);auto it=engines.find(base);
 if(it==engines.end()||!it->second.ringConfigured){fprintf(stderr,"PENDING_GPU_MODE writeback before ring initialization\n");std::_Exit(20);}
 it->second.readPointerPhysical=address;
 it->second.readPointerUpdateFrequency=(uint32_t(1)<<log2)>>2;
 // No commands have been processed: do not publish a completion/read pointer.
 fprintf(stderr,"GPU_RING_WRITEBACK physical=%08X frequency=%u delivery=pending\n",address,it->second.readPointerUpdateFrequency);
}

PPC_FUNC(__imp__VdCallGraphicsNotificationRoutines){
 // Xenia xboxkrnl_video.cc describes four BE uint16 scaling dimensions and
 // returns zero for event 1. Capture only; callback dispatch remains absent.
 const uint32_t event=ctx.r3.u32,address=ctx.r4.u32;
 if(event!=1){fprintf(stderr,"PENDING_GPU_MODE graphics notification event=%u\n",event);std::_Exit(20);}
 if(!readable(base,address,8)){fprintf(stderr,"INVALID_GRAPHICS_NOTIFICATION_BUFFER\n");std::_Exit(22);}
 auto u16=[&](uint32_t offset){return uint16_t((uint16_t(base[address+offset])<<8)|base[address+offset+1]);};
 const auto fw=u16(0),fh=u16(2),bw=u16(4),bh=u16(6);
 std::lock_guard<std::mutex> guard(mutex);auto it=engines.find(base);
 if(it==engines.end()){fprintf(stderr,"PENDING_GPU_MODE graphics notification before engine initialization\n");std::_Exit(20);}
 auto& s=it->second;++s.notificationCount;s.frontWidth=fw;s.frontHeight=fh;s.backWidth=bw;s.backHeight=bh;
 fprintf(stderr,"GPU_GRAPHICS_NOTIFICATION event=1 front=%ux%u back=%ux%u dispatch=pending\n",fw,fh,bw,bh);
 ctx.r3.u64=0;
}
