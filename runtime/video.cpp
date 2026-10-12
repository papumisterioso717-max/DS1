#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace {
// Diagnostic-only virtual display, matching Xenia's baseline video-mode query.
// This profile describes no physical PC connector and does not create a renderer.
constexpr uint32_t mode[12]={1280,720,0,1,1,0x42700000,1,0x4A,1,0,0,0};
void writeMode(uint8_t*base,uint32_t a){
 if(!a||uint64_t(a)+sizeof(mode)>0x100000000ull){fprintf(stderr,"INVALID_VIDEO_MODE_BUFFER\n");std::exit(22);}
 for(uint32_t i=0;i<sizeof(mode);++i){
  MEMORY_BASIC_INFORMATION info{};
  if(!VirtualQuery(base+a+i,&info,sizeof(info))||info.State!=MEM_COMMIT||
     (info.Protect&PAGE_GUARD)||!(info.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))){
   fprintf(stderr,"INVALID_VIDEO_MODE_BUFFER address=0x%08X\n",a);std::exit(22);
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
 else {fprintf(stderr,"PENDING_XCONFIG category=%u setting=%u guest_lr=0x%08X\n",category,setting,uint32_t(ctx.lr));std::exit(20);}
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
