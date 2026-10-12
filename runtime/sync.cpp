#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <mutex>
#include <condition_variable>
#include <map>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <limits>
// All guest critical-section entry points share this monitor. Guest fields
// retain Xbox big-endian representation; host wait queues are stored separately.
namespace {
struct Lock {std::condition_variable changed;uint32_t waiters=0;};
std::mutex monitor;
std::map<uint8_t*,std::unique_ptr<Lock>> locks;
[[noreturn]] void invalid(const char* reason,uint32_t a){fprintf(stderr,"INVALID_SYNC %s address=0x%08X\n",reason,a);std::_Exit(22);}
void check(uint8_t*base,uint32_t a,uint32_t n,bool write){
 if(!a||uint64_t(a)+n>0x100000000ull)invalid("range",a);
 for(uint64_t pos=a;pos<uint64_t(a)+n;){
  MEMORY_BASIC_INFORMATION info{};
  if(!VirtualQuery(base+pos,&info,sizeof(info))||info.State!=MEM_COMMIT||
     (info.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
     (write&&!(info.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))))invalid("unmapped",a);
  pos=std::min(uint64_t(a)+n,uint64_t(static_cast<uint8_t*>(info.BaseAddress)-base)+info.RegionSize);
 }
}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
uint32_t thread(PPCContext&ctx,uint8_t*b){
 const uint32_t p=ctx.r13.u32;
 if(!p||uint64_t(p)+0x104>0x100000000ull)invalid("missing KPCR",p);
 check(b,p+0x100,4,false);uint32_t t=get(b,p+0x100);check(b,t,0xAB0,false);return t;
}
Lock& lookup(uint8_t*b,uint32_t a){check(b,a,28,true);auto& l=locks[b+a];if(!l)l=std::make_unique<Lock>();return *l;}
void update(uint8_t*b,uint32_t a,Lock&l,uint32_t owner,uint32_t recursion){
 put(b,a+0x10,recursion+l.waiters-1);put(b,a+0x14,recursion);put(b,a+0x18,owner);
}
bool enter(PPCContext&ctx,uint8_t*b,bool attempt){
 const uint32_t a=ctx.r3.u32,t=thread(ctx,b);
 std::unique_lock<std::mutex> guard(monitor);auto&l=lookup(b,a);
 uint32_t owner=get(b,a+0x18),recursion=get(b,a+0x14);
 if(owner==t){if(recursion==0||recursion>=0x7ffffffe)invalid("recursion",a);update(b,a,l,t,recursion+1);return true;}
 if(owner){
  if(attempt)return false;
  ++l.waiters;update(b,a,l,owner,recursion);
  l.changed.wait(guard,[&]{return get(b,a+0x18)==0;});--l.waiters;
 }
 update(b,a,l,t,1);return true;
}
}
PPC_FUNC(__imp__RtlEnterCriticalSection){enter(ctx,base,false);}
PPC_FUNC(__imp__RtlTryEnterCriticalSection){ctx.r3.u64=enter(ctx,base,true)?1:0;}
PPC_FUNC(__imp__RtlLeaveCriticalSection){
 const uint32_t a=ctx.r3.u32,t=thread(ctx,base);
 std::lock_guard<std::mutex> guard(monitor);auto&l=lookup(base,a);
 const uint32_t owner=get(base,a+0x18),recursion=get(base,a+0x14);
 if(owner!=t||!recursion)invalid("release by non-owner",a);
 update(base,a,l,recursion>1?t:0,recursion-1);
 if(recursion==1)l.changed.notify_one();
}
