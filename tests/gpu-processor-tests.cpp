#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/gpu.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
int main(int argc,char**argv){
 int n=0;auto check=[&](bool ok){++n;if(!ok){fprintf(stderr,"FAIL processor %d\n",n);exit(1);}};
 auto*base=(uint8_t*)VirtualAlloc(nullptr,0x100000000ull,MEM_RESERVE,PAGE_NOACCESS);check(base!=nullptr);
 check(VirtualAlloc(base+0x10000,4096,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 check(VirtualAlloc(base+0xA0010000,0x3000,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 PPCContext c{};c.r6.u64=0x10000;c.r7.u64=0x10004;__imp__VdInitializeEngines(c,base);check(c.r3.u32==1);
 c.r3.u64=0x10000;c.r4.u64=2;__imp__VdInitializeRingBuffer(c,base);
 c.r3.u64=0x11000;c.r4.u64=6;__imp__VdEnableRingBufferRPtrWriteBack(c,base);
 auto put=[&](uint32_t i,uint32_t v){PPC_STORE_U32(0xA0010000+i*4,v);};
 if(argc>1){
  if(!strcmp(argv[1],"opaqueadjacent")){put(0,0x00010082);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  if(!strcmp(argv[1],"rmwunknown")||!strcmp(argv[1],"rmwshort")||!strcmp(argv[1],"rmwmode")){
   put(0,!strcmp(argv[1],"rmwshort")?0xC0012100:0xC0022100);
   put(1,!strcmp(argv[1],"rmwmode")?0x2000D04:0xD03);put(2,0);put(3,1);
   diagnosticMmioStore32(base,0x7FC80714,!strcmp(argv[1],"rmwshort")?3:4,0);
  }
  if(!strcmp(argv[1],"sqgap")){put(0,0x00010D02);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  if(!strcmp(argv[1],"waitvsync")||!strcmp(argv[1],"waitfifo")||!strcmp(argv[1],"waitdirty")){
   put(0,0x000005C8);put(1,!strcmp(argv[1],"waitvsync")?8:0x400);uint32_t producer=2;
   if(!strcmp(argv[1],"waitdirty")){put(0,0x00000A31);put(1,0x01000000);put(2,0x000005C8);put(3,0x20000);producer=4;}
   diagnosticMmioStore32(base,0x7FC80714,producer,0);
  }
  if(!strcmp(argv[1],"scratchbad")||!strcmp(argv[1],"scratchreadonly")){
   put(0,0x000101DC);put(1,1);put(2,!strcmp(argv[1],"scratchbad")?0x1FFFFFFF:0x12000);
   if(!strcmp(argv[1],"scratchreadonly")){DWORD old;VirtualProtect(base+0xA0012000,4096,PAGE_READONLY,&old);}
   put(3,0x00000578);put(4,123);diagnosticMmioStore32(base,0x7FC80714,5,0);
  }
  if(!strcmp(argv[1],"unknown")){put(0,0xC0002200);put(1,0);diagnosticMmioStore32(base,0x7FC80714,2,0);}
  if(!strcmp(argv[1],"truncated")){put(0,0xC0034800);diagnosticMmioStore32(base,0x7FC80714,1,0);}
  if(!strcmp(argv[1],"unknownrange")){put(0,0x00010A05);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  if(!strcmp(argv[1],"register")){put(0,0x00000A32);put(1,0);diagnosticMmioStore32(base,0x7FC80714,2,0);}
  if(!strcmp(argv[1],"waitfalse")||!strcmp(argv[1],"waitmemory")){
   put(0,0xC0043C00);put(1,!strcmp(argv[1],"waitmemory")?0x13:3);put(2,0xA2F);put(3,1);put(4,0xffffffff);put(5,0);diagnosticMmioStore32(base,0x7FC80714,6,0);
  }
  if(!strcmp(argv[1],"index"))diagnosticMmioStore32(base,0x7FC80714,8,0);
  if(!strcmp(argv[1],"cycle")){put(0,0xC0013F00);put(1,0x10000);put(2,3);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  return 1;
 }
 put(0,0xC0004800);put(1,0x12345678);put(2,0xC0013F00);put(3,0x12000);put(4,2);
 PPC_STORE_U32(0xA0012000,0x80000000);PPC_STORE_U32(0xA0012004,0);
 diagnosticMmioStore32(base,0x7FC80714,5,0);auto s=queryDiagnosticGpu(base);
 check(s.ringReadIndex==5&&s.processedPackets==4&&s.microEngineWords==1);check(PPC_LOAD_U32(0xA0011000)==5);
 for(auto i:{5u,6u,7u,0u})put(i,0x80000000);
 diagnosticMmioStore32(base,0x7FC80714,1,0);s=queryDiagnosticGpu(base);check(s.ringReadIndex==1&&s.processedPackets==8);check(PPC_LOAD_U32(0xA0011000)==1);
 diagnosticMmioStore32(base,0x10000,0xAABBCCDD,0);check(PPC_LOAD_U32(0x10000)==0xAABBCCDD);
 diagnosticMmioStore32(base,0x7FC80714,1,0);check(queryDiagnosticGpu(base).processedPackets==8);
 // Sequential type-0 writes stage base/size/status without clearing pending.
 put(1,0x00020A2F);put(2,4096);put(3,0x12000);put(4,0x123);
 diagnosticMmioStore32(base,0x7FC80714,5,0);s=queryDiagnosticGpu(base);
 check(s.coherSize==4096&&s.coherBase==0x12000&&s.coherStatus==0x80000123);
 check(s.ringReadIndex==5&&PPC_LOAD_U32(0xA0011000)==5);
 // The single-register flag writes twice to the same register across wrap.
 put(5,0x00018A30);put(6,0x10000);put(7,0x11000);
 diagnosticMmioStore32(base,0x7FC80714,0,0);check(queryDiagnosticGpu(base).coherBase==0x11000);
 check(queryDiagnosticGpu(base).coherStatus==0x80000123);
 // Install the observed shared-memory coherence request, then wait for zero.
 put(0,0x00000A31);put(1,0x01000000);diagnosticMmioStore32(base,0x7FC80714,2,0);
 put(2,0xC0043C00);put(3,3);put(4,0xA31);put(5,0);put(6,0x80000000);put(7,16);
 diagnosticMmioStore32(base,0x7FC80714,0,0);check(queryDiagnosticGpu(base).coherStatus==0);check(PPC_LOAD_U32(0xA0011000)==0);
 // Verify the masked comparison against a nonzero size register.
 for(auto compare:{1u,2u,3u,4u,5u,6u,7u}){
  c.r3.u64=0x10000;c.r4.u64=2;__imp__VdInitializeRingBuffer(c,base);
  put(0,0xC0043C00);put(1,compare);put(2,0xA2F);put(3,compare==1?4097:compare==4||compare==6?4095:4096);put(4,0xffffffff);put(5,0);
  diagnosticMmioStore32(base,0x7FC80714,6,0);check(queryDiagnosticGpu(base).ringReadIndex==6);
 }
 // SQ configuration retains exact words, including repeated writes across wrap.
 put(6,0x00020D00);put(7,0x12345678);put(0,0x87654321);put(1,0xFFFFFFFF);
 diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.sqGprManagement==0x12345678&&s.sqFlowControl==0x87654321&&s.sqInstStoreManagement==0xFFFFFFFF);
 check(s.coherSize==4096&&s.coherStatus==0&&PPC_LOAD_U32(0xA0011000)==2);
 put(2,0x00018D02);put(3,1);put(4,0);
 diagnosticMmioStore32(base,0x7FC80714,5,0);s=queryDiagnosticGpu(base);
 check(s.sqInstStoreManagement==0&&s.sqGprManagement==0x12345678&&s.sqFlowControl==0x87654321);
 // The observed four-register packet wraps the ring; preserve every raw bit.
 put(5,0x00030A02);put(6,0x80000001);put(7,0xFFFFFFFF);put(0,0);put(1,0x12345678);
 diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.unknownA02[0]==0x80000001&&s.unknownA02[1]==0xFFFFFFFF&&s.unknownA02[2]==0&&s.unknownA02[3]==0x12345678);
 check(s.coherSize==4096&&s.sqGprManagement==0x12345678&&PPC_LOAD_U32(0xA0011000)==2);
 put(2,0x00018A03);put(3,1);put(4,2);
 diagnosticMmioStore32(base,0x7FC80714,5,0);s=queryDiagnosticGpu(base);
 check(s.unknownA02[1]==2&&s.unknownA02[0]==0x80000001&&s.unknownA02[3]==0x12345678);
 // Scratch writes are gated per slot; setup alone must not touch memory.
 put(5,0x000101DC);put(6,0x81);put(7,0x12000);
 diagnosticMmioStore32(base,0x7FC80714,0,0);
 check(PPC_LOAD_U32(0xA0012000)==0x80000000);
 put(0,0x00010578);put(1,0x12345678);put(2,0xFFFFFFFF);
 diagnosticMmioStore32(base,0x7FC80714,3,0);s=queryDiagnosticGpu(base);
 check(PPC_LOAD_U32(0xA0012000)==0x12345678&&PPC_LOAD_U32(0xA0012004)==0);
 check(s.scratch[0]==0x12345678&&s.scratch[1]==0xFFFFFFFF);
 put(3,0x0000057F);put(4,0xABCDEF01);diagnosticMmioStore32(base,0x7FC80714,5,0);
 check(PPC_LOAD_U32(0xA001201C)==0xABCDEF01&&base[0xA001201C]==0xAB);
 put(5,0x000001DC);put(6,0);diagnosticMmioStore32(base,0x7FC80714,7,0);
 put(7,0x00000578);put(0,0);diagnosticMmioStore32(base,0x7FC80714,1,0);
 check(PPC_LOAD_U32(0xA0012000)==0x12345678&&queryDiagnosticGpu(base).scratch[0]==0);
 // All combinations of idle bits, including clearing the register.
 for(uint32_t bits=0;bits<16;++bits){
  c.r3.u64=0x10000;c.r4.u64=2;__imp__VdInitializeRingBuffer(c,base);
  put(0,0x000005C8);put(1,bits<<14);diagnosticMmioStore32(base,0x7FC80714,2,0);
  s=queryDiagnosticGpu(base);check(s.waitUntil==(bits<<14)&&s.ringReadIndex==2&&PPC_LOAD_U32(0xA0011000)==2&&s.coherStatus==0);
 }
 put(2,0x00000D04);put(3,0xFEDCBA98);diagnosticMmioStore32(base,0x7FC80714,4,0);
 s=queryDiagnosticGpu(base);check(s.sqEoRt==0xFEDCBA98&&s.sqGprManagement==0x12345678&&PPC_LOAD_U32(0xA0011000)==4);
 put(4,0x00018D04);put(5,1);put(6,0);diagnosticMmioStore32(base,0x7FC80714,7,0);
 check(queryDiagnosticGpu(base).sqEoRt==0);
 put(7,0x00000D04);put(0,123);diagnosticMmioStore32(base,0x7FC80714,1,0);
 check(queryDiagnosticGpu(base).sqEoRt==123);
 // All immediate/register operand combinations. Sources may alias destination.
 for(uint32_t mode=0;mode<4;++mode){
  c.r3.u64=0x10000;c.r4.u64=2;__imp__VdInitializeRingBuffer(c,base);
  put(0,0x00000D04);put(1,0xF0);diagnosticMmioStore32(base,0x7FC80714,2,0);
  put(2,0xC0022100);put(3,0xD04|(mode<<30));put(4,(mode&2)?0xD04:0x0F);put(5,(mode&1)?0xD04:1);
  diagnosticMmioStore32(base,0x7FC80714,6,0);
  check(queryDiagnosticGpu(base).sqEoRt==((0xF0&((mode&2)?0xF0:0x0F))|((mode&1)?0xF0:1)));
 }
 // RMW must take the normal scratch writeback path.
 put(6,0x000001DC);put(7,1);diagnosticMmioStore32(base,0x7FC80714,0,0);
 put(0,0xC0022100);put(1,0x578);put(2,0);put(3,0xABCDEF12);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(PPC_LOAD_U32(0xA0012000)==0xABCDEF12&&queryDiagnosticGpu(base).scratch[0]==0xABCDEF12);
 // Observed RMW on the opaque register starts from a zero diagnostic value.
 check(queryDiagnosticGpu(base).unknown0081==0);
 put(4,0xC0022100);put(5,0x81);put(6,0xFFFFFFFF);put(7,0x80010000);
 diagnosticMmioStore32(base,0x7FC80714,0,0);check(queryDiagnosticGpu(base).unknown0081==0x80010000);
 put(0,0x00000081);put(1,0x12345678);diagnosticMmioStore32(base,0x7FC80714,2,0);
 put(2,0xC0022100);put(3,0x81);put(4,0xFFFFFFFF);put(5,0x80010000);
 diagnosticMmioStore32(base,0x7FC80714,6,0);
 check(queryDiagnosticGpu(base).unknown0081==0x92355678&&PPC_LOAD_U32(0xA0011000)==6);
 check(queryDiagnosticGpu(base).unknown0082==0);
 put(6,0x00000082);put(7,0x1234ABCD);diagnosticMmioStore32(base,0x7FC80714,0,0);
 put(0,0xC0022100);put(1,0x82);put(2,0xFFFFFFFF);put(3,0);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(queryDiagnosticGpu(base).unknown0082==0x1234ABCD&&queryDiagnosticGpu(base).unknown0081==0x92355678);
 __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
 check(s.unknown0082==0);
 check(s.unknown0081==0);
 check(s.sqEoRt==0);
 check(s.waitUntil==0);
 check(s.scratchAddress==0&&s.scratchMask==0&&s.scratch[7]==0);
 check(s.unknownA02[0]==0&&s.unknownA02[1]==0&&s.unknownA02[2]==0&&s.unknownA02[3]==0);
 check(s.sqGprManagement==0&&s.sqFlowControl==0&&s.sqInstStoreManagement==0);
 check(queryDiagnosticGpu(base).processedPackets==0);check(VirtualFree(base,0,MEM_RELEASE));
 printf("PASS %d processor checks: initialization, indirect list, wraparound, writeback and fallback\n",n);
}
