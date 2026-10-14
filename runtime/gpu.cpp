#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "gpu.h"
#include <map>
#include <vector>
#include <atomic>
#include <mutex>
#include <cstdio>
#include <cstdlib>
namespace {
std::mutex mutex;
std::map<uint8_t*,DiagnosticGpuState> engines;
std::map<uint8_t*,std::vector<uint32_t>> microEngine;
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
 std::lock_guard<std::mutex> guard(mutex);engines[base]=next;microEngine.erase(base);
 fprintf(stderr,"GPU_DIAGNOSTIC_INIT flags=%08X callback=%08X argument=%08X pfp=%08X me=%08X renderer=absent microcode=not-executed\n",next.flags,next.callback,next.argument,next.pfp,next.me);
 // Compatibility return used by Xenia's VdInitializeEngines stub, not GPU readiness.
 ctx.r3.u64=1;
}
PPC_FUNC(__imp__VdShutdownEngines){std::lock_guard<std::mutex> guard(mutex);engines.erase(base);microEngine.erase(base);fprintf(stderr,"GPU_DIAGNOSTIC_SHUTDOWN\n");}

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

void dumpDiagnosticGpuRing(uint8_t* base){
 const auto s=queryDiagnosticGpu(base);
 if(!s.ringConfigured||!s.ringBytes||!physicalRange(base,s.ringPhysical,s.ringBytes)){
  fprintf(stderr,"GPU_RING_SNAPSHOT unavailable\n");return;
 }
 // No read-pointer advancement or completion is published by this inspection.
 const uint32_t words=s.ringBytes/4,limit=words<256?words:256;
 fprintf(stderr,"GPU_RING_SNAPSHOT physical=%08X bytes=%08X read_index=%u words=%u truncated=%u execution=absent\n",s.ringPhysical,s.ringBytes,s.ringReadIndex,limit,words>limit);
 for(uint32_t i=0;i<limit;++i){
  const uint32_t index=(s.ringReadIndex+i)%words;
  const auto* p=base+0xA0000000ull+s.ringPhysical+uint64_t(index)*4;
  const uint32_t value=(uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|p[3];
  fprintf(stderr,"GPU_RING_WORD index=%u value=%08X\n",index,value);
 }
}

namespace {
uint32_t physicalWord(uint8_t* base,uint32_t address){
 if((address&3)||!physicalRange(base,address,4))invalidRing();
 const auto* p=base+0xA0000000ull+address;
 return (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|p[3];
}
uint32_t& gpuRegister(DiagnosticGpuState& s,uint32_t index){
 if(index>=0x2006&&index<=0x200D)return s.coherDestBase[index-0x2006];
 switch(index){
 case 0x2302:return s.vertexControl;
 case 0x2280:return s.pointSize;
 case 0x2104:return s.colorMask;
 case 0x2208:return s.modeControl;
 case 0x2203:return s.hizControl;
 case 0x2200:return s.depthControl;
 case 0x2312:return s.aaMask;
 case 0x2204:return s.clipTransformConfig[0];case 0x2205:return s.clipTransformConfig[1];case 0x2206:return s.clipTransformConfig[2];
 case 0x2293:return s.vizQueryConfig;
 case 0x2100:return s.vertexIndexConfig[0];case 0x2101:return s.vertexIndexConfig[1];case 0x2102:return s.vertexIndexConfig[2];
 case 0x2180:return s.sqProgramControl;case 0x2181:return s.sqContextMisc;
 case 0xE42:return s.unknownE42;case 0xC85:return s.paClEnhance;case 0xF01:return s.rbBcControl;
 case 0x2080:return s.windowConfig[0];case 0x2081:return s.windowConfig[1];case 0x2082:return s.windowConfig[2];
 case 0x81:return s.unknown0081;case 0x82:return s.unknown0082;
 case 0xA2F:return s.coherSize;case 0xA30:return s.coherBase;case 0xA31:return s.coherStatus;
 case 0xD00:return s.sqGprManagement;case 0xD01:return s.sqFlowControl;case 0xD02:return s.sqInstStoreManagement;case 0xD04:return s.sqEoRt;
 case 0x5C8:return s.waitUntil;case 0x1DC:return s.scratchMask;case 0x1DD:return s.scratchAddress;
 }
 if(index>=0xA02&&index<=0xA05)return s.unknownA02[index-0xA02];
 if(index>=0x578&&index<=0x57F)return s.scratch[index-0x578];
 fprintf(stderr,"PENDING_GPU_REGISTER_ACCESS index=%04X\n",index);std::_Exit(20);
}
void writeGpuRegister(uint8_t* base,DiagnosticGpuState& state,uint32_t index,uint32_t value){
 (void)gpuRegister(state,index); // Reject unsupported indices before side effects.
    // Opaque compatibility storage, matching Xenia's zero-initialized register
    // file and generic WriteRegister fallback. Hardware semantics are unknown.
    // Initialization/configuration storage only. Rendering and EDRAM effects
    // are not implemented; draws and events still stop before completion.
    if(index==0x2302||index==0x2280||index==0x2104||index==0x2208||index==0x2203||index==0x2200||(index>=0x2006&&index<=0x200D)||index==0x2312||(index>=0x2204&&index<=0x2206)||index==0x2293||(index>=0x2100&&index<=0x2102)||index==0x2180||index==0x2181||index==0xE42||index==0xC85||index==0xF01||(index>=0x2080&&index<=0x2082))gpuRegister(state,index)=value;
    else if(index==0x81)state.unknown0081=value;
    else if(index==0x82)state.unknown0082=value;
    else if(index==0xA2F)state.coherSize=value;
    else if(index==0xA30)state.coherBase=value;
    else if(index==0xA31)state.coherStatus=value|0x80000000u;
     // SQ configuration is retained for a future shader backend; no shader runs.
     else if(index==0xD00)state.sqGprManagement=value;
     else if(index==0xD01)state.sqFlowControl=value;
     else if(index==0xD02)state.sqInstStoreManagement=value;
     // Retain SQ_EO_RT configuration only; no shader execution is implied.
     else if(index==0xD04)state.sqEoRt=value;
     // Xenia retains UNKNOWN_0A02..0A05 as raw register words. Their hardware
     // semantics remain unknown; this is diagnostic storage, not GPU execution.
     else if(index>=0xA02&&index<=0xA05)state.unknownA02[index-0xA02]=value;
     else if(index==0x5C8){
      // Xenia registers.h: only 2D/3D idle and idle-clean bits are supported.
      // Accepted packets execute synchronously and no draws or GPU resources
      // exist yet. A future async renderer must wait on actual fences here.
      if((value&~0x0003C000u)||(state.coherStatus&0x80000000u)){
       fprintf(stderr,"PENDING_GPU_WAIT_UNTIL value=%08X coherence=%08X\n",value,state.coherStatus);std::_Exit(20);
      }
      std::atomic_thread_fence(std::memory_order_seq_cst);
      state.waitUntil=value;
      fprintf(stderr,"GPU_IDLE_WAIT value=%08X backend=synchronous-no-renderer\n",value);
     }
     else if(index==0x1DC)state.scratchMask=value;
     else if(index==0x1DD)state.scratchAddress=value;
     else {
      const uint32_t slot=index-0x578;
      if(state.scratchMask&(1u<<slot)){
       const uint64_t target=uint64_t(state.scratchAddress)+slot*4;
       if(target+4>0x20000000ull||(target&3)||!physicalRange(base,uint32_t(target),4))invalidRing();
       MEMORY_BASIC_INFORMATION m{};
       VirtualQuery(base+0xA0000000ull+target,&m,sizeof(m));
       if(!(m.Protect&(PAGE_READWRITE|PAGE_WRITECOPY|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)))invalidRing();
       PPC_STORE_U32(0xA0000000u+uint32_t(target),value);
       fprintf(stderr,"GPU_SCRATCH_WRITE slot=%u physical=%08X value=%08X\n",slot,uint32_t(target),value);
      }
      state.scratch[slot]=value;
     }
    fprintf(stderr,"GPU_REGISTER index=%04X value=%08X\n",index,value);
}
void processPackets(uint8_t* base,DiagnosticGpuState& state,uint32_t start,uint32_t words,uint32_t ringStart,uint32_t ringBytes,unsigned depth,uint32_t& budget){
 if(depth>8||words>65536)invalidRing();
 auto address=[&](uint32_t i){return ringBytes?ringStart+((start-ringStart+i*4)%ringBytes):start+i*4;};
 for(uint32_t i=0;i<words;){
  if(!budget--)invalidRing();
  const uint32_t head=physicalWord(base,address(i)),type=head>>30;
  const uint32_t count=head==0||type==2?0:type==1?2:((head>>16)&0x3FFF)+1;
  if(count>words-i-1)invalidRing();
  const uint32_t opcode=(head>>8)&0x7F;
  fprintf(stderr,"GPU_PACKET depth=%u physical=%08X header=%08X payload=%u\n",depth,address(i),head,count);
  if(head==0||type==2){ /* Explicit no-op packet. */ }
  else if(type==0){
   const uint32_t first=head&0x7FFF;const bool single=(head&0x8000)!=0;
   // COHER_* writes stage a cache-coherence request; status remains pending.
   // Match Xenia's WriteRegister behavior without pretending to flush a GPU.
   for(uint32_t n=0;n<count;++n){
    const uint32_t index=first+(single?0:n);
    if(!(index==0x2302||index==0x2280||index==0x2104||index==0x2208||index==0x2203||index==0x2200||(index>=0x2006&&index<=0x200D)||index==0x2312||(index>=0x2204&&index<=0x2206)||index==0x2293||(index>=0xA2F&&index<=0xA31)||(index>=0xD00&&index<=0xD02)||(index>=0xA02&&index<=0xA05)||(index>=0x2100&&index<=0x2102)||index==0x2180||index==0x2181||index==0xE42||index==0xC85||index==0xF01||(index>=0x2080&&index<=0x2082)||index==0x81||index==0x82||index==0xD04||index==0x5C8||index==0x1DC||index==0x1DD||(index>=0x578&&index<=0x57F))){fprintf(stderr,"PENDING_GPU_REGISTER_PACKET index=%04X physical=%08X\n",index,address(i));std::_Exit(20);}
   }
   for(uint32_t n=0;n<count;++n){
    const uint32_t index=first+(single?0:n),value=physicalWord(base,address(i+1+n));
    writeGpuRegister(base,state,index,value);
   }
  }
  else if(type==3&&(head&1)==0&&opcode==0x21){
   if(count!=3)invalidRing();
   const uint32_t info=physicalWord(base,address(i+1));
   const uint32_t andWord=physicalWord(base,address(i+2)),orWord=physicalWord(base,address(i+3));
   fprintf(stderr,"GPU_REG_RMW info=%08X and=%08X or=%08X\n",info,andWord,orWord);
   if(info&0x3FFFE000u){fprintf(stderr,"PENDING_GPU_RMW_MODE\n");std::_Exit(20);}
   const uint32_t index=info&0x1FFF;
   const uint32_t before=gpuRegister(state,index);
   const uint32_t andValue=(info&0x80000000u)?gpuRegister(state,andWord&0x1FFF):andWord;
   const uint32_t orValue=(info&0x40000000u)?gpuRegister(state,orWord&0x1FFF):orWord;
   writeGpuRegister(base,state,index,(before&andValue)|orValue);
  }
  else if(type==3&&(head&1)==0&&opcode==0x3C&&count==5){
   const uint32_t info=physicalWord(base,address(i+1)),reg=physicalWord(base,address(i+2));
   const uint32_t ref=physicalWord(base,address(i+3)),mask=physicalWord(base,address(i+4));
   const uint32_t interval=physicalWord(base,address(i+5));
   fprintf(stderr,"GPU_WAIT info=%08X register=%08X ref=%08X mask=%08X interval=%u\n",info,reg,ref,mask,interval);
   if((info&~7u)||reg<0xA2F||reg>0xA31){fprintf(stderr,"PENDING_GPU_WAIT_MODE\n");std::_Exit(20);}
   if(reg==0xA31&&(state.coherStatus&0x80000000u)){
    // Only the observed initial request is supported. All currently accepted
    // packets use shared CPU memory; no textures, draws or host GPU caches exist.
    // A future rendering backend must replace this path with resource barriers.
    if(state.coherStatus!=0x81000000u){fprintf(stderr,"PENDING_GPU_COHERENCE_MODE status=%08X\n",state.coherStatus);std::_Exit(20);}
    if(!physicalRange(base,state.coherBase,state.coherSize))invalidRing();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    state.coherStatus=0;
    fprintf(stderr,"GPU_COHERENCE_COMPLETE base=%08X bytes=%08X backend=shared-cpu-memory\n",state.coherBase,state.coherSize);
   }
   const uint32_t v=(reg==0xA2F?state.coherSize:reg==0xA30?state.coherBase:state.coherStatus)&mask;
   bool matched=false;
   switch(info&7){case 1:matched=v<ref;break;case 2:matched=v<=ref;break;case 3:matched=v==ref;break;case 4:matched=v!=ref;break;case 5:matched=v>=ref;break;case 6:matched=v>ref;break;case 7:matched=true;break;}
   // The synchronous diagnostic has no producer for an unsatisfied register
   // condition. Stop without publishing a completed ring read pointer.
   if(!matched){fprintf(stderr,"PENDING_GPU_WAIT_CONDITION value=%08X\n",v);std::_Exit(20);}
  }
  else if(type==3&&(head&1)==0&&opcode==0x2B){
   if(count<2)invalidRing();
   const uint32_t stage=physicalWord(base,address(i+1)),descriptor=physicalWord(base,address(i+2));
   const uint32_t offset=descriptor>>16,length=descriptor&0xFFFF;
   fprintf(stderr,"GPU_SHADER_UPLOAD stage=%u offset=%u words=%u validation=packet-only\n",stage,offset,length);
   if(stage>1||offset){fprintf(stderr,"PENDING_GPU_SHADER_MODE\n");std::_Exit(20);}
   if(!length||length!=count-2)invalidRing();
   // Retain host-order microcode words. No translation, instruction validation,
   // shader execution or resource binding is performed by this diagnostic.
   std::vector<uint32_t> code;code.reserve(length);
   for(uint32_t n=0;n<length;++n)code.push_back(physicalWord(base,address(i+3+n)));
   state.shaderWords[stage]=std::move(code);++state.shaderUploadCount;
  }
  else if(type==3&&(head&1)==0&&opcode==0x3B){
   if(count!=1)invalidRing();
   // No derived GPU state or shader/resource caches exist in this diagnostic.
   // Record the request without resetting architectural registers or memory.
   // A future renderer must invalidate its actual cached state here.
   state.invalidationMask=physicalWord(base,address(i+1));
   ++state.invalidationCount;
   fprintf(stderr,"GPU_INVALIDATE_STATE mask=%08X backend=no-derived-state\n",state.invalidationMask);
  }
  else if(type==3&&(head&1)==0&&opcode==0x10){ /* NOP payload. */ }
  else if(type==3&&(head&1)==0&&opcode==0x48){
   auto& data=microEngine[base];data.clear();
   for(uint32_t n=0;n<count;++n)data.push_back(physicalWord(base,address(i+1+n)));
   state.microEngineWords=count;
  }else if(type==3&&(head&1)==0&&(opcode==0x3F||opcode==0x37)&&count==2){
   const uint32_t child=physicalWord(base,address(i+1))&0x1FFFFFFF;
   const uint32_t length=physicalWord(base,address(i+2));
   if(length>65536||(child&3)||!physicalRange(base,child,uint64_t(length)*4))invalidRing();
   // Read-only bounded preview. Capture upcoming data before execution stops;
   // these words have not executed and do not advance the completion pointer.
   if(depth==0){
    const uint32_t preview=length<64?length:64;
    fprintf(stderr,"GPU_IB_PREVIEW physical=%08X total_words=%u captured=%u execution=not-implied\n",child,length,preview);
    for(uint32_t n=0;n<preview;++n)fprintf(stderr,"GPU_IB_WORD physical=%08X value=%08X\n",child+n*4,physicalWord(base,child+n*4));
   }
   processPackets(base,state,child,length,0,0,depth+1,budget);
  }else{
   fprintf(stderr,"PENDING_GPU_PACKET type=%u opcode=%02X physical=%08X depth=%u execution=incomplete\n",type,opcode,address(i),depth);std::_Exit(20);
  }
  ++state.processedPackets;i+=count+1;
 }
}
}
void diagnosticMmioStore32(uint8_t* base,uint32_t address,uint32_t value,uint32_t lr){
 if((address&0xFFFF0000)!=0x7FC80000){PPC_STORE_U32(address,value);return;}
 if(address!=0x7FC80714){fprintf(stderr,"PENDING_GPU_REGISTER address=%08X value=%08X lr=%08X\n",address,value,lr);std::_Exit(20);}
 std::lock_guard<std::mutex> guard(mutex);auto it=engines.find(base);
 if(it==engines.end()||!it->second.ringConfigured)invalidRing();
 auto& state=it->second;const uint32_t capacity=state.ringBytes/4;
 if(value>=capacity)invalidRing();
 const uint32_t words=(value+capacity-state.ringReadIndex)%capacity;
 fprintf(stderr,"GPU_SUBMIT write_index=%u read_index=%u words=%u lr=%08X\n",value,state.ringReadIndex,words,lr);
 uint32_t budget=65536;
 processPackets(base,state,state.ringPhysical+state.ringReadIndex*4,words,state.ringPhysical,state.ringBytes,0,budget);
 state.ringReadIndex=value;
 if(state.readPointerPhysical){
  const auto a=0xA0000000u+state.readPointerPhysical;
  PPC_STORE_U32(a,value);
 }
}
