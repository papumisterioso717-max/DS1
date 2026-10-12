#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/objects.h"
#include <cstdio>
#include <cstdlib>
static int checks=0;
void check(bool x){++checks;if(!x){fprintf(stderr,"FAIL duplicate %d\n",checks);exit(1);}}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
int main(){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);PPCContext c{};c.r13.u64=128;
 registerCurrentGuestThread(b,128);
 auto dup=[&](uint32_t h,uint32_t options,uint32_t out=68){c.r3.u64=h;c.r4.u64=out;c.r5.u64=options;__imp__NtDuplicateObject(c,b);return c.r3.u32;};
 auto wait=[&](uint32_t h){c.r3.u64=h;c.r4.u64=0;c.r5.u64=0;c.r6.u64=96;__imp__NtWaitForSingleObjectEx(c,b);return c.r3.u32;};
 auto close=[&](uint32_t h){c.r3.u64=h;__imp__NtClose(c,b);return c.r3.u32;};
 c.r3.u64=64;c.r4.u64=0;c.r5.u64=1;c.r6.u64=2;__imp__NtCreateSemaphore(c,b);check(c.r3.u32==0);const auto original=get(b,64);
 check(dup(original,2)==0);const auto copy=get(b,68);check(copy!=original);check(wait(copy)==0);check(wait(original)==0x102);
 check(close(original)==0);check(wait(copy)==0x102);check(dup(original,0)==0xC0000008);
 check(dup(copy,1)==0);const auto moved=get(b,68);check(wait(copy)==0xC0000008);check(wait(moved)==0x102);
 check(dup(moved,1,0)==0);check(wait(moved)==0xC0000008);
 check(dup(0xfffffffe,2)==0);const auto thread=get(b,68);check(wait(thread)==0x102);
 c.r3.u64=thread;c.r4.u64=1;c.r5.u64=0;__imp__NtReleaseSemaphore(c,b);check(c.r3.u32==0xC0000024);
 check(dup(thread,0)==0);auto threadCopy=get(b,68);check(close(thread)==0);check(wait(threadCopy)==0x102);
 check(dup(threadCopy,1,0xffffffff)==0xC000000D);check(wait(threadCopy)==0xC0000008);
 check(dup(0xfffffffe,0)==0);check(close(get(b,68))==0);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d duplicate checks: aliasing, source close, lifetime, pseudo-thread and type safety\n",checks);
}
