#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "tls.h"
#include <map>
#include <vector>
#include <mutex>
#include <cstring>
#include <cstdio>
#include <cstdlib>
namespace {
struct State {std::vector<bool> used;std::map<uint32_t,uint32_t> dynamic;};
std::map<uint8_t*,State> states;
std::mutex mutex;
[[noreturn]] void fail(){fprintf(stderr,"INVALID_TLS_STATE\n");std::_Exit(22);}
void range(uint8_t*b,uint32_t a,uint64_t n,bool write){
 if(!a||uint64_t(a)+n>0x100000000ull)fail();
 for(uint64_t i=0;i<n;){MEMORY_BASIC_INFORMATION m{};
  if(!VirtualQuery(b+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
     (write&&!(m.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))))fail();
  i=uint64_t(static_cast<uint8_t*>(m.BaseAddress)-(b+a))+m.RegionSize;
 }
}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
State& current(uint8_t*b,uint32_t pcr){auto it=states.find(b);if(it==states.end()||!it->second.dynamic.count(pcr))fail();return it->second;}
}
void registerGuestTls(uint8_t*b,uint32_t pcr,uint32_t storage,uint32_t slots,uint32_t dataSize,uint32_t raw,uint32_t rawSize){
 std::lock_guard<std::mutex> guard(mutex);
 if(!slots||slots>2048||rawSize>dataSize)fail();
 range(b,pcr,4,true);range(b,storage,uint64_t(dataSize)+slots*4,true);if(rawSize)range(b,raw,rawSize,false);
 auto&s=states[b];if(s.used.empty())s.used.resize(slots);if(s.used.size()!=slots||s.dynamic.count(pcr))fail();
 memset(b+storage,0,size_t(dataSize)+slots*4);if(rawSize)memcpy(b+storage,b+raw,rawSize);
 put(b,pcr,storage);s.dynamic.emplace(pcr,storage+dataSize);
}
PPC_FUNC(__imp__KeTlsAlloc){
 std::lock_guard<std::mutex> guard(mutex);auto&s=current(base,ctx.r13.u32);
 for(uint32_t i=0;i<s.used.size();++i)if(!s.used[i]){s.used[i]=true;put(base,s.dynamic.at(ctx.r13.u32)+4*i,0);ctx.r3.u64=i;return;}
 ctx.r3.u64=0xffffffff;
}
PPC_FUNC(__imp__KeTlsGetValue){
 std::lock_guard<std::mutex> guard(mutex);auto&s=current(base,ctx.r13.u32);uint32_t slot=ctx.r3.u32;
 ctx.r3.u64=slot<s.used.size()?get(base,s.dynamic.at(ctx.r13.u32)+4*slot):0;
}
PPC_FUNC(__imp__KeTlsSetValue){
 std::lock_guard<std::mutex> guard(mutex);auto&s=current(base,ctx.r13.u32);uint32_t slot=ctx.r3.u32;
 if(slot>=s.used.size()){ctx.r3.u64=0;return;}put(base,s.dynamic.at(ctx.r13.u32)+4*slot,ctx.r4.u32);ctx.r3.u64=1;
}
PPC_FUNC(__imp__KeTlsFree){
 std::lock_guard<std::mutex> guard(mutex);auto&s=current(base,ctx.r13.u32);uint32_t slot=ctx.r3.u32;
 if(slot>=s.used.size()||!s.used[slot]){ctx.r3.u64=0;return;}
 for(const auto&t:s.dynamic)put(base,t.second+4*slot,0);
 s.used[slot]=false;ctx.r3.u64=1;
}

void unregisterGuestTls(uint8_t*b,uint32_t pcr){
 std::lock_guard<std::mutex> guard(mutex);auto it=states.find(b);if(it!=states.end())it->second.dynamic.erase(pcr);
}
