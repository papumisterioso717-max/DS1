#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
static int checks;void check(bool b){++checks;if(!b)exit(1);}
int main(int argc,char**argv){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);
 memset(b,0xA5,4096);memset(b+64,0,8);
 PPCContext c{};c.r3.u64=0;c.r4.u64=64;c.r5.u64=4096;c.r6.u64=68;c.r7.u64=128;c.r8.u64=2048;c.lr=123;
 if(argc>1){
  if(!strcmp(argv[1],"worker")){c.r3.u64=1;__imp__VdRetrainEDRAMWorker(c,b);return 2;}
  if(!strcmp(argv[1],"nonempty"))b[67]=1;
  if(!strcmp(argv[1],"invalid"))c.r4.u64=0;
  if(!strcmp(argv[1],"capacity"))c.r8.u64=1;
  __imp__VdRetrainEDRAM(c,b);return 2;
 }
 unsigned char before[4096];memcpy(before,b,4096);
 __imp__VdRetrainEDRAMWorker(c,b);check(c.r3.u64==0&&c.r4.u64==64&&c.lr==123);
 c.r3.u64=0x1234;__imp__VdRetrainEDRAM(c,b);
 check(c.r3.u64==0);check(c.r4.u64==64&&c.r5.u64==4096&&c.r6.u64==68&&c.r7.u64==128&&c.r8.u64==2048&&c.lr==123);
 check(!memcmp(before,b,4096));
 c.r3.u64=0xFFFFFFFFFFFFFFFFull;__imp__VdIsHSIOTrainingSucceeded(c,b);
 check(c.r3.u64==1);check(c.r4.u64==64&&c.r8.u64==2048&&c.lr==123);
 check(!memcmp(before,b,4096));
 // The title computes a failure flag from whether this return is zero.
 check((c.r3.u32==0?1:0)==0);
 VirtualFree(b,0,MEM_RELEASE);
 printf("PASS %d EDRAM compatibility checks\n",checks);
}
