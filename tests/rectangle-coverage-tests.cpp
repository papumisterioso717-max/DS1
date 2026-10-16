#include "../runtime/rectangle-coverage.h"
#include <cstdio>
#include <cstdlib>
unsigned checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL %u\n",checks);exit(1);}}
int main(){
 // Explicit synthetic pattern, ordered by logical storage slot.
 std::array<ds1::SampleOffset16,4> pattern={{{-2,-6},{-6,2},{6,-2},{2,6}}};
 std::vector<ds1::StorageSample> samples;
 check(ds1::rectangleCoverage4x({-8,-8,10232,5752},{0,0,640,360},pattern,samples));
 check(samples.size()==640*360*4);
 ds1::EdramStorage m;uint32_t value;
 check(ds1::writeCoveredSamples(m,samples,16,720,0,0,~0u,0xffffff,0x8777,0xffff00));
 check(m.read(720,16,1279,719,false,~0u,value)&&value==0);
 check(m.read(0,16,1279,719,true,~0u,value)&&value==0xffffff00);
 check(!m.read(720,16,0,720,false,~0u,value));
 // Exact sample-edge equality must include left/top and exclude right/bottom.
 check(ds1::rectangleCoverage4x({-2,-6,0,0},{0,0,1,1},pattern,samples));
 check(samples.size()==1&&samples[0].x==0&&samples[0].y==0);
 check(ds1::rectangleCoverage4x({-8,-8,-2,8},{0,0,1,1},pattern,samples));
 check(samples.size()==1&&samples[0].x==0&&samples[0].y==1);
 check(ds1::rectangleCoverage4x({-8,-8,24,24},{1,1,2,2},pattern,samples));
 check(samples.size()==4);
 for(auto s:samples)check(s.x>=2&&s.y>=2);
 check(!ds1::rectangleCoverage4x({1,0,0,1},{0,0,1,1},pattern,samples)&&samples.size()==4);
 check(!ds1::rectangleCoverage4x({0,0,262128,262128},{0,0,16383,16383},pattern,samples)&&samples.size()==4);
 pattern[0].x=8;
 check(!ds1::rectangleCoverage4x({0,0,1,1},{0,0,1,1},pattern,samples)&&samples.size()==4);
 pattern[0].x=-2;
 check(ds1::rectangleCoverage4x({-8,-8,10232,5752},{0,0,8192,8192},pattern,samples));
 check(samples.size()==640*360*4);
 for(auto s:samples)check(s.x<1280&&s.y<720);
 check(ds1::rectangleCoverage4x({-1000,-1000,-500,-500},{0,0,8192,8192},pattern,samples)&&samples.empty());
 check(ds1::rectangleCoverage4x({0,0,0,0},{0,0,1,1},pattern,samples)&&samples.empty());
 // Half-pixel-aligned bounds cover the same complete pixels for every
 // strictly interior 1/16 offset, including center-sampled coverage.
 for(int dx=-7;dx<=7;++dx)for(int dy=-7;dy<=7;++dy){
  for(auto& p:pattern)p={dx,dy};
  check(ds1::rectangleCoverage4x({-8,-8,24,24},{0,0,3,3},pattern,samples));
  check(samples.size()==16);
  for(auto s:samples)check(s.x<4&&s.y<4);
 }
 printf("PASS %u rectangle coverage/integration checks\n",checks);
}
