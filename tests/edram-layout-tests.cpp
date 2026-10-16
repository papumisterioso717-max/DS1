#include "../runtime/edram-layout.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
static unsigned checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL %u\n",checks);exit(1);}}
int main(){
 uint32_t pitch=777,address=777;
 check(ds1::edramPitch32(640,4,pitch)&&pitch==16);
 check(ds1::edramPitch32(640,2,pitch)&&pitch==8);
 check(ds1::edramPitch32(81,1,pitch)&&pitch==2);
 check(!ds1::edramPitch32(0,1,pitch)&&pitch==2);
 check(!ds1::edramPitch32(640,3,pitch)&&pitch==2);
 check(!ds1::edramPitch32(16384,4,pitch));
 check(ds1::edramWord32(0,1,0,0,false,address)&&address==0);
 check(ds1::edramWord32(0,1,0,0,true,address)&&address==40);
 check(ds1::edramWord32(0,1,40,0,true,address)&&address==0);
 check(ds1::edramWord32(2047,1,0,16,false,address)&&address==0);
 check(ds1::edramWord32(0,1,79,15,false,address)&&address==1279);
 address=777;
 check(!ds1::edramWord32(2048,1,0,0,false,address)&&address==777);
 check(!ds1::edramWord32(0,1,80,0,false,address)&&address==777);
 check(!ds1::edramWord32(0,0,0,0,false,address)&&address==777);
 // Every sample slot in the candidate 640x360 4x footprint is unique.
 // Color and depth occupy adjacent, disjoint 720-tile regions.
 std::vector<unsigned char> used(ds1::edramWords,0);
 for(unsigned plane=0;plane<2;++plane)
  for(unsigned y=0;y<720;++y)for(unsigned x=0;x<1280;++x){
   check(ds1::edramWord32(plane?720:0,16,x,y,!plane,address));
   check(address<used.size()&&!used[address]);used[address]=1;
  }
 unsigned count=0;for(auto v:used)count+=v;
 check(count==640*360*4*2);
 // Large coordinates must wrap without a 32-bit intermediate overflow.
 check(ds1::edramWord32(2047,410,0,0xffffffff,true,address));
 check(address<ds1::edramWords);
 printf("PASS %u EDRAM layout checks\n",checks);
}
