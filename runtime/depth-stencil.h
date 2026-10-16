#pragma once
#include "edram-storage.h"

namespace ds1 {
// Limited D24S8 path: both comparisons ALWAYS, front state only, KEEP on
// failures, REPLACE on depth pass. Caller supplies a covered storage sample
// and already quantized depth. No rasterization or float conversion here.
inline bool writeAlwaysD24S8(EdramStorage& memory,uint32_t base,uint32_t pitch,
 uint32_t x,uint32_t y,uint32_t control,uint32_t stencilRefMask,
 uint32_t depth24){
 // Permit depth-write enable to vary, reject every other unhandled state bit.
 if((control&~4u)!=0x8773u||depth24>0xffffffu)return false;
 const uint32_t stencilMask=(stencilRefMask>>16)&255;
 const uint32_t mask=((control&4)?0xffffff00u:0)|stencilMask;
 // Xenos D24S8 stores stencil in bits 0:7, depth in bits 8:31.
 // Reference: Xenia dxbc_shader_translator_om.cc depth/stencil merge.
 return memory.write(base,pitch,x,y,true,(depth24<<8)|(stencilRefMask&255),mask);
}
}
