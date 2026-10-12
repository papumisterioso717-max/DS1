#define NOMINMAX
#include <windows.h>
#include "memory.h"
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
namespace {
constexpr uint32_t invalid=0xC000000D, noMemory=0xC0000017, conflict=0xC0000018, unsupported=0xC0000002;
constexpr uint32_t first=0x10000000, last=0x20000000, page=0x1000;
bool pointerOK(uint8_t*base,uint32_t address){
 if(!address||uint64_t(address)+4>0x100000000ull)return false;
 // Check both ends in case the four-byte output crosses a page boundary.
 for(uint32_t delta: {0u,3u}){
  MEMORY_BASIC_INFORMATION info{};
  if(!VirtualQuery(base+address+delta,&info,sizeof(info))||info.State!=MEM_COMMIT||
     (info.Protect&PAGE_GUARD)||!(info.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE)))return false;
 }
 return true;
}
uint32_t read32(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
void write32(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
}
namespace deadspace {
// Xbox ABI and flag meanings verified against Xenia xboxkrnl_memory.cc and xbox.h.
// 4 KiB and 64 KiB guest virtual pages are implemented in this diagnostic runtime.
uint32_t VirtualMemory::allocate(uint32_t& address,uint32_t& size,uint32_t flags,uint32_t protection,uint32_t debug){
 std::lock_guard<std::mutex> lock(mutex_);
 if(!size||!(flags&0x3000))return invalid;
 if(debug||(flags&~0x60903000u))return unsupported; // reserve, commit, top-down, no-zero, large pages, heap
 if(protection!=PAGE_READWRITE&&protection!=PAGE_READONLY&&protection!=PAGE_NOACCESS)return unsupported;
 uint32_t pageSize=(flags&0x20000000)?0x10000:page;
 if(address){auto owner=reservations_.upper_bound(address);if(owner!=reservations_.begin()){--owner;if(address<uint64_t(owner->first)+owner->second)pageSize=pageSizes_.at(owner->first);}}
 const uint64_t bytes=(uint64_t(size)+pageSize-1)&~uint64_t(pageSize-1);
 uint32_t start=address&~(pageSize-1);
 if(bytes>last-first)return noMemory;
 if(address&&(start<first||uint64_t(start)+bytes>last))return conflict;
 auto containing=reservations_.upper_bound(start);
 bool existing=false;
 if(containing!=reservations_.begin()){
  --containing;
  existing=start>=containing->first&&uint64_t(start)+bytes<=uint64_t(containing->first)+containing->second;
 }
 const bool reserve=(flags&0x2000)!=0;
 if(address&&existing&&reserve)return conflict;
 if(address&&!existing&&!reserve)return conflict;
 if(!address){
  start=first;
  if(flags&0x100000){
   uint64_t end=last;
   for(auto it=reservations_.rbegin();it!=reservations_.rend();++it){
    if(end>=bytes&&((end-bytes)&~uint64_t(pageSize-1))>=uint64_t(it->first)+it->second)break;
    end=it->first;
   }
   if(end<first+bytes)return noMemory;
   start=uint32_t((end-bytes)&~uint64_t(pageSize-1));
  }else{
   for(const auto& r:reservations_){if(uint64_t(start)+bytes<=r.first)break;start=uint32_t((uint64_t(r.first)+r.second+pageSize-1)&~uint64_t(pageSize-1));}
   if(uint64_t(start)+bytes>last)return noMemory;
  }
 }else if(!existing){
  for(const auto&r:reservations_)if(start<uint64_t(r.first)+r.second&&r.first<uint64_t(start)+bytes)return conflict;
 }
 // Host range is already reserved by main. Commit preserves previously committed
 // bytes and zero-initializes newly committed pages, including after decommit.
 if(flags&0x1000){
  if(!VirtualAlloc(base_+start,size_t(bytes),MEM_COMMIT,protection))return noMemory;
 }
 if(!existing){reservations_.emplace(start,uint32_t(bytes));pageSizes_.emplace(start,pageSize);}
 address=start;size=uint32_t(bytes);return 0;
}
uint32_t VirtualMemory::release(uint32_t& address,uint32_t& size,uint32_t flags,uint32_t debug){
 std::lock_guard<std::mutex> lock(mutex_);
 if(debug)return unsupported;
 if(flags!=MEM_RELEASE&&flags!=MEM_DECOMMIT)return invalid;
 auto it=reservations_.upper_bound(address);
 if(it==reservations_.begin())return conflict;
 --it;
 if(address>=uint64_t(it->first)+it->second)return conflict;
 if(flags==MEM_RELEASE){
  if(address!=it->first||size!=0)return invalid;
  if(!VirtualFree(base_+address,it->second,MEM_DECOMMIT))return noMemory;
  size=it->second;pageSizes_.erase(it->first);reservations_.erase(it);return 0;
 }
 const uint32_t pageSize=pageSizes_.at(it->first);
 const uint32_t start=address&~(pageSize-1);
 const uint64_t bytes=(uint64_t(size)+pageSize-1)&~uint64_t(pageSize-1);
 if(!size||uint64_t(start)+bytes>uint64_t(it->first)+it->second)return invalid;
 if(!VirtualFree(base_+start,size_t(bytes),MEM_DECOMMIT))return noMemory;
 address=start;size=uint32_t(bytes);return 0;
}
}
static deadspace::VirtualMemory& manager(uint8_t*base){static deadspace::VirtualMemory m(base);return m;}
PPC_FUNC(__imp__NtAllocateVirtualMemory){
 const uint32_t ap=ctx.r3.u32,sp=ctx.r4.u32;
 if(!pointerOK(base,ap)||!pointerOK(base,sp)){ctx.r3.u64=invalid;return;}
 uint32_t a=read32(base,ap),s=read32(base,sp);
 const auto result=manager(base).allocate(a,s,ctx.r5.u32,ctx.r6.u32,ctx.r7.u32);
 fprintf(stderr,"NtAllocateVirtualMemory flags=0x%08X protect=0x%X address=0x%08X size=0x%X status=0x%08X\n",ctx.r5.u32,ctx.r6.u32,a,s,result);
 if(result==unsupported){fprintf(stderr,"PENDING_MEMORY_MODE guest_lr=0x%08X\n",uint32_t(ctx.lr));std::_Exit(20);}
 if(!result){write32(base,ap,a);write32(base,sp,s);}ctx.r3.u64=result;
}
PPC_FUNC(__imp__NtFreeVirtualMemory){
 const uint32_t ap=ctx.r3.u32,sp=ctx.r4.u32;
 if(!pointerOK(base,ap)||!pointerOK(base,sp)){ctx.r3.u64=invalid;return;}
 uint32_t a=read32(base,ap),s=read32(base,sp);
 const auto result=manager(base).release(a,s,ctx.r5.u32,ctx.r6.u32);
 if(result==unsupported){fprintf(stderr,"PENDING_MEMORY_FREE_MODE\n");std::_Exit(20);}
 if(!result){write32(base,ap,a);write32(base,sp,s);}ctx.r3.u64=result;
}
