#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL physical %d\n",checks);exit(1);}}
int main(){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,0x100000000ull,MEM_RESERVE,PAGE_NOACCESS));check(b!=nullptr);PPCContext c{};
 auto alloc=[&](uint32_t size,uint32_t protect,uint32_t low,uint32_t high,uint32_t align){c.r3.u64=0;c.r4.u64=size;c.r5.u64=protect;c.r6.u64=low;c.r7.u64=high;c.r8.u64=align;__imp__MmAllocatePhysicalMemoryEx(c,b);return c.r3.u32;};
 auto phys=[&](uint32_t a){c.r3.u64=a;__imp__MmGetPhysicalAddress(c,b);return c.r3.u32;};
 auto release=[&](uint32_t a){c.r3.u64=0;c.r4.u64=a;__imp__MmFreePhysicalMemory(c,b);};
 check(alloc(0,4,0,0xffffffff,0)==0);check(alloc(4096,4,8192,4095,0)==0);check(alloc(4096,4,0,0xffffffff,3)==0);
 check(alloc(0xffffffff,4,0,0xffffffff,0)==0);
 uint32_t a=alloc(1,4,0x10000,0x1ffff,0x4000);check(a!=0);auto p=phys(a);check(p>=0x10000&&uint64_t(p)+4096<=0x20000&&(p%0x4000)==0);
 c.r3.u64=a;__imp__MmQueryAllocationSize(c,b);check(c.r3.u32==4096);check(b[a]==0&&b[a+4095]==0);b[a]=42;
 c.r3.u64=a;__imp__MmQueryAddressProtect(c,b);check(c.r3.u32==4);
 uint32_t large=alloc(1,0x20000004,0,0x1ffff,0);check(large>=0xa0000000&&large<0xc0000000);
 auto lp=phys(large);check(lp%65536==0);check(uint64_t(lp)+65536<=p||lp>=p+4096);
 check(b[a]==42);release(large);release(a);
 auto reused=alloc(1,4,0x10000,0x1ffff,0x4000);check(reused==a&&b[reused]==0);release(reused);
 auto huge=alloc(1,0x80000002,0,0xffffffff,0);check(huge>=0xc0000000&&huge<0xe0000000);
 c.r3.u64=huge;__imp__MmQueryAllocationSize(c,b);check(c.r3.u32==0x1000000);
 MEMORY_BASIC_INFORMATION m{};VirtualQuery(b+huge,&m,sizeof(m));check(m.Protect==PAGE_READONLY);release(huge);
 auto exact=alloc(4096,4,0x10000,0x10fff,0);check(phys(exact)==0x10000);check(alloc(4096,4,0x10000,0x10fff,0)==0);release(exact);
 check(phys(0x82000000)==0xffffffff);check(phys(0xe0000000)==4096);check(phys(0xa0000000)==0);check(phys(0xc0000000)==0);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d physical-memory checks: ranges, rounding, alignment, overlap, protection and reuse\n",checks);
}
