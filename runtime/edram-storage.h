#pragma once
#include "edram-layout.h"
#include <vector>

namespace ds1 {
// Host-side logical EDRAM. Unknown initial bits remain unknown until written.
// Raw packed words only: format conversion and coverage belong to the caller.
class EdramStorage {
 std::vector<uint32_t> values_,known_;
public:
 EdramStorage():values_(edramWords,0),known_(edramWords,0){}
 bool write(uint32_t base,uint32_t pitch,uint32_t x,uint32_t y,bool depth,
            uint32_t packed,uint32_t mask){
  uint32_t address;
  if(!edramWord32(base,pitch,x,y,depth,address))return false;
  values_[address]=(values_[address]&~mask)|(packed&mask);
  known_[address]|=mask;
  return true;
 }
 bool read(uint32_t base,uint32_t pitch,uint32_t x,uint32_t y,bool depth,
           uint32_t requiredMask,uint32_t& packed)const{
  uint32_t address;
  if(!edramWord32(base,pitch,x,y,depth,address)||
     (known_[address]&requiredMask)!=requiredMask)return false;
  packed=values_[address]&requiredMask;
  return true;
 }
};
}
