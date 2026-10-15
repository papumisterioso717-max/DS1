#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
static int checks;
static void check(bool b){++checks;if(!b)exit(1);}
int main(int argc,char**argv){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,16384,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);
 // Two independent KPCR/KTHREAD pairs.
 b[0x202]=0x10;b[0x402]=0x20;
 PPCContext c{};c.r13.u64=0x100;c.r3.u64=123;c.lr=456;
 auto count=[&](uint32_t a){int32_t v;memcpy(&v,b+a+0xB0,4);return v;};
 if(argc>1){
  if(!strcmp(argv[1],"unbalanced"))__imp__KeLeaveCriticalRegion(c,b);
  if(!strcmp(argv[1],"null")){c.r13.u64=0;__imp__KeEnterCriticalRegion(c,b);}
  if(!strcmp(argv[1],"overflow")){int32_t v=INT32_MIN;memcpy(b+0x10B0,&v,4);__imp__KeEnterCriticalRegion(c,b);}
  if(!strcmp(argv[1],"readonly")){DWORD old;VirtualProtect(b+0x1000,4096,PAGE_READONLY,&old);__imp__KeEnterCriticalRegion(c,b);}
  return 2;
 }
 unsigned char before[16384];memcpy(before,b,sizeof(before));
 __imp__KeEnterCriticalRegion(c,b);check(count(0x1000)==-1);
 __imp__KeEnterCriticalRegion(c,b);check(count(0x1000)==-2);
 c.r13.u64=0x300;__imp__KeEnterCriticalRegion(c,b);check(count(0x2000)==-1&&count(0x1000)==-2);
 __imp__KeLeaveCriticalRegion(c,b);check(count(0x2000)==0);
 c.r13.u64=0x100;__imp__KeLeaveCriticalRegion(c,b);check(count(0x1000)==-1);
 __imp__KeLeaveCriticalRegion(c,b);check(count(0x1000)==0);
 check(c.r3.u64==123&&c.lr==456);check(!memcmp(before,b,sizeof(before)));
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d critical-region checks\n",checks);
}
