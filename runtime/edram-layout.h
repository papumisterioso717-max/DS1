#pragma once
#include <cstdint>

// Logical 32-bit EDRAM storage, tile-major with row-major sample slots.
// Reference: Xenia gpu/xenos.h EDRAM layout documentation.
// Coordinates are storage sample coordinates, NOT rasterizer sample IDs.
// No sample coverage, format conversion, 64-bit formats or guest writes here.
namespace ds1 {
constexpr uint32_t edramTileWords=80*16;
constexpr uint32_t edramWords=2048*edramTileWords;

inline bool edramPitch32(uint32_t pixels,uint32_t samples,uint32_t& tiles){
 if(!pixels||pixels>16383||(samples!=1&&samples!=2&&samples!=4))return false;
 tiles=(pixels*(samples==4?2:1)+79)/80;
 return true;
}

inline bool edramWord32(uint32_t baseTile,uint32_t pitchTiles,
                       uint32_t sampleX,uint32_t sampleY,bool depth,
                       uint32_t& address){
 if(baseTile>=2048||!pitchTiles||pitchTiles>410||sampleX>=pitchTiles*80)return false;
 const uint64_t tile=(uint64_t(baseTile)+uint64_t(sampleY/16)*pitchTiles+sampleX/80)%2048;
 uint32_t x=sampleX%80;
 if(depth)x=x<40?x+40:x-40;
 address=uint32_t(tile)*edramTileWords+(sampleY%16)*80+x;
 return true;
}
}
