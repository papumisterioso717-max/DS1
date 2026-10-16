#pragma once
#include "gpu.h"
// Conservative classifier, not a shader interpreter. Layout: Xenia ucode.h.
// Only unconditional linear MAX / RETAIN_PREV programs with ordinary exports.
inline bool diagnosticShaderHasOnlyOrdinaryExports(const std::vector<uint32_t>& code,bool pixel){
 if(code.empty()||code.size()%3||code.size()>768)return false;
 const uint32_t groups=uint32_t(code.size()/3);
 uint32_t firstInstruction=groups,aluCount=0;bool anyExport=false;
 for(uint32_t group=0;group<firstInstruction;++group){
  const uint32_t a=code[group*3],b=code[group*3+1],c=code[group*3+2];
  const uint32_t low[2]={a,(b>>16)|((c&65535)<<16)},high[2]={b&65535,c>>16};
  for(unsigned half=0;half<2;++half){
   const uint32_t lo=low[half],hi=high[half],op=hi>>12;
   if(op==0){if(lo||hi)return false;continue;}
   if(op==12){if(lo|| (hi!=0xC200&&hi!=0xC400))return false;continue;}
   if((op!=1&&op!=2)||(hi&4095)!=0x200||(lo&~0x7FFFu))return false;
   const uint32_t start=lo&4095,count=(lo>>12)&7;
   if(!count||count>6||start<=group||start+count>groups)return false;
   if(start<firstInstruction)firstInstruction=start;
   for(uint32_t slot=start;slot<start+count;++slot){
    const uint32_t x=code[slot*3],y=code[slot*3+1],z=code[slot*3+2];
    // No predicates, addressing modifiers, clamps, temporary/scalar writes,
    // fetch, kill, depth export or memory export in the accepted subset.
    if((x>>26)!=50||(x&0x03003FC0u)||y||((z>>24)&31)!=2||
       (z&0xE0000000u)!=0xE0000000u||(z&0x00C0C0C0u))return false;
    const uint32_t vm=(x>>16)&15,sm=(x>>20)&15,dest=x&63;
    if(x&0x8000){
     if((sm&~vm)||(pixel?dest>3:!(dest<16||dest==62)))return false;
     anyExport=true;
    }else if(vm||sm||(x&0x4000))return false;
    ++aluCount;
   }
   if(op==2)return aluCount&&anyExport&&firstInstruction>=group+1;
  }
 }
 return false;
}
inline bool diagnosticNoOutputDraw(const DiagnosticGpuState& s,uint32_t draw,uint32_t count){
 // Zero-pitch rasterization policy matches Xenia draw_util / IssueDraw.
 // Unlike a blind zero-pitch bypass, require both shaders to be memory-free.
 return count==1&&draw==0x00010081&&s.surfaceInfo==0&&s.modeControl==4&&
  s.colorMask==0&&s.depthControl==0&&s.hizControl==0&&s.vizQueryConfig==0&&
  s.sqProgramControl==0x1000000E&&s.sqContextMisc==0&&s.rasterCopyConfig[0x24]==0&&
  diagnosticShaderHasOnlyOrdinaryExports(s.shaderWords[0],false)&&
  diagnosticShaderHasOnlyOrdinaryExports(s.shaderWords[1],true);
}
