#include "generated-instruction-tests.cpp"
#include <cstdio>
#include <random>
#include <limits>
#include <algorithm>
#include <array>
#include <cassert>
void shared_exit(PPCContext& ctx,uint8_t*){ctx.r6.u64=88;}
static size_t checks=0;
static void require(bool x,const char*msg){++checks;if(!x){fprintf(stderr,"FAIL %s\n",msg);exit(1);}}
using Fn=void(*)(PPCContext&,uint8_t*);
int main(){
 alignas(64) uint8_t memory[256]{};std::mt19937 rng(0xdead360);
 Fn fns[8][3]={
 {test_vaddsws_3,test_vaddsws_4,test_vaddsws_5},
 {test_vadduhs_3,test_vadduhs_4,test_vadduhs_5},
 {test_vcmpgtuw_3,test_vcmpgtuw_4,test_vcmpgtuw_5},
 {test_vminsw_3,test_vminsw_4,test_vminsw_5},
 {test_vpkswss128_3,test_vpkswss128_4,test_vpkswss128_5},
 {test_vnor128_3,test_vnor128_4,test_vnor128_5},
 {test_vpkuwus128_3,test_vpkuwus128_4,test_vpkuwus128_5},
 {test_vsro128_3,test_vsro128_4,test_vsro128_5}};
 for(unsigned iter=0;iter<2000;iter++)for(unsigned op=0;op<8;op++)for(unsigned alias=0;alias<3;alias++){
  PPCContext c{};c.fpscr.loadFromHost();PPCVRegister a{},b{},expected{};uint32_t sat=iter&1;
  for(unsigned i=0;i<4;i++){a.u32[i]=rng();b.u32[i]=rng();}
  if(iter<5){const int32_t edge[]={INT32_MIN,INT32_MAX,0,-1,1};for(unsigned i=0;i<4;i++){a.s32[i]=edge[(iter+i)%5];b.s32[i]=edge[(iter+2*i)%5];}}
  c.v4=a;c.v5=b;c.vscr=0x10000|sat;
  if(op==0)for(unsigned i=0;i<4;i++){int64_t n=int64_t(a.s32[i])+b.s32[i];expected.s32[i]=int32_t(std::clamp(n,int64_t(INT32_MIN),int64_t(INT32_MAX)));sat|=n!=expected.s32[i];}
  if(op==1)for(unsigned i=0;i<8;i++){unsigned n=unsigned(a.u16[i])+b.u16[i];expected.u16[i]=uint16_t(std::min(n,65535u));sat|=n>65535;}
  if(op==2)for(unsigned i=0;i<4;i++)expected.u32[i]=a.u32[i]>b.u32[i]?UINT32_MAX:0;
  if(op==3)for(unsigned i=0;i<4;i++)expected.s32[i]=std::min(a.s32[i],b.s32[i]);
  if(op==4)for(unsigned guest=0;guest<8;guest++){int32_t n=guest<4?a.s32[3-guest]:b.s32[7-guest];expected.s16[7-guest]=int16_t(std::clamp(n,-32768,32767));sat|=n<-32768||n>32767;}
  if(op==5)for(unsigned i=0;i<16;i++)expected.u8[i]=uint8_t(~(a.u8[i]|b.u8[i]));
  if(op==6)for(unsigned guest=0;guest<8;guest++){uint32_t n=guest<4?a.u32[3-guest]:b.u32[7-guest];expected.u16[7-guest]=uint16_t(std::min(n,65535u));sat|=n>65535;}
  if(op==7){unsigned shift=(b.u8[0]&0x78)/8;for(unsigned guest=0;guest<16;guest++)expected.u8[15-guest]=guest<shift?0:a.u8[15-(guest-shift)];}
  fns[op][alias](c,memory);auto&actual=alias==0?c.v3:alias==1?c.v4:c.v5;
  require(memcmp(&actual,&expected,16)==0,"vector result / source alias");require(c.vscr==(0x10000|sat),"sticky SAT");
 }
 Fn conversions[3][2]={{test_convert_0_3,test_convert_0_5},{test_convert_2_3,test_convert_2_5},{test_convert_31_3,test_convert_31_5}};
 const float values[]={0.f,-0.f,-1.f,-.5f,.5f,1.f,1.75f,65535.f,2147483648.f,4294967040.f,4294967296.f,INFINITY,-INFINITY,NAN,std::numeric_limits<float>::denorm_min()};
 const unsigned shifts[]={0,2,31};
 for(unsigned k=0;k<3;k++)for(unsigned alias=0;alias<2;alias++)for(float x:values){PPCContext c{};c.fpscr.loadFromHost();for(auto&v:c.v5.f32)v=x;conversions[k][alias](c,memory);auto&actual=alias?c.v5:c.v3;long double n=std::ldexp((long double)x,shifts[k]);uint32_t expected=std::isnan(x)||n<0?0:n>=4294967296.L?UINT32_MAX:uint32_t(n);for(auto a:actual.u32)require(a==expected,"float unsigned conversion");if(std::isnan(x))require((c.vscr&1)==0,"NaN does not set SAT");}
 for(unsigned i=0;i<1000;i++){PPCContext c{};c.fpscr.loadFromHost();c.r4.u64=(uint64_t(rng())<<32)|rng();c.r5.u64=(uint64_t(rng())<<32)|rng();uint64_t expected=~(c.r4.u64^c.r5.u64);test_eqv(c,memory);require(c.r3.u64==expected,"eqv");test_eqv_record(c,memory);require(c.cr0.lt==(int32_t(expected)<0)&&c.cr0.gt==(int32_t(expected)>0)&&c.cr0.eq==(int32_t(expected)==0),"eqv record");}
 for(unsigned pattern=0;pattern<3;pattern++){PPCContext c{};c.fpscr.loadFromHost();for(unsigned i=0;i<4;i++){c.v4.u32[i]=pattern==0?2:pattern==1?0:i;c.v5.u32[i]=1;}test_vcmpgtuw_record(c,memory);require(c.cr6.lt==(pattern==0)&&c.cr6.eq==(pattern==1)&&c.cr6.gt==0&&c.cr6.so==0,"vcmpgtuw record");}
 for(float f:{0.f,1.25f,-9.5f}){PPCContext c{};c.fpscr.loadFromHost();c.r4.u64=64;c.f3.f64=f;test_stfsu(c,memory);uint32_t bits;memcpy(&bits,&f,4);require(c.r4.u32==72&&__builtin_bswap32(*reinterpret_cast<uint32_t*>(memory+72))==bits,"stfsu update/endian");}
 for(unsigned i=0;i<256;i++)memory[i]=uint8_t(i);
 for(unsigned offset=0;offset<16;offset++){PPCContext c{};c.fpscr.loadFromHost();c.r4.u64=64;c.r5.u64=offset;test_lvebx(c,memory);require(c.v3.u8[15-offset]==memory[64+offset],"lvebx lane");c.r0.u64=128;c.r5.u64=64+offset;test_lvebx_ra0(c,memory);require(c.v3.u8[15-offset]==memory[64+offset],"lvebx RA0");}
 {PPCContext c{};c.fpscr.loadFromHost();c.r4.u64=64;c.r5.u64=4;uint8_t before[256];memcpy(before,memory,256);test_dcbst(c,memory);require(!memcmp(before,memory,256),"dcbst coherent-memory policy");}
 for(unsigned i=0;i<2;i++){PPCContext c{};c.fpscr.loadFromHost();c.ctr.u64=i?0x1010:0x1008;test_computed(c,memory);require(c.r3.u64==(i?9:7),"computed local jump");}
 {PPCContext c{};c.fpscr.loadFromHost();c.r3.u64=0;c.lr=0x12345678;test_external(c,memory);require(c.r6.u64==88&&c.lr==0x12345678,"external switch tail-call");}
 printf("PASS %zu checks: 13 instruction types, aliases, saturation, NaN, endian, record bits, computed/shared jumps\n",checks);
}


