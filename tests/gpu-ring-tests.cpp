#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/gpu.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
int main(int argc,char**argv){
 int count=0;auto check=[&](bool v){++count;if(!v){fprintf(stderr,"FAIL ring %d\n",count);exit(1);}};
 auto*b=(uint8_t*)VirtualAlloc(nullptr,0x100000000ull,MEM_RESERVE,PAGE_NOACCESS);check(b!=nullptr);
 check(VirtualAlloc(b+0x10000,4096,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 check(VirtualAlloc(b+0xA0010000,8192,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 PPCContext c{};c.r3.u64=0;c.r4.u64=0;c.r5.u64=0;c.r6.u64=0x10000;c.r7.u64=0x10004;__imp__VdInitializeEngines(c,b);check(c.r3.u32==1);
 auto ring=[&](uint32_t p,uint32_t n){c.r3.u64=p;c.r4.u64=n;__imp__VdInitializeRingBuffer(c,b);};
 auto wb=[&](uint32_t p,uint32_t n){c.r3.u64=p;c.r4.u64=n;__imp__VdEnableRingBufferRPtrWriteBack(c,b);};
 if(argc>1){
  if(!strcmp(argv[1],"shift"))ring(0x10000,0xffffffff);
  if(!strcmp(argv[1],"alignment"))ring(0x10001,9);
  if(!strcmp(argv[1],"unmapped"))ring(0x10000,11);
  if(!strcmp(argv[1],"overflow"))ring(0x1ffffffc,9);
  if(!strcmp(argv[1],"order"))wb(0x11000,6);
  ring(0x10000,9);
  if(!strcmp(argv[1],"wbshift"))wb(0x11000,32);
  if(!strcmp(argv[1],"wbunmapped"))wb(0x12000,6);
  return 1;
 }
 b[0xA0010000]=0x91;b[0xA0011000]=0x82;
 c.lr=0x12345678;ring(0x10000,9);auto s=queryDiagnosticGpu(b);
 check(s.ringConfigured&&s.ringPhysical==0x10000&&s.ringBytes==4096&&s.ringReadIndex==0);
 check(c.r3.u32==0x10000&&c.r4.u32==9&&c.lr==0x12345678);check(b[0xA0010000]==0x91);
 wb(0x11000,6);s=queryDiagnosticGpu(b);check(s.readPointerPhysical==0x11000&&s.readPointerUpdateFrequency==16);check(b[0xA0011000]==0x82);
 ring(0x10000,10);s=queryDiagnosticGpu(b);check(s.ringBytes==8192&&s.ringReadIndex==0);check(s.readPointerPhysical==0x11000);
 wb(0x11004,0);check(queryDiagnosticGpu(b).readPointerUpdateFrequency==0);
 __imp__VdShutdownEngines(c,b);s=queryDiagnosticGpu(b);check(!s.ringConfigured&&s.ringBytes==0&&s.readPointerPhysical==0);
 check(VirtualFree(b,0,MEM_RELEASE));printf("PASS %d GPU ring checks: size, state, replacement, preserved memory and shutdown\n",count);
}
