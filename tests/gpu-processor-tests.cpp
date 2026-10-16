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
  if(!strcmp(argv[1],"rastercopy")){
   c.r3.u32=0x10000;c.r4.u32=5;__imp__VdInitializeRingBuffer(c,base);
   put(0,0x00252300);for(uint32_t k=0;k<38;++k)put(k+1,0xA0000000+k);
   PPC_STORE_U32(0xA0012000,0x55AA55AA);
   diagnosticMmioStore32(base,0x7FC80714,39,0);
   auto s=queryDiagnosticGpu(base);
   for(uint32_t k=0;k<38;++k){
    if(k==2)check(s.vertexControl==0xA0000002);
    else if(k==18)check(s.aaMask==0xA0000012);
    else check(s.rasterCopyConfig[k]==0xA0000000+k);
   }
   check(PPC_LOAD_U32(0xA0012000)==0x55AA55AA&&s.completedShaderEvents==0);
   check(s.ringReadIndex==39&&PPC_LOAD_U32(0xA0011000)==39);
   __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
   for(auto v:s.rasterCopyConfig)check(v==0);
   puts("PASS raster/copy register storage, no copy side effects, writeback and reset");return 0;
  }

  if(!strcmp(argv[1],"primitives")){
   c.r3.u32=0x10000;c.r4.u32=4;__imp__VdInitializeRingBuffer(c,base);
   put(0,0x00142280);for(uint32_t k=0;k<21;++k)put(k+1,0x80000000+k);
   diagnosticMmioStore32(base,0x7FC80714,22,0);
   auto s=queryDiagnosticGpu(base);check(s.pointSize==0x80000000);
   for(uint32_t k=0;k<18;++k)check(s.primitiveConfig[k]==0x80000001+k);
   check(s.vizQueryConfig==0x80000013&&s.vgtEnhance==0x80000014);
   check(s.ringReadIndex==22&&PPC_LOAD_U32(0xA0011000)==22);
   // Overwrite one register without changing its neighbors.
   put(22,0x00002281);put(23,0x80000041);
   diagnosticMmioStore32(base,0x7FC80714,24,0);
   check(queryDiagnosticGpu(base).primitiveConfig[0]==0x80000041&&queryDiagnosticGpu(base).primitiveConfig[1]==0x80000002);
   __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
   for(auto v:s.primitiveConfig)check(v==0);check(s.vgtEnhance==0&&s.vizQueryConfig==0);
   puts("PASS primitive state block, exact bits, isolated overwrite, writeback and reset");return 0;
  }

  if(!strncmp(argv[1],"viewport",8)){
   c.r3.u32=0x10000;c.r4.u32=4;__imp__VdInitializeRingBuffer(c,base);
   bool bad=!strcmp(argv[1],"viewportbad");
   put(0,bad?0x00152100:0x00142100);for(uint32_t k=0;k<(bad?22u:21u);++k)put(k+1,0x7FC00000+k);
   diagnosticMmioStore32(base,0x7FC80714,bad?23:22,0);
   auto s=queryDiagnosticGpu(base);
   for(uint32_t k=0;k<3;++k)check(s.vertexIndexConfig[k]==0x7FC00000+k);
   check(s.primitiveResetIndex==0x7FC00003&&s.colorMask==0x7FC00004);
   for(uint32_t k=0;k<4;++k)check(s.blendConstant[k]==0x7FC00005+k);
   for(uint32_t k=0;k<3;++k)check(s.fogColor[k]==0x7FC00009+k);
   check(s.stencilRefMask[0]==0x7FC0000C&&s.stencilRefMask[1]==0x7FC0000D&&s.alphaRef==0x7FC0000E);
   for(uint32_t k=0;k<6;++k)check(s.viewport[k]==0x7FC0000F+k);
   check(s.ringReadIndex==22&&PPC_LOAD_U32(0xA0011000)==22);
   __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
   check(s.primitiveResetIndex==0&&s.alphaRef==0);for(auto v:s.viewport)check(v==0);
   puts("PASS viewport/color configuration block, exact float bits and reset");return 0;
  }

  if(!strcmp(argv[1],"surfaces")){
   c.r3.u32=0x10000;c.r4.u32=4;__imp__VdInitializeRingBuffer(c,base);
   put(0,0x000F2000);for(uint32_t k=0;k<16;++k)put(k+1,0x56780000+k);
   diagnosticMmioStore32(base,0x7FC80714,17,0);
   auto s=queryDiagnosticGpu(base);check(s.surfaceInfo==0x56780000&&s.colorInfo[0]==0x56780001&&s.depthInfo==0x56780002);
   for(uint32_t k=1;k<4;++k)check(s.colorInfo[k]==0x56780002+k);
   for(uint32_t k=0;k<8;++k)check(s.coherDestBase[k]==0x56780006+k);
   check(s.screenScissor[0]==0x5678000E&&s.screenScissor[1]==0x5678000F&&s.coverageWrittenMask==7);
   check(s.ringReadIndex==17&&PPC_LOAD_U32(0xA0011000)==17);
   __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
   check(s.depthInfo==0);for(auto v:s.colorInfo)check(v==0);
   puts("PASS surface block: four color targets, depth, existing fields, writeback and reset");return 0;
  }

  if(!strcmp(argv[1],"interpolator")){
   put(0,0x00042180);for(uint32_t k=0;k<5;++k)put(k+1,0x12340000+k);
   diagnosticMmioStore32(base,0x7FC80714,6,0);
   auto s=queryDiagnosticGpu(base);
   check(s.sqProgramControl==0x12340000&&s.sqContextMisc==0x12340001);
   check(s.sqInterpolatorControl==0x12340002&&s.sqWrapping[0]==0x12340003&&s.sqWrapping[1]==0x12340004);
   check(s.ringReadIndex==6&&PPC_LOAD_U32(0xA0011000)==6);
   __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
   check(s.sqInterpolatorControl==0&&s.sqWrapping[0]==0&&s.sqWrapping[1]==0);
   puts("PASS shader interpolator/wrapping state, writeback and reset");return 0;
  }

  if(!strncmp(argv[1],"renderstate",11)){
   c.r3.u32=0x10000;c.r4.u32=4;__imp__VdInitializeRingBuffer(c,base);
   const bool bad=!strcmp(argv[1],"renderstatebad");
   put(0,bad?0x000C2200:0x000B2200);
   for(uint32_t k=0;k<(bad?13u:12u);++k)put(k+1,0xA5000000+k);
   diagnosticMmioStore32(base,0x7FC80714,bad?14:13,0);
   auto s=queryDiagnosticGpu(base);
   check(s.depthControl==0xA5000000&&s.blendControl[0]==0xA5000001&&s.colorControl==0xA5000002&&s.hizControl==0xA5000003);
   for(uint32_t k=0;k<3;++k)check(s.clipTransformConfig[k]==0xA5000004+k);
   check(s.currentBinIdMin==0xA5000007&&s.modeControl==0xA5000008);
   for(uint32_t k=1;k<4;++k)check(s.blendControl[k]==0xA5000008+k);
   check(s.ringReadIndex==13&&PPC_LOAD_U32(0xA0011000)==13);
   __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
   check(s.colorControl==0&&s.currentBinIdMin==0);for(auto v:s.blendControl)check(v==0);
   puts("PASS contiguous render-state packet, existing fields, writeback and reset");return 0;
  }

  if(!strncmp(argv[1],"texturecoher",12)){
   c.r3.u32=0x10000;c.r4.u32=4;__imp__VdInitializeRingBuffer(c,base);
   uint32_t mode=0x02000000,addr=0x12000;
   if(!strcmp(argv[1],"texturecoherunsupported"))mode=0x04000000;
   if(!strcmp(argv[1],"texturecoherbad"))addr=0x20000;
   PPC_STORE_U32(0xA0012000,0x12345678);
   put(0,0x00020A2F);put(1,4096);put(2,addr);put(3,mode);
   put(4,0xC0043C00);put(5,3);put(6,0xA31);put(7,0);put(8,0x80000000);put(9,8);
   diagnosticMmioStore32(base,0x7FC80714,10,0);
   auto result=queryDiagnosticGpu(base);check(result.coherStatus==0&&result.coherBase==addr&&result.coherSize==4096);
   check(result.ringReadIndex==10&&PPC_LOAD_U32(0xA0011000)==10);
   check(PPC_LOAD_U32(0xA0012000)==0x12345678);
   puts("PASS texture coherence: validated range, completed wait, preserved data");return 0;
  }

  if(!strncmp(argv[1],"bin",3)){
   if(!strcmp(argv[1],"binbad")){put(0,0xC0016000);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);return 2;}
   if(!strcmp(argv[1],"binpredicate")){put(0,0xC0006001);put(1,1);diagnosticMmioStore32(base,0x7FC80714,2,0);return 2;}
   auto packet=[&](uint32_t op,uint32_t a,uint32_t b,bool full){
    c.r3.u32=0x10000;c.r4.u32=2;__imp__VdInitializeRingBuffer(c,base);
    put(0,0xC0000000|(full?0x10000:0)|(op<<8));put(1,a);put(2,b);
    diagnosticMmioStore32(base,0x7FC80714,full?3:2,0);
   };
   packet(0x50,0x12345678,0x9ABCDEF0,true);check(queryDiagnosticGpu(base).binMask==0x123456789ABCDEF0ull);
   packet(0x60,0x76543210,0,false);check(queryDiagnosticGpu(base).binMask==0x1234567876543210ull);
   packet(0x61,0xFEDCBA98,0,false);check(queryDiagnosticGpu(base).binMask==0xFEDCBA9876543210ull);
   packet(0x51,0x89ABCDEF,0x01234567,true);check(queryDiagnosticGpu(base).binSelect==0x89ABCDEF01234567ull);
   packet(0x62,0,0,false);check(queryDiagnosticGpu(base).binSelect==0x89ABCDEF00000000ull);
   packet(0x63,1,0,false);check(queryDiagnosticGpu(base).binSelect==0x100000000ull);
   check(queryDiagnosticGpu(base).binMask==0xFEDCBA9876543210ull);
   __imp__VdShutdownEngines(c,base);check(queryDiagnosticGpu(base).binMask==0&&queryDiagnosticGpu(base).binSelect==0);
   puts("PASS bin mask/select full and half updates, independent state and reset");return 0;
  }

  if(!strncmp(argv[1],"nooutput",8)){
   c.r3.u32=0x10000;c.r4.u32=6;__imp__VdInitializeRingBuffer(c,base);
   uint32_t cursor=0;auto emit=[&](uint32_t value){put(cursor++,value);};
   emit(0x2180);emit(0x1000000E);emit(0x2208);emit(4);
   if(!strcmp(argv[1],"nooutputcolor")){emit(0x2104);emit(15);}
   if(!strcmp(argv[1],"nooutputsample")){emit(0x2324);emit(1);}
   for(uint32_t stage=0;stage<2;++stage){
    emit(0xC0072B00);emit(stage);emit(6);emit(0x1001);emit(0x2200);emit(0);
    emit((50u<<26)|0xC000|(!strcmp(argv[1],"nooutputmem")?32:0));emit(0);emit((7u<<29)|(2u<<24));
   }
   emit(0xC0003600);emit(0x10081);emit(0xC0025800);emit(3);emit(0x12002);emit(0x12345678);
   diagnosticMmioStore32(base,0x7FC80714,cursor,0);
   auto result=queryDiagnosticGpu(base);
   check(result.noOutputDraws==1&&result.diagnosticBypassedDraws==0&&result.completedShaderEvents==1);
   check(PPC_LOAD_U32(0xA0012000)==0x12345678&&result.ringReadIndex==cursor);
   __imp__VdShutdownEngines(c,base);check(queryDiagnosticGpu(base).noOutputDraws==0);
   puts("PASS analyzed no-output draw and completion integration");return 0;
  }

  if(!strncmp(argv[1],"shaderevent",11)){
   const bool shortPacket=!strcmp(argv[1],"shadereventshort");
   uint32_t initiator=3,target=0x12002;
   if(!strcmp(argv[1],"shadereventcounter"))initiator=0x80000003;
   if(!strcmp(argv[1],"shadereventtype"))initiator=4;
   if(!strcmp(argv[1],"shadereventbad"))target=0x20000002;
   if(!strcmp(argv[1],"shadereventreadonly")){DWORD old;VirtualProtect(base+0xA0012000,4096,PAGE_READONLY,&old);}
   put(0,shortPacket?0xC0015800:0xC0025800);put(1,initiator);put(2,target);put(3,0x12345678);
   diagnosticMmioStore32(base,0x7FC80714,shortPacket?3:4,0);
   check(PPC_LOAD_U32(0xA0012000)==0x12345678);
   check(queryDiagnosticGpu(base).completedShaderEvents==1&&queryDiagnosticGpu(base).shaderEventInitiator==3);
   const uint8_t expected[4][4]={{0x78,0x56,0x34,0x12},{0x56,0x78,0x12,0x34},{0x12,0x34,0x56,0x78},{0x34,0x12,0x78,0x56}};
   for(uint32_t mode=0;mode<4;++mode){
    c.r3.u32=0x10000;c.r4.u32=2;__imp__VdInitializeRingBuffer(c,base);
    memset(base+0xA0012000,0xA5,12);put(0,0xC0025800);put(1,3);put(2,0x12004|mode);put(3,0x12345678);
    diagnosticMmioStore32(base,0x7FC80714,4,0);
    check(!memcmp(base+0xA0012004,expected[mode],4));
    check(base[0xA0012003]==0xA5&&base[0xA0012008]==0xA5);
   }
   __imp__VdShutdownEngines(c,base);check(queryDiagnosticGpu(base).completedShaderEvents==0);
   puts("PASS shader events: literal write, four byte orders, neighboring memory and reset");return 0;
  }

  if(!strcmp(argv[1],"zeroprobe")||!strcmp(argv[1],"zeroprobeevent")){
   put(0,0x00002208);put(1,4);put(2,0xC0003600);put(3,0x00010081);
   diagnosticMmioStore32(base,0x7FC80714,4,0);
   const auto probe=queryDiagnosticGpu(base);
   check(probe.diagnosticBypassedDraws==1&&probe.ringReadIndex==4);
   if(!strcmp(argv[1],"zeroprobeevent")){
    put(4,0xC0025800);put(5,3);put(6,0x12002);put(7,0x12345678);
    diagnosticMmioStore32(base,0x7FC80714,0,0);return 1;
   }

   __imp__VdShutdownEngines(c,base);check(queryDiagnosticGpu(base).diagnosticBypassedDraws==0);
   puts("PASS opt-in zero-surface investigation");return 0;
  }

  if(!strcmp(argv[1],"drawcapture")){put(0,0xC0003600);put(1,0x12340883);put(2,0x00002000);put(3,0x12345678);diagnosticMmioStore32(base,0x7FC80714,4,0);}
  if(!strcmp(argv[1],"destneighbor")){put(0,0x0001200F);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  if(!strcmp(argv[1],"aarange")){put(0,0x00012325);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  if(!strcmp(argv[1],"vizexecute")){put(0,0xC0002300);put(1,0);diagnosticMmioStore32(base,0x7FC80714,2,0);}
  if(!strcmp(argv[1],"vizrange")){put(0,0x00012294);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  if(!strcmp(argv[1],"programrange")){put(0,0x00012184);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
  if(!strcmp(argv[1],"shaderlength")||!strcmp(argv[1],"shaderstage")||!strcmp(argv[1],"shaderoffset")||!strcmp(argv[1],"shaderempty")){
   put(0,0xC0022B00);put(1,!strcmp(argv[1],"shaderstage")?2:0);
   put(2,!strcmp(argv[1],"shaderlength")?2:!strcmp(argv[1],"shaderoffset")?0x10001:!strcmp(argv[1],"shaderempty")?0:1);put(3,0);
   diagnosticMmioStore32(base,0x7FC80714,4,0);
  }
  if(!strcmp(argv[1],"invalidatelength")||!strcmp(argv[1],"invalidatepred")){
   put(0,!strcmp(argv[1],"invalidatelength")?0xC0013B00:0xC0003B01);put(1,0x7FFF);put(2,0);
   diagnosticMmioStore32(base,0x7FC80714,!strcmp(argv[1],"invalidatelength")?3:2,0);
  }
  if(!strcmp(argv[1],"windowrange")){put(0,0x00012082);put(1,1);put(2,2);diagnosticMmioStore32(base,0x7FC80714,3,0);}
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
 // Startup register group and sequential window coordinates, including wrap.
 for(auto index:{0xE42u,0xC85u,0xF01u}){
  c.r3.u64=0x10000;c.r4.u64=2;__imp__VdInitializeRingBuffer(c,base);
  put(0,index);put(1,index^0xABCDEF00);diagnosticMmioStore32(base,0x7FC80714,2,0);
 }
 s=queryDiagnosticGpu(base);check(s.unknownE42==(0xE42u^0xABCDEF00)&&s.paClEnhance==(0xC85u^0xABCDEF00)&&s.rbBcControl==(0xF01u^0xABCDEF00));
 put(2,0x00022080);put(3,0xFFFF8000);put(4,0x80010002);put(5,0x00030004);
 diagnosticMmioStore32(base,0x7FC80714,6,0);s=queryDiagnosticGpu(base);
 check(s.windowConfig[0]==0xFFFF8000&&s.windowConfig[1]==0x80010002&&s.windowConfig[2]==0x00030004);
 put(6,0x00012081);put(7,0);put(0,0);diagnosticMmioStore32(base,0x7FC80714,1,0);s=queryDiagnosticGpu(base);
 check(s.windowConfig[0]==0xFFFF8000&&s.windowConfig[1]==0&&s.windowConfig[2]==0);
 const auto beforeInvalidate=queryDiagnosticGpu(base);
 put(1,0xC0003B00);put(2,0x7FFF);diagnosticMmioStore32(base,0x7FC80714,3,0);s=queryDiagnosticGpu(base);
 check(s.invalidationMask==0x7FFF&&s.invalidationCount==1&&s.processedPackets==beforeInvalidate.processedPackets+1);
 check(s.windowConfig[0]==beforeInvalidate.windowConfig[0]&&s.coherStatus==beforeInvalidate.coherStatus&&s.scratch[0]==beforeInvalidate.scratch[0]&&PPC_LOAD_U32(0xA0012000)==0xABCDEF12);
 put(3,0xC0003B00);put(4,0);diagnosticMmioStore32(base,0x7FC80714,5,0);
 check(queryDiagnosticGpu(base).invalidationMask==0&&queryDiagnosticGpu(base).invalidationCount==2&&PPC_LOAD_U32(0xA0011000)==5);
 // Inline shader uploads preserve big-endian words across ring wrap.
 put(5,0xC0032B00);put(6,0);put(7,2);put(0,0x12345678);put(1,0xFEDCBA98);
 diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.shaderWords[0]==std::vector<uint32_t>({0x12345678,0xFEDCBA98})&&s.shaderWords[1].empty()&&s.shaderUploadCount==1);
 put(2,0xC0022B00);put(3,1);put(4,1);put(5,0xABCD1234);diagnosticMmioStore32(base,0x7FC80714,6,0);
 check(queryDiagnosticGpu(base).shaderWords[1]==std::vector<uint32_t>({0xABCD1234}));
 put(6,0xC0022B00);put(7,0);put(0,1);put(1,0x55555555);diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.shaderWords[0]==std::vector<uint32_t>({0x55555555})&&s.shaderWords[1]==std::vector<uint32_t>({0xABCD1234})&&s.shaderUploadCount==3);
 put(2,0xC0003B00);put(3,0x7FFF);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 put(4,0x00012180);put(5,0xFEDCBA98);put(6,0x87654321);
 diagnosticMmioStore32(base,0x7FC80714,7,0);s=queryDiagnosticGpu(base);
 check(s.sqProgramControl==0xFEDCBA98&&s.sqContextMisc==0x87654321);
 check(s.shaderWords[0]==std::vector<uint32_t>({0x55555555})&&s.shaderWords[1]==std::vector<uint32_t>({0xABCD1234}));
 put(7,0x0001A180);put(0,1);put(1,0);diagnosticMmioStore32(base,0x7FC80714,2,0);
 check(queryDiagnosticGpu(base).sqProgramControl==0&&queryDiagnosticGpu(base).sqContextMisc==0x87654321);
 put(2,0x00022100);put(3,0xFFFFFF);put(4,0);put(5,0xFFFFFFFF);
 diagnosticMmioStore32(base,0x7FC80714,6,0);s=queryDiagnosticGpu(base);
 check(s.vertexIndexConfig[0]==0xFFFFFF&&s.vertexIndexConfig[1]==0&&s.vertexIndexConfig[2]==0xFFFFFFFF);
 put(6,0x00002102);put(7,0);diagnosticMmioStore32(base,0x7FC80714,0,0);
 check(queryDiagnosticGpu(base).vertexIndexConfig[2]==0&&queryDiagnosticGpu(base).vertexIndexConfig[0]==0xFFFFFF);
 check(queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]&&queryDiagnosticGpu(base).sqContextMisc==s.sqContextMisc);
 put(0,0x00002293);put(1,0xFFFFFFFF);diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.vizQueryConfig==0xFFFFFFFF&&s.vertexIndexConfig[0]==0xFFFFFF);
 put(2,0x00002293);put(3,0);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(queryDiagnosticGpu(base).vizQueryConfig==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 put(4,0x00022204);put(5,0x12345678);put(6,0xFFFFFFFF);put(7,0x80000001);
 diagnosticMmioStore32(base,0x7FC80714,0,0);s=queryDiagnosticGpu(base);
 check(s.clipTransformConfig[0]==0x12345678&&s.clipTransformConfig[1]==0xFFFFFFFF&&s.clipTransformConfig[2]==0x80000001);
 put(0,0x00002205);put(1,0);diagnosticMmioStore32(base,0x7FC80714,2,0);
 check(queryDiagnosticGpu(base).clipTransformConfig[1]==0&&queryDiagnosticGpu(base).clipTransformConfig[0]==0x12345678);
 check(queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]&&queryDiagnosticGpu(base).vizQueryConfig==s.vizQueryConfig);
 put(2,0x00002312);put(3,0xA5A5FFFF);diagnosticMmioStore32(base,0x7FC80714,4,0);s=queryDiagnosticGpu(base);
 check(s.aaMask==0xA5A5FFFF&&s.clipTransformConfig[0]==0x12345678);
 put(4,0x0001A312);put(5,1);put(6,0);diagnosticMmioStore32(base,0x7FC80714,7,0);
 check(queryDiagnosticGpu(base).aaMask==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 for(uint32_t slot=0;slot<8;++slot){
  c.r3.u64=0x10000;c.r4.u64=2;__imp__VdInitializeRingBuffer(c,base);
  put(0,0x2006+slot);put(1,0x1000+slot*0x100);diagnosticMmioStore32(base,0x7FC80714,2,0);
 }
 s=queryDiagnosticGpu(base);for(uint32_t slot=0;slot<8;++slot)check(s.coherDestBase[slot]==0x1000+slot*0x100);
 check(s.coherStatus==0&&PPC_LOAD_U32(0xA0012000)==0xABCDEF12);
 put(2,0x0000200D);put(3,0);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(queryDiagnosticGpu(base).coherDestBase[7]==0&&queryDiagnosticGpu(base).coherDestBase[0]==0x1000);
 put(4,0x00002200);put(5,0xFEDCBA98);diagnosticMmioStore32(base,0x7FC80714,6,0);s=queryDiagnosticGpu(base);
 check(s.depthControl==0xFEDCBA98&&s.coherStatus==0&&s.coherDestBase[0]==0x1000);
 put(6,0x00002200);put(7,0);diagnosticMmioStore32(base,0x7FC80714,0,0);
 check(queryDiagnosticGpu(base).depthControl==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 put(0,0x00002203);put(1,0xFEDCBA98);diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.hizControl==0xFEDCBA98&&s.depthControl==0&&s.coherStatus==0);
 put(2,0x00002203);put(3,0);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(queryDiagnosticGpu(base).hizControl==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 put(4,0x00002208);put(5,0xFEDCBA98);diagnosticMmioStore32(base,0x7FC80714,6,0);s=queryDiagnosticGpu(base);
 check(s.modeControl==0xFEDCBA98&&s.hizControl==0&&s.coherStatus==0);
 put(6,0x00002208);put(7,0);diagnosticMmioStore32(base,0x7FC80714,0,0);
 check(queryDiagnosticGpu(base).modeControl==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 put(0,0x00002104);put(1,0x1234ABCD);diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.colorMask==0x1234ABCD&&s.modeControl==0&&s.coherStatus==0);
 put(2,0x00002104);put(3,0);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(queryDiagnosticGpu(base).colorMask==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 put(4,0x00002280);put(5,0xABCD1234);diagnosticMmioStore32(base,0x7FC80714,6,0);s=queryDiagnosticGpu(base);
 check(s.pointSize==0xABCD1234&&s.colorMask==0&&s.coherStatus==0);
 put(6,0x00002280);put(7,0);diagnosticMmioStore32(base,0x7FC80714,0,0);
 check(queryDiagnosticGpu(base).pointSize==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 put(0,0x00002302);put(1,0xA5A51234);diagnosticMmioStore32(base,0x7FC80714,2,0);s=queryDiagnosticGpu(base);
 check(s.vertexControl==0xA5A51234&&s.pointSize==0&&s.coherStatus==0);
 put(2,0x00002302);put(3,0);diagnosticMmioStore32(base,0x7FC80714,4,0);
 check(queryDiagnosticGpu(base).vertexControl==0&&queryDiagnosticGpu(base).shaderWords[0]==s.shaderWords[0]);
 check(s.coverageWrittenMask==0&&s.surfaceInfo==0&&s.screenScissor[0]==0&&s.screenScissor[1]==0);
 put(4,0x00002000);put(5,0x12345678);diagnosticMmioStore32(base,0x7FC80714,6,0);
 s=queryDiagnosticGpu(base);check(s.surfaceInfo==0x12345678&&s.coverageWrittenMask==1);
 put(6,0x0001200E);put(7,0x00010002);put(0,0x00100020);diagnosticMmioStore32(base,0x7FC80714,1,0);
 s=queryDiagnosticGpu(base);check(s.screenScissor[0]==0x00010002&&s.screenScissor[1]==0x00100020&&s.coverageWrittenMask==7);
 put(1,0x00002000);put(2,0);diagnosticMmioStore32(base,0x7FC80714,3,0);
 s=queryDiagnosticGpu(base);check(s.surfaceInfo==0&&s.coverageWrittenMask==7);
 __imp__VdShutdownEngines(c,base);s=queryDiagnosticGpu(base);
 check(s.coverageWrittenMask==0&&s.surfaceInfo==0&&s.screenScissor[0]==0&&s.screenScissor[1]==0);
 check(s.vertexControl==0);
 check(s.pointSize==0);
 check(s.colorMask==0);
 check(s.modeControl==0);
 check(s.hizControl==0);
 check(s.depthControl==0);
 check(s.coherDestBase[0]==0&&s.coherDestBase[7]==0);
 check(s.aaMask==0);
 check(s.clipTransformConfig[0]==0&&s.clipTransformConfig[1]==0&&s.clipTransformConfig[2]==0);
 check(s.vizQueryConfig==0);
 check(s.vertexIndexConfig[0]==0&&s.vertexIndexConfig[1]==0&&s.vertexIndexConfig[2]==0);
 check(s.sqProgramControl==0&&s.sqContextMisc==0);
 check(s.shaderWords[0].empty()&&s.shaderWords[1].empty()&&s.shaderUploadCount==0);
 check(s.invalidationCount==0&&s.invalidationMask==0);
 check(s.unknownE42==0&&s.paClEnhance==0&&s.rbBcControl==0&&s.windowConfig[0]==0);
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
