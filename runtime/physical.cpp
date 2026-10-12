#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <map>
#include <mutex>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
namespace {
struct Region {uint32_t guest,size,protect;};
std::map<uint32_t,Region> regions; // keyed by guest physical address
std::mutex physicalMutex;
[[noreturn]] void pending(const char* why,PPCContext&ctx){fprintf(stderr,"PENDING_PHYSICAL_MODE %s guest_lr=0x%08X\n",why,uint32_t(ctx.lr));std::_Exit(20);}
uint32_t physical(uint32_t a){if(a>=0xE0000000&&a<0xFFD00000)return a-0xE0000000+0x1000;if(a>=0xA0000000&&a<0xE0000000)return a&0x1FFFFFFF;return 0xFFFFFFFF;}
auto containing(uint32_t a){for(auto it=regions.begin();it!=regions.end();++it)if(a>=it->second.guest&&uint64_t(a)<uint64_t(it->second.guest)+it->second.size)return it;return regions.end();}
uint32_t allocate(PPCContext&ctx,uint8_t*b,uint32_t kind,uint32_t size,uint32_t protect,uint32_t low,uint32_t high,uint32_t alignment){
 fprintf(stderr,"Physical request size=0x%X protect=0x%X min=0x%X max=0x%X align=0x%X\n",size,protect,low,high,alignment);
 if(kind>2||(protect&~0xA0000606u))pending("flags or protection",ctx);
 if((protect&6)!=2&&(protect&6)!=4)return 0;
 if((protect&0xA0000000)==0xA0000000||(protect&0x600)==0x600)return 0;
 uint32_t page=0x1000,heap=0xE0000000,offset=0x1000;
 uint64_t end=0x1FD01000;
 if(protect&0x20000000){page=0x10000;heap=0xA0000000;offset=0;end=0x1FFF0000;}
 else if(protect&0x80000000){page=0x1000000;heap=0xC0000000;offset=0;end=0x1FFF0000;}
 if(!size||low>high||(alignment&&(alignment&(alignment-1))))return 0;
 const uint64_t bytes=(uint64_t(size)+page-1)&~uint64_t(page-1);
 const uint64_t align=std::max(page,alignment);
 end=std::min(end,uint64_t(high)+1);const uint64_t begin=std::max(low,offset);
 std::lock_guard<std::mutex> guard(physicalMutex);
 while(end>=bytes){
  const uint64_t candidate=((end-bytes)/align)*align;
  if(candidate<begin)return 0;
  bool collision=false;
  for(auto it=regions.rbegin();it!=regions.rend();++it){
   if(candidate<uint64_t(it->first)+it->second.size&&it->first<candidate+bytes){end=it->first;collision=true;break;}
  }
  if(collision)continue;
  const uint32_t address=uint32_t(uint64_t(heap)+candidate-offset);
  // CPU backing only. Cache hints are retained as metadata; no GPU or alias
  // mapping is claimed. Host pages remain ordinary 4 KiB Windows pages.
  if(!VirtualAlloc(b+address,size_t(bytes),MEM_COMMIT,(protect&4)?PAGE_READWRITE:PAGE_READONLY))return 0;
  regions.emplace(uint32_t(candidate),Region{address,uint32_t(bytes),protect});
  fprintf(stderr,"Physical allocation guest=0x%08X physical=0x%08X size=0x%X\n",address,uint32_t(candidate),uint32_t(bytes));
  return address;
 }
 return 0;
}
}
PPC_FUNC(__imp__MmAllocatePhysicalMemoryEx){ctx.r3.u64=allocate(ctx,base,ctx.r3.u32,ctx.r4.u32,ctx.r5.u32,ctx.r6.u32,ctx.r7.u32,ctx.r8.u32);}
PPC_FUNC(__imp__MmAllocatePhysicalMemory){ctx.r3.u64=allocate(ctx,base,ctx.r3.u32,ctx.r4.u32,ctx.r5.u32,0,0xffffffff,0);}
PPC_FUNC(__imp__MmFreePhysicalMemory){
 std::lock_guard<std::mutex> guard(physicalMutex);const uint32_t address=ctx.r4.u32;
 auto it=containing(address);if(it==regions.end()||it->second.guest!=address){fprintf(stderr,"INVALID_PHYSICAL_FREE address=0x%08X\n",address);std::_Exit(22);}
 if(!VirtualFree(base+address,it->second.size,MEM_DECOMMIT)){fprintf(stderr,"PHYSICAL_FREE_FAILED\n");std::_Exit(22);}regions.erase(it);
}
PPC_FUNC(__imp__MmGetPhysicalAddress){ctx.r3.u64=physical(ctx.r3.u32);}
PPC_FUNC(__imp__MmQueryAllocationSize){
 std::lock_guard<std::mutex> guard(physicalMutex);auto it=containing(ctx.r3.u32);
 if(it==regions.end())pending("allocation-size query outside physical allocation",ctx);
 ctx.r3.u64=it->second.size;
}
PPC_FUNC(__imp__MmQueryAddressProtect){
 std::lock_guard<std::mutex> guard(physicalMutex);auto it=containing(ctx.r3.u32);
 if(it==regions.end())pending("protection query outside physical allocation",ctx);
 ctx.r3.u64=it->second.protect&0x606;
}
