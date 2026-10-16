#pragma once
#include "depth-stencil.h"
#include <vector>

namespace ds1 {
struct StorageSample {uint32_t x,y;};
// Applies already established coverage, never infers it from a bounding box.
// Packed color and bit masks are caller inputs; no blending or format conversion.
inline bool writeCoveredSamples(EdramStorage& memory,
 const std::vector<StorageSample>& samples,uint32_t pitch,
 uint32_t colorBase,uint32_t depthBase,uint32_t color,uint32_t colorMask,
 uint32_t depth24,uint32_t control,uint32_t stencilRefMask){
 if(samples.size()>edramWords||(control&~4u)!=0x8773u||depth24>0xffffffu)return false;
 // Validate the entire batch before modifying either plane.
 for(const auto& s:samples){
  uint32_t address;
  if(!edramWord32(colorBase,pitch,s.x,s.y,false,address)||
     !edramWord32(depthBase,pitch,s.x,s.y,true,address))return false;
 }
 for(const auto& s:samples){
  writeAlwaysD24S8(memory,depthBase,pitch,s.x,s.y,control,stencilRefMask,depth24);
  memory.write(colorBase,pitch,s.x,s.y,false,color,colorMask);
 }
 return true;
}
}
