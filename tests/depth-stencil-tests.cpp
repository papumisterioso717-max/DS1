#include "../runtime/depth-stencil.h"
#include <cstdio>
#include <cstdlib>
unsigned checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL %u\n",checks);exit(1);}}
int main(){
 ds1::EdramStorage m;uint32_t v=123;
 // Captured state initializes an unknown sample without needing old contents.
 check(ds1::writeAlwaysD24S8(m,0,16,0,0,0x8777,0xffff00,0xffffff));
 check(m.read(0,16,0,0,true,0xffffffff,v)&&v==0xffffff00);
 // Partial stencil replacement preserves depth and untouched stencil bits.
 check(m.write(0,16,1,0,true,0x123456a5,0xffffffff));
 check(ds1::writeAlwaysD24S8(m,0,16,1,0,0x8773,0x0f0003,0));
 check(m.read(0,16,1,0,true,0xffffffff,v)&&v==0x123456a3);
 // A masked write must not invent values for unknown bits.
 check(ds1::writeAlwaysD24S8(m,0,16,2,0,0x8773,0x0f0003,0));
 check(m.read(0,16,2,0,true,15,v)&&v==3);
 check(!m.read(0,16,2,0,true,0xffffffff,v));
 // All single-bit state deviations except depth-write must fail atomically.
 for(unsigned bit=0;bit<32;++bit){
  if(bit==2)continue;
  check(!ds1::writeAlwaysD24S8(m,0,16,0,0,0x8777^(1u<<bit),0,0));
  check(m.read(0,16,0,0,true,0xffffffff,v)&&v==0xffffff00);
 }
 check(!ds1::writeAlwaysD24S8(m,0,16,0,0,0x8777,0,0x1000000));
 check(!ds1::writeAlwaysD24S8(m,0,16,1280,0,0x8777,0,0));
 check(m.read(0,16,0,0,true,0xffffffff,v)&&v==0xffffff00);
 printf("PASS %u depth/stencil checks\n",checks);
}
