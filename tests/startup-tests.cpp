#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <initializer_list>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL startup check %d\n",checks);exit(1);}}
int main(){
 auto*base=static_cast<uint8_t*>(VirtualAlloc(nullptr,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(base!=nullptr);
 PPCContext c{};c.r3.u64=0xffffffffffffffffull;c.r4.u64=123;c.lr=456;
 __imp__KeGetCurrentProcessType(c,base);check(c.r3.u64==1);check(c.r4.u64==123&&c.lr==456);
 for(uint32_t address: {64u,4080u}){
  memset(base,0xA5,8192);c.r3.u64=address;
  __imp__RtlInitializeCriticalSection(c,base);
  check(c.r3.u64==address);
  for(unsigned i=0;i<8192;++i){
   unsigned char expected=0xA5;
   if(i==address)expected=1;
   if(i==address+1||(i>=address+4&&i<address+8)||(i>=address+20&&i<address+28))expected=0;
   if(i>=address+16&&i<address+20)expected=0xff;
   check(base[i]==expected);
  }
 }
 for(uint32_t spin: {0u,1u,256u,257u,65535u,0xffffffffu}){
  c.r3.u64=64;c.r4.u64=spin;__imp__RtlInitializeCriticalSectionAndSpinCount(c,base);
  check(c.r3.u64==0);uint64_t expected=(uint64_t(spin)+255)/256;
  check(base[65]==(expected>255?255:expected));
 }
 VirtualFree(base,0,MEM_RELEASE);printf("PASS %d startup checks: process type, critical-section fields, boundaries and preserved bytes\n",checks);
}
