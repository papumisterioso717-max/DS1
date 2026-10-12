#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "threads.h"
#include "objects.h"
#include "tls.h"
#include <mutex>
#include <memory>
#include <cstdio>
#include <cstdlib>
thread_local PPCContext* diagnosticContext=nullptr;
namespace {
struct Config {uint8_t* base=nullptr;uint32_t slots,dataSize,raw,rawSize,stackSize;} config;
std::mutex creationMutex;
uint32_t nextStack=0x71000000,nextId=2;
thread_local bool managedWorker=false;
struct ThreadExit {uint32_t code;};
struct Start {uint8_t*base;uint32_t pcr,thread,top,startup,entry,argument,id;};
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
bool writable(uint8_t*b,uint32_t a){
 if(!a||uint64_t(a)+4>0x100000000ull)return false;
 for(unsigned i=0;i<4;++i){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(b+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))||!(m.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE)))return false;}
 return true;
}
bool mapped(uint8_t*b,uint32_t a){return a>=PPC_CODE_BASE&&uint64_t(a)<uint64_t(PPC_CODE_BASE)+PPC_CODE_SIZE&&!(a&3)&&PPC_LOOKUP_FUNC(b,a);}
[[noreturn]] void pending(const char*why,PPCContext&ctx){fprintf(stderr,"PENDING_THREAD_MODE %s guest_lr=0x%08X\n",why,uint32_t(ctx.lr));std::_Exit(20);}
DWORD WINAPI execute(void*opaque){
 std::unique_ptr<Start> start(static_cast<Start*>(opaque));auto& s=*start;
 PPCContext c{};c.r13.u64=s.pcr;c.r1.u64=s.top-0x100;c.fpscr.loadFromHost();
 diagnosticContext=&c;managedWorker=true;
 c.r3.u64=s.startup?s.entry:s.argument;c.r4.u64=s.startup?s.argument:0;
 uint32_t result=0;
 fprintf(stderr,"GUEST_THREAD_START id=%u entry=0x%08X startup=0x%08X\n",s.id,s.entry,s.startup);
 try {(PPC_LOOKUP_FUNC(s.base,s.startup?s.startup:s.entry))(c,s.base);result=s.startup?0:c.r3.u32;}
 catch(const ThreadExit&e){result=e.code;}
 put(s.base,s.thread+0x140,result);put(s.base,s.thread+4,1);
 unregisterGuestTls(s.base,s.pcr);
 diagnosticContext=nullptr;managedWorker=false;
 fprintf(stderr,"GUEST_THREAD_EXIT id=%u status=0x%08X\n",s.id,result);
 // Keep guest structures/stack allocated for the diagnostic process lifetime.
 return result;
}
}
void configureGuestThreads(uint8_t*b,uint32_t slots,uint32_t dataSize,uint32_t raw,uint32_t rawSize,uint32_t stackSize){
 std::lock_guard<std::mutex> guard(creationMutex);config={b,slots,dataSize,raw,rawSize,stackSize};
}
PPC_FUNC(__imp__ExCreateThread){
 const uint32_t out=ctx.r3.u32,requested=ctx.r4.u32,idOut=ctx.r5.u32,startup=ctx.r6.u32,entry=ctx.r7.u32,argument=ctx.r8.u32,flags=ctx.r9.u32;
 std::lock_guard<std::mutex> guard(creationMutex);
 if(config.base!=base)pending("loader configuration missing",ctx);
 if(flags&~1u)pending("creation flags beyond create-suspended",ctx);
 if((out&&!writable(base,out))||(idOut&&!writable(base,idOut))||!mapped(base,entry)||(startup&&!mapped(base,startup))){ctx.r3.u64=0xC000000D;return;}
 uint64_t size=requested?requested:config.stackSize;
 if(!size)pending("default stack size missing",ctx);
 size=(size+0xFFF)&~0xFFFull;if(size<0x4000)size=0x4000;
 if(size>0x1000000||nextId>=128||uint64_t(nextStack)+size+0x10000>0x7F000000){ctx.r3.u64=0xC0000017;return;}
 if(uint64_t(config.dataSize)+config.slots*4>0xE000)pending("TLS exceeds thread arena",ctx);
 const uint32_t pcr=0x61000000+(nextId-2)*0x10000,thread=pcr+0x1000,storage=pcr+0x2000;
 const uint32_t allocation=nextStack,low=allocation+0x1000,top=low+uint32_t(size);
 if(!VirtualAlloc(base+pcr,0x10000,MEM_COMMIT,PAGE_READWRITE)){ctx.r3.u64=0xC0000017;return;}
 if(!VirtualAlloc(base+low,size,MEM_COMMIT,PAGE_READWRITE)){VirtualFree(base+pcr,0x10000,MEM_DECOMMIT);ctx.r3.u64=0xC0000017;return;}
 put(base,pcr+0x30,pcr);put(base,pcr+0x70,top);put(base,pcr+0x74,low);put(base,pcr+0x100,thread);
 base[thread]=6;base[thread+0xBC]=flags&1;
 put(base,thread+0x5C,top);put(base,thread+0x60,low);put(base,thread+0x68,storage);put(base,thread+0xD0,allocation);
 put(base,thread+0x14C,nextId);put(base,thread+0x150,entry);put(base,thread+0x16C,flags);
 registerGuestTls(base,pcr,storage,config.slots,config.dataSize,config.raw,config.rawSize);
 auto start=std::make_unique<Start>(Start{base,pcr,thread,top,startup,entry,argument,nextId});
 HANDLE native=CreateThread(nullptr,0x1000000,execute,start.get(),CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION,nullptr);
 if(!native){unregisterGuestTls(base,pcr);VirtualFree(base+low,size,MEM_DECOMMIT);VirtualFree(base+pcr,0x10000,MEM_DECOMMIT);ctx.r3.u64=0xC0000017;return;}
 const uint32_t handle=registerNativeGuestThread(base,pcr,native);start.release();
 if(out)put(base,out,handle);if(idOut)put(base,idOut,nextId);
 fprintf(stderr,"ExCreateThread handle=0x%08X id=%u suspended=%u stack=0x%X pcr=0x%08X\n",handle,nextId,flags&1,uint32_t(size),pcr);
 ++nextId;nextStack=(top+0xFFFF)&~0xFFFFu;
 if(!(flags&1)&&ResumeThread(native)==DWORD(-1))pending("native resume failed",ctx);
 ctx.r3.u64=0;
}
PPC_FUNC(__imp__ExTerminateThread){
 if(!managedWorker)pending("termination of the main guest thread",ctx);
 throw ThreadExit{ctx.r3.u32};
}
