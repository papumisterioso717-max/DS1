#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "physical.h"
#include <vector>
#include <map>
#include <mutex>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
namespace {
using Alloc2 = PVOID (WINAPI*)(HANDLE,PVOID,SIZE_T,ULONG,ULONG,void*,ULONG);
using Map3 = PVOID (WINAPI*)(HANDLE,HANDLE,PVOID,ULONG64,SIZE_T,ULONG,ULONG,void*,ULONG);
using Unmap2 = BOOL (WINAPI*)(HANDLE,PVOID,ULONG);
const auto alloc2=reinterpret_cast<Alloc2>(GetProcAddress(GetModuleHandleW(L"kernelbase.dll"),"VirtualAlloc2"));
const auto map3=reinterpret_cast<Map3>(GetProcAddress(GetModuleHandleW(L"kernelbase.dll"),"MapViewOfFile3"));
const auto unmap2=reinterpret_cast<Unmap2>(GetProcAddress(GetModuleHandleW(L"kernelbase.dll"),"UnmapViewOfFile2"));
constexpr ULONG replacePlaceholder=0x4000,preservePlaceholder=2,coalescePlaceholders=1;
struct Region {uint32_t guest,size,protect;std::vector<uint8_t*> views;};
// Split/coalesce reserved placeholders without ever exposing a free-address gap.
bool placeholder(uint8_t* address,size_t size){
 MEMORY_BASIC_INFORMATION m{};
 if(!VirtualQuery(address,&m,sizeof(m))||m.State!=MEM_RESERVE||m.Type!=MEM_PRIVATE)return false;
 auto* start=static_cast<uint8_t*>(m.AllocationBase);
 if(start<address && !VirtualFree(start,address-start,MEM_RELEASE|preservePlaceholder))return false;
 if(!VirtualQuery(address,&m,sizeof(m)))return false;
 size_t span=m.RegionSize;
 while(span<size){
  MEMORY_BASIC_INFORMATION next{};
  if(!VirtualQuery(address+span,&next,sizeof(next))||next.State!=MEM_RESERVE||next.Type!=MEM_PRIVATE)return false;
  span+=next.RegionSize;
  if(!VirtualFree(address,span,MEM_RELEASE|coalescePlaceholders)){fprintf(stderr,"COALESCE_FAILED span=%llX next=%llX error=%lu\n",(unsigned long long)span,(unsigned long long)next.RegionSize,GetLastError());return false;}
 }
 return span==size||VirtualFree(address,size,MEM_RELEASE|preservePlaceholder);
}
void unmapViews(const std::vector<uint8_t*>& views){
 for(auto* v:views)if(!unmap2(GetCurrentProcess(),v,preservePlaceholder)){fprintf(stderr,"PHYSICAL_UNMAP_FAILED %lu\n",GetLastError());std::_Exit(22);}
}
bool mapPhysical(uint8_t* b,uint32_t p,uint32_t size,uint32_t protection,std::vector<uint8_t*>& views){
 HANDLE section=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,size,nullptr);
 if(!section)return false;
 bool ok=true;
 // Each alias maps the same section. The 4 KiB heap starts at physical 0x1000.
 for(uint64_t heap:{0xA0000000ull,0xC0000000ull,0xE0000000ull}){
  uint64_t first=p,last=uint64_t(p)+size;
  if(heap==0xE0000000){first=std::max(first,0x1000ull);last=std::min(last,0x1FD01000ull);}
  if(first>=last)continue;
  auto* address=b+heap+first-(heap==0xE0000000?0x1000:0);
  if(!placeholder(address,size_t(last-first))||!map3(section,GetCurrentProcess(),address,first-p,size_t(last-first),replacePlaceholder,protection,nullptr,0)){ok=false;break;}
  views.push_back(address);
 }
 const auto error=GetLastError();CloseHandle(section);
 if(!ok){fprintf(stderr,"MAP_STAGE physical=%X views=%zu\n",p,views.size());unmapViews(views);views.clear();fprintf(stderr,"PHYSICAL_MAP_FAILED %lu\n",error);}
 return ok;
}
std::map<uint32_t,Region> regions; // keyed by guest physical address
std::mutex physicalMutex;
[[noreturn]] void pending(const char* why,PPCContext&ctx){fprintf(stderr,"PENDING_PHYSICAL_MODE %s guest_lr=0x%08X\n",why,uint32_t(ctx.lr));std::_Exit(20);}
uint32_t physical(uint32_t a){if(a>=0xE0000000&&a<0xFFD00000)return a-0xE0000000+0x1000;if(a>=0xA0000000&&a<0xE0000000)return a&0x1FFFFFFF;return 0xFFFFFFFF;}
auto containing(uint32_t a){for(auto it=regions.begin();it!=regions.end();++it)if(physical(a)>=it->first&&uint64_t(physical(a))<uint64_t(it->first)+it->second.size)return it;return regions.end();}
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
  std::vector<uint8_t*> views;
  if(!mapPhysical(b,uint32_t(candidate),uint32_t(bytes),(protect&4)?PAGE_READWRITE:PAGE_READONLY,views))return 0;
  regions.emplace(uint32_t(candidate),Region{address,uint32_t(bytes),protect,std::move(views)});
  fprintf(stderr,"Physical allocation guest=0x%08X physical=0x%08X size=0x%X\n",address,uint32_t(candidate),uint32_t(bytes));
  return address;
 }
 return 0;
}
}
uint8_t* reserveGuestMemory(){
 if(!alloc2||!map3||!unmap2){SetLastError(ERROR_NOT_SUPPORTED);return nullptr;}
 auto* b=static_cast<uint8_t*>(alloc2(GetCurrentProcess(),nullptr,0x100000000ull,MEM_RESERVE|0x40000,PAGE_NOACCESS,nullptr,0));
 if(!b)return nullptr;
 if(!VirtualFree(b,0xA0000000ull,MEM_RELEASE|preservePlaceholder)) {VirtualFree(b,0,MEM_RELEASE);return nullptr;}
 if(!alloc2(GetCurrentProcess(),b,0xA0000000ull,MEM_RESERVE|replacePlaceholder,PAGE_NOACCESS,nullptr,0)){
  VirtualFree(b,0,MEM_RELEASE);VirtualFree(b+0xA0000000ull,0,MEM_RELEASE);return nullptr;
 }
 return b;
}
bool releaseGuestMemory(uint8_t* b){
 // The low private reservation may contain many separately committed ranges.
 if(!VirtualFree(b,0,MEM_RELEASE))return false;
 for(size_t offset=0xA0000000ull;offset<0x100000000ull;){
  MEMORY_BASIC_INFORMATION m{};
  if(!VirtualQuery(b+offset,&m,sizeof(m))||m.State!=MEM_RESERVE||m.Type!=MEM_PRIVATE)return false;
  if(!VirtualFree(b+offset,0,MEM_RELEASE))return false;
  offset+=m.RegionSize;
 }
 return true;
}
PPC_FUNC(__imp__MmAllocatePhysicalMemoryEx){ctx.r3.u64=allocate(ctx,base,ctx.r3.u32,ctx.r4.u32,ctx.r5.u32,ctx.r6.u32,ctx.r7.u32,ctx.r8.u32);}
PPC_FUNC(__imp__MmAllocatePhysicalMemory){ctx.r3.u64=allocate(ctx,base,ctx.r3.u32,ctx.r4.u32,ctx.r5.u32,0,0xffffffff,0);}
PPC_FUNC(__imp__MmFreePhysicalMemory){
 std::lock_guard<std::mutex> guard(physicalMutex);const uint32_t address=ctx.r4.u32;
 auto it=containing(address);if(it==regions.end()||it->second.guest!=address){fprintf(stderr,"INVALID_PHYSICAL_FREE address=0x%08X\n",address);std::_Exit(22);}
 unmapViews(it->second.views);regions.erase(it);
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
