#include "../runtime/edram-storage.h"
#include <cstdio>
#include <cstdlib>
static unsigned checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL %u\n",checks);exit(1);}}
int main(){
 ds1::EdramStorage memory;
 uint32_t value=0xdeadbeef;
 check(!memory.read(0,16,0,0,false,0xffffffff,value)&&value==0xdeadbeef);
 check(memory.write(0,16,0,0,false,0x12345678,0x0000ffff));
 check(memory.read(0,16,0,0,false,0xffff,value)&&value==0x5678);
 check(!memory.read(0,16,0,0,false,0xffffffff,value)&&value==0x5678);
 check(memory.write(0,16,0,0,false,0xabcdef00,0xffff0000));
 check(memory.read(0,16,0,0,false,0xffffffff,value)&&value==0xabcd5678);
 check(memory.write(0,16,0,0,false,0xff,0));
 check(memory.read(0,16,0,0,false,0xffffffff,value)&&value==0xabcd5678);
 // Color and depth refer to the same backing storage, with half-tile exchange.
 check(memory.read(0,16,40,0,true,0xffffffff,value)&&value==0xabcd5678);
 check(!memory.read(0,16,0,0,true,0xffffffff,value));
 check(!memory.write(2048,16,0,0,false,0,0xffffffff));
 check(!memory.write(0,16,1280,0,false,0,0xffffffff));
 check(memory.read(0,16,0,0,false,0xffffffff,value)&&value==0xabcd5678);
 check(memory.write(2047,1,0,16,false,0x76543210,0xffffffff));
 check(memory.read(0,1,0,0,false,0xffffffff,value)&&value==0x76543210);
 // Synthetic full-plane storage exercise, not a captured draw or rasterization.
 for(unsigned y=0;y<720;++y)for(unsigned x=0;x<1280;++x){
  check(memory.write(0,16,x,y,true,0xa1b2c3d4,0xffffffff));
  check(memory.write(720,16,x,y,false,0x11223344,0xffffffff));
 }
 for(unsigned y=0;y<720;++y)for(unsigned x=0;x<1280;++x){
  check(memory.read(0,16,x,y,true,0xffffffff,value)&&value==0xa1b2c3d4);
  check(memory.read(720,16,x,y,false,0xffffffff,value)&&value==0x11223344);
 }
 check(!memory.read(1440,16,0,0,false,0xffffffff,value));
 ds1::EdramStorage independent;
 check(!independent.read(0,16,0,0,false,0xffffffff,value));
 printf("PASS %u EDRAM storage checks\n",checks);
}
