#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <algorithm>
namespace {
// Diagnostic-only virtual display, matching Xenia's baseline video-mode query.
// This profile describes no physical PC connector and does not create a renderer.
constexpr uint32_t mode[12]={1280,720,0,1,1,0x42700000,1,0x4A,1,0,0,0};
void writeMode(uint8_t*base,uint32_t a){
 if(!a||uint64_t(a)+sizeof(mode)>0x100000000ull){fprintf(stderr,"INVALID_VIDEO_MODE_BUFFER\n");std::_Exit(22);}
 for(uint32_t i=0;i<sizeof(mode);++i){
  MEMORY_BASIC_INFORMATION info{};
  if(!VirtualQuery(base+a+i,&info,sizeof(info))||info.State!=MEM_COMMIT||
     (info.Protect&PAGE_GUARD)||!(info.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))){
   fprintf(stderr,"INVALID_VIDEO_MODE_BUFFER address=0x%08X\n",a);std::_Exit(22);
  }
 }
 for(uint32_t i=0;i<12;++i){const uint32_t v=mode[i],p=a+i*4;base[p]=v>>24;base[p+1]=v>>16;base[p+2]=v>>8;base[p+3]=v;}
}
}
PPC_FUNC(__imp__XGetAVPack){
 // Compatibility value used by hedge-dev/UnleashedRecomp, not a verified
 // identification of a physical Xbox cable. Keep this limitation visible.
 fprintf(stderr,"VIDEO_DIAGNOSTIC_PROFILE av_pack=0 mode=1280x720@60 renderer=pending\n");ctx.r3.u64=0;
}
PPC_FUNC(__imp__XGetVideoMode){writeMode(base,ctx.r3.u32);}
PPC_FUNC(__imp__VdQueryVideoMode){writeMode(base,ctx.r3.u32);}
PPC_FUNC(__imp__VdQueryVideoFlags){ctx.r3.u64=3;}

PPC_FUNC(__imp__ExGetXConfigSetting){
 const uint16_t category=ctx.r3.u16,setting=ctx.r4.u16,capacity=ctx.r6.u16;
 const uint32_t buffer=ctx.r5.u32,required=ctx.r7.u32;
 uint32_t value;
 // Virtual diagnostic console configuration, consistent with Xenia's defaults.
 if(category==2&&setting==2)value=0x1000; // AV region: NTSC North America
 else if(category==3&&setting==10)value=0x40000; // widescreen video preference
 else {fprintf(stderr,"PENDING_XCONFIG category=%u setting=%u guest_lr=0x%08X\n",category,setting,uint32_t(ctx.lr));std::_Exit(20);}
 auto writable=[&](uint32_t a,uint32_t n){
  if(!a||uint64_t(a)+n>0x100000000ull)return false;
  for(uint32_t i=0;i<n;++i){MEMORY_BASIC_INFORMATION m{};
   if(!VirtualQuery(base+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&PAGE_GUARD)||
      !(m.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE)))return false;}
  return true;
 };
 if(required&&!writable(required,2)){ctx.r3.u64=0xC000000D;return;}
 uint32_t status=0;
 if(buffer&&capacity<4)status=0xC0000023;
 else if(!buffer&&capacity)status=0xC00000F1;
 else if(buffer&&!writable(buffer,4))status=0xC000000D;
 if(!status&&buffer){base[buffer]=value>>24;base[buffer+1]=value>>16;base[buffer+2]=value>>8;base[buffer+3]=value;}
 if(required){base[required]=0;base[required+1]=status?0:4;}
 ctx.r3.u64=status;
}

PPC_FUNC(__imp__VdGetCurrentDisplayGamma){
 const uint32_t type=ctx.r3.u32,power=ctx.r4.u32;
 // Validate both destinations before modifying either one.
 for(uint32_t a:{type,power}){
  if(!a||uint64_t(a)+4>0x100000000ull){fprintf(stderr,"INVALID_GAMMA_BUFFER\n");std::_Exit(22);}
  for(uint32_t i=0;i<4;++i){MEMORY_BASIC_INFORMATION m{};
   if(!VirtualQuery(base+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&PAGE_GUARD)||!(m.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))){fprintf(stderr,"INVALID_GAMMA_BUFFER\n");std::_Exit(22);}
  }
 }
 auto put=[&](uint32_t a,uint32_t v){base[a]=v>>24;base[a+1]=v>>16;base[a+2]=v>>8;base[a+3]=v;};
 // Diagnostic display defaults from Xenia, not the host monitor calibration.
 const float gamma=2.22222233f;uint32_t bits;memcpy(&bits,&gamma,4);
 put(type,2);put(power,bits);
 fprintf(stderr,"VIDEO_GAMMA type=2 power=%.8f profile=diagnostic\n",double(gamma));
}

// Diagnostic compatibility with Xenia's zero-return retraining stubs.
// Only accept the title's initial empty request; no EDRAM or commands generated.
PPC_FUNC(__imp__VdRetrainEDRAMWorker){
 if(ctx.r3.u32){fprintf(stderr,"PENDING_EDRAM_WORKER_MODE argument=%08X\n",ctx.r3.u32);std::_Exit(20);}
 fprintf(stderr,"EDRAM_WORKER compatibility=no-op hardware-training=absent\n");ctx.r3.u64=0;
}
PPC_FUNC(__imp__VdRetrainEDRAM){
 auto readable=[&](uint32_t a,uint32_t n){
  if(!a||uint64_t(a)+n>0x100000000ull)return false;
  for(uint64_t pos=a;pos<uint64_t(a)+n;){MEMORY_BASIC_INFORMATION m{};
   if(!VirtualQuery(base+pos,&m,sizeof(m))||m.State!=MEM_COMMIT||
     (m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
   pos=std::min(uint64_t(a)+n,uint64_t(static_cast<uint8_t*>(m.BaseAddress)-base)+m.RegionSize);
  }return true;
 };
 auto get=[&](uint32_t a){return(uint32_t(base[a])<<24)|(uint32_t(base[a+1])<<16)|(uint32_t(base[a+2])<<8)|base[a+3];};
 const uint32_t cursor=ctx.r4.u32,state=ctx.r6.u32;
 if(ctx.r5.u32!=4096||ctx.r8.u32!=2048||!readable(cursor,4)||!readable(state,4)||
    !readable(ctx.r7.u32,2048)||get(cursor)||get(state)){
  fprintf(stderr,"PENDING_EDRAM_REQUEST guest_lr=0x%08X\n",uint32_t(ctx.lr));std::_Exit(20);
 }
 fprintf(stderr,"EDRAM_RETRAIN compatibility=empty-request result=0 commands=0 hardware-training=absent\n");
 ctx.r3.u64=0;
}
