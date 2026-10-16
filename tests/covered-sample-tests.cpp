#include "../runtime/covered-sample-writer.h"
#include <cstdio>
#include <cstdlib>
unsigned checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL %u\n",checks);exit(1);}}
int main(){
 ds1::EdramStorage m;uint32_t v=0;
 std::vector<ds1::StorageSample> samples={{0,0},{1,0},{0,1},{1,1}};
 check(ds1::writeCoveredSamples(m,samples,16,720,0,0,~0u,0xffffff,0x8777,0xffff00));
 for(auto s:samples){
  check(m.read(720,16,s.x,s.y,false,~0u,v)&&v==0);
  check(m.read(0,16,s.x,s.y,true,~0u,v)&&v==0xffffff00);
 }
 check(!m.read(720,16,2,0,false,~0u,v));
 check(!m.read(0,16,2,0,true,~0u,v));
 // A later invalid coordinate rejects the complete batch, including first sample.
 samples.push_back({1280,0});
 check(!ds1::writeCoveredSamples(m,samples,16,720,0,~0u,~0u,0,0x8777,0xffffab));
 check(m.read(720,16,0,0,false,~0u,v)&&v==0);
 check(m.read(0,16,0,0,true,~0u,v)&&v==0xffffff00);
 samples.pop_back();
 check(!ds1::writeCoveredSamples(m,samples,16,2048,0,~0u,~0u,0,0x8777,0xffffab));
 check(m.read(0,16,0,0,true,~0u,v)&&v==0xffffff00);
 // Disabled depth writes, partial stencil and partial color preserve other bits.
 check(ds1::writeCoveredSamples(m,samples,16,720,0,0x11223344,0xff00,0,0x8773,0x0f000a));
 check(m.read(720,16,0,0,false,~0u,v)&&v==0x3300);
 check(m.read(0,16,0,0,true,~0u,v)&&v==0xffffff0a);
 check(!ds1::writeCoveredSamples(m,samples,16,720,0,0,~0u,0,0,0));
 check(m.read(0,16,0,0,true,~0u,v)&&v==0xffffff0a);
 printf("PASS %u covered-sample integration checks\n",checks);
}
