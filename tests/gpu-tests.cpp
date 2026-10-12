#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/gpu.h"
#include <cstdio>
#include <cstdlib>
int main(int argc,char**argv){
 int checks=0;auto check=[&](bool v){++checks;if(!v){fprintf(stderr,"FAIL gpu %d\n",checks);exit(1);}};
 auto*b=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);check(b!=nullptr);PPCContext c{};
 auto init=[&](uint32_t pfp,uint32_t cb){c.r3.u64=0x1B530000;c.r4.u64=cb;c.r5.u64=123;c.r6.u64=pfp;c.r7.u64=128;__imp__VdInitializeEngines(c,b);return c.r3.u32;};
 check(!queryDiagnosticGpu(b).initialized);check(init(0,0)==0);check(init(0xFFFFFFFF,0)==0);check(init(64,1)==0);check(!queryDiagnosticGpu(b).initialized);
 b[64]=0xAA;b[128]=0xBB;check(init(64,PPC_CODE_BASE)==1);auto s=queryDiagnosticGpu(b);check(s.initialized&&s.flags==0x1B530000&&s.callback==PPC_CODE_BASE&&s.argument==123&&s.pfp==64&&s.me==128);check(b[64]==0xAA&&b[128]==0xBB);

 b[192]=5;b[193]=0;b[194]=2;b[195]=0xD0;b[196]=2;b[197]=0x80;b[198]=1;b[199]=0xE0;
 c.r3.u64=argc>1&&argv[1][0]=='e'?2:1;c.r4.u64=argc>1&&argv[1][0]=='p'?0xFFFFFFFF:192;
 __imp__VdCallGraphicsNotificationRoutines(c,b);
 check(c.r3.u32==0&&c.r4.u32==192);s=queryDiagnosticGpu(b);
 check(s.notificationCount==1&&s.frontWidth==1280&&s.frontHeight==720&&s.backWidth==640&&s.backHeight==480);
 check(b[192]==5&&b[199]==0xE0);
 c.r3.u64=1;__imp__VdCallGraphicsNotificationRoutines(c,b);check(queryDiagnosticGpu(b).notificationCount==2);
c.r3.u64=PPC_CODE_BASE;c.r4.u64=1234;__imp__VdSetGraphicsInterruptCallback(c,b);check(queryDiagnosticGpu(b).interruptCallback==PPC_CODE_BASE&&queryDiagnosticGpu(b).interruptArgument==1234);c.r3.u64=0;c.r4.u64=0;__imp__VdSetGraphicsInterruptCallback(c,b);check(queryDiagnosticGpu(b).interruptCallback==0);
 c.r3.u64=64;c.r4.u64=0x12345678;c.lr=0x87654320;__imp__VdSetSystemCommandBufferGpuIdentifierAddress(c,b);
 check(queryDiagnosticGpu(b).systemCommandIdentifierAddress==64);check(c.r3.u32==64&&c.r4.u32==0x12345678&&c.lr==0x87654320);check(b[64]==0xAA);
 c.r3.u64=128;__imp__VdSetSystemCommandBufferGpuIdentifierAddress(c,b);check(queryDiagnosticGpu(b).systemCommandIdentifierAddress==128);check(b[128]==0xBB);
 c.r3.u64=0;__imp__VdSetSystemCommandBufferGpuIdentifierAddress(c,b);check(queryDiagnosticGpu(b).systemCommandIdentifierAddress==0);
 c.r3.u64=64;__imp__VdSetSystemCommandBufferGpuIdentifierAddress(c,b);
 check(init(0,0)==0);check(queryDiagnosticGpu(b).pfp==64);__imp__VdShutdownEngines(c,b);check(!queryDiagnosticGpu(b).initialized);
 check(queryDiagnosticGpu(b).systemCommandIdentifierAddress==0);c.r3.u64=0;__imp__VdSetSystemCommandBufferGpuIdentifierAddress(c,b);check(!queryDiagnosticGpu(b).initialized);
 check(init(64,0)==1);__imp__VdShutdownEngines(c,b);__imp__VdShutdownEngines(c,b);check(!queryDiagnosticGpu(b).initialized);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d GPU diagnostic checks: captured state, bad inputs, unchanged microcode and shutdown\n",checks);
}
