#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/gpu.h"
#include <cstdio>
#include <cstdlib>
int main(){
 int checks=0;auto check=[&](bool v){++checks;if(!v){fprintf(stderr,"FAIL gpu %d\n",checks);exit(1);}};
 auto*b=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);check(b!=nullptr);PPCContext c{};
 auto init=[&](uint32_t pfp,uint32_t cb){c.r3.u64=0x1B530000;c.r4.u64=cb;c.r5.u64=123;c.r6.u64=pfp;c.r7.u64=128;__imp__VdInitializeEngines(c,b);return c.r3.u32;};
 check(!queryDiagnosticGpu(b).initialized);check(init(0,0)==0);check(init(0xFFFFFFFF,0)==0);check(init(64,1)==0);check(!queryDiagnosticGpu(b).initialized);
 b[64]=0xAA;b[128]=0xBB;check(init(64,PPC_CODE_BASE)==1);auto s=queryDiagnosticGpu(b);check(s.initialized&&s.flags==0x1B530000&&s.callback==PPC_CODE_BASE&&s.argument==123&&s.pfp==64&&s.me==128);check(b[64]==0xAA&&b[128]==0xBB);
 c.r3.u64=PPC_CODE_BASE;c.r4.u64=1234;__imp__VdSetGraphicsInterruptCallback(c,b);check(queryDiagnosticGpu(b).interruptCallback==PPC_CODE_BASE&&queryDiagnosticGpu(b).interruptArgument==1234);c.r3.u64=0;c.r4.u64=0;__imp__VdSetGraphicsInterruptCallback(c,b);check(queryDiagnosticGpu(b).interruptCallback==0);
 check(init(0,0)==0);check(queryDiagnosticGpu(b).pfp==64);__imp__VdShutdownEngines(c,b);check(!queryDiagnosticGpu(b).initialized);
 check(init(64,0)==1);__imp__VdShutdownEngines(c,b);__imp__VdShutdownEngines(c,b);check(!queryDiagnosticGpu(b).initialized);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d GPU diagnostic checks: captured state, bad inputs, unchanged microcode and shutdown\n",checks);
}
