#define NOMINMAX
#include <windows.h>
#include "../runtime/timestamp.h"
#include <cstdio>
#include <cstdlib>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL timestamp %d\n",checks);exit(1);}}
uint32_t read(uint8_t*b,uint32_t a){return __builtin_bswap32(uint32_t(InterlockedCompareExchange((volatile LONG*)(b+a),0,0)));}
int main(){
 auto*b=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);check(b!=nullptr);
 check(!startGuestTimestamp(b,65,128));check(!startGuestTimestamp(b,64,64));check(!startGuestTimestamp(b,64,0xFFFFFFFC));
 b[127]=0xAA;b[152]=0xBB;check(startGuestTimestamp(b,64,128));check(read(b,64)==128);check(!startGuestTimestamp(b,68,256));
 for(unsigned a=128;a<144;a+=4)check(read(b,a)==0);check(read(b,148)==0);
 const auto before=read(b,144);Sleep(50);const auto after=read(b,144);check(after>before&&after-before<2000);
 check(b[127]==0xAA&&b[152]==0xBB);stopGuestTimestamp();auto stopped=read(b,144);Sleep(20);check(read(b,144)==stopped);stopGuestTimestamp();
 check(startGuestTimestamp(b,64,128));stopGuestTimestamp();VirtualFree(b,0,MEM_RELEASE);
 printf("PASS %d timestamp checks: import, layout, advancing clock, boundaries and callback shutdown\n",checks);
}
