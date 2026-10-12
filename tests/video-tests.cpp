#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL video %d\n",checks);exit(1);}}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
int main(int argc,char**argv){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);PPCContext c{};
 if(argc>1&&argv[1][0]=='g'){c.r3.u64=64;c.r4.u64=0xFFFFFFFF;__imp__VdGetCurrentDisplayGamma(c,b);return 1;}
 if(argc>1){DWORD old=0;VirtualProtect(b+4096,4096,PAGE_NOACCESS,&old);c.r3.u64=4080;__imp__XGetVideoMode(c,b);return 1;}
 for(uint32_t a:{64u,4080u}){
  memset(b,0xA5,8192);c.r3.u64=a;__imp__XGetVideoMode(c,b);
  check(get(b,a)==1280&&get(b,a+4)==720);check(get(b,a+8)==0&&get(b,a+12)==1&&get(b,a+16)==1);
  uint32_t bits=get(b,a+20);float hz;memcpy(&hz,&bits,4);check(hz==60.0f);
  check(get(b,a+24)==1);check(get(b,a+36)==0&&get(b,a+40)==0&&get(b,a+44)==0);
  for(unsigned i=0;i<8192;++i)if(i<a||i>=a+48)check(b[i]==0xA5);
  unsigned char copy[48];memcpy(copy,b+a,48);memset(b+a,0,48);__imp__VdQueryVideoMode(c,b);check(memcmp(copy,b+a,48)==0);
 }
 c.r3.u64=0xffffffffffffffffull;__imp__VdQueryVideoFlags(c,b);check(c.r3.u64==3);
 __imp__XGetAVPack(c,b);check(c.r3.u64==0);
 auto query=[&](uint32_t category,uint32_t setting,uint32_t address,uint32_t capacity){
  c.r3.u64=category;c.r4.u64=setting;c.r5.u64=address;c.r6.u64=capacity;c.r7.u64=100;
  __imp__ExGetXConfigSetting(c,b);
 };
 query(2,2,64,4);check(c.r3.u64==0&&get(b,64)==0x1000);check(b[100]==0&&b[101]==4);
 query(3,10,64,4);check(c.r3.u64==0&&get(b,64)==0x40000);
 query(2,2,0,0);check(c.r3.u64==0&&b[101]==4);
 query(2,2,64,3);check(c.r3.u64==0xC0000023&&get(b,64)==0x40000&&b[101]==0);
 query(2,2,0,4);check(c.r3.u64==0xC00000F1);
 memset(b,0xA5,8192);c.r3.u64=64;c.r4.u64=4094;__imp__VdGetCurrentDisplayGamma(c,b);
 check(get(b,64)==2);uint32_t gammaBits=get(b,4094);float gamma;memcpy(&gamma,&gammaBits,4);check(gamma>2.22222f&&gamma<2.22223f);
 check(c.r3.u32==64&&c.r4.u32==4094);check(b[63]==0xA5&&b[68]==0xA5&&b[4093]==0xA5&&b[4098]==0xA5);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d video checks: profile, endian, cross-page write, padding and query parity\n",checks);
}
