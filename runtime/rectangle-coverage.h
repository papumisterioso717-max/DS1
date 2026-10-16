#pragma once
#include "covered-sample-writer.h"
#include <array>
#include <algorithm>

namespace ds1 {
struct Rect16 {int32_t left,top,right,bottom;};
struct PixelScissor {uint32_t left,top,right,bottom;};
struct SampleOffset16 {int32_t x,y;};
// Axis-aligned rectangle only, integer pixel centers, 1/16-pixel input units.
// Pattern entries correspond to storage slots TL, BL, TR, BR, respectively.
// The caller must validate guest geometry/state and choose a justified pattern.
inline bool rectangleCoverage4x(Rect16 rect,PixelScissor clip,
 const std::array<SampleOffset16,4>& pattern,std::vector<StorageSample>& output){
 if(rect.left>rect.right||rect.top>rect.bottom||clip.left>clip.right||
    clip.top>clip.bottom||clip.right>16383||clip.bottom>16383)return false;
 for(auto p:pattern)if(p.x<=-8||p.x>=8||p.y<=-8||p.y>=8)return false;
 // Intersect a conservative pixel bound with the scissor before budgeting.
 // Floor division is explicit for negative fixed-point coordinates.
 auto floor16=[](int64_t v){return v>=0?v/16:-((-v+15)/16);};
 auto clampPixel=[](int64_t v){return uint32_t(std::max<int64_t>(0,std::min<int64_t>(16383,v)));};
 clip.left=std::max(clip.left,clampPixel(floor16(rect.left)-1));
 clip.top=std::max(clip.top,clampPixel(floor16(rect.top)-1));
 clip.right=std::min(clip.right,clampPixel(floor16(rect.right)+2));
 clip.bottom=std::min(clip.bottom,clampPixel(floor16(rect.bottom)+2));
 if(clip.left>=clip.right||clip.top>=clip.bottom){output.clear();return true;}
 // Bound work and output before allocation; no partial output on rejection.
 const uint64_t candidates=uint64_t(clip.right-clip.left)*(clip.bottom-clip.top)*4;
 if(candidates>edramWords)return false;
 std::vector<StorageSample> result;
 result.reserve(static_cast<size_t>(candidates));
 for(uint32_t y=clip.top;y<clip.bottom;++y)
  for(uint32_t x=clip.left;x<clip.right;++x)
   for(uint32_t s=0;s<4;++s){
    const int64_t sx=int64_t(x)*16+pattern[s].x,sy=int64_t(y)*16+pattern[s].y;
    // Top/left inclusive, bottom/right exclusive.
    if(sx>=rect.left&&sx<rect.right&&sy>=rect.top&&sy<rect.bottom)
     result.push_back({x*2+(s>>1),y*2+(s&1)});
   }
 output.swap(result);
 return true;
}
}
