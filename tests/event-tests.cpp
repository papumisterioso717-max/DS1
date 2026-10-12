#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
#include <future>
#include <chrono>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL event %d\n",checks);exit(1);}}
uint32_t get(uint8_t*b,int a){return uint32_t(b[a])<<24|uint32_t(b[a+1])<<16|uint32_t(b[a+2])<<8|b[a+3];}
int main(){
 auto*b=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);check(b!=nullptr);
 auto create=[&](int type,int initial){PPCContext c{};c.r3.u64=64;c.r5.u64=type;c.r6.u64=initial;__imp__NtCreateEvent(c,b);check(c.r3.u32==0);return get(b,64);};
 auto wait=[&](uint32_t h,bool poll){PPCContext c{};c.r3.u64=h;c.r6.u64=poll?128:0;__imp__NtWaitForSingleObjectEx(c,b);return c.r3.u32;};
 auto set=[&](uint32_t h){PPCContext c{};c.r3.u64=h;c.r4.u64=68;__imp__NtSetEvent(c,b);check(c.r3.u32==0);return get(b,68);};
 auto clear=[&](uint32_t h){PPCContext c{};c.r3.u64=h;__imp__NtClearEvent(c,b);check(c.r3.u32==0);};
 uint32_t manual=create(0,1);check(wait(manual,true)==0);check(wait(manual,true)==0);check(set(manual)==1);clear(manual);check(wait(manual,true)==0x102);
 auto a=std::async(std::launch::async,[&]{return wait(manual,false);});auto d=std::async(std::launch::async,[&]{return wait(manual,false);});
 check(a.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);check(set(manual)==0);
 check(a.wait_for(std::chrono::seconds(2))==std::future_status::ready);check(d.wait_for(std::chrono::seconds(2))==std::future_status::ready);check(a.get()==0&&d.get()==0);
 uint32_t automatic=create(1,1);check(wait(automatic,true)==0);check(wait(automatic,true)==0x102);
 check(set(automatic)==0);check(set(automatic)==1);check(wait(automatic,true)==0);check(wait(automatic,true)==0x102);
 PPCContext c{};c.r3.u64=automatic;c.r4.u64=72;c.r5.u64=2;__imp__NtDuplicateObject(c,b);check(c.r3.u32==0);uint32_t copy=get(b,72);
 c.r3.u64=automatic;__imp__NtClose(c,b);check(c.r3.u32==0);check(wait(automatic,true)==0xC0000008);check(set(copy)==0);check(wait(copy,true)==0);
 c.r3.u64=copy;c.r4.u64=0xffffffff;__imp__NtSetEvent(c,b);check(c.r3.u32==0xC000000D);check(wait(copy,true)==0x102);
 c.r3.u64=copy;c.r4.u64=1;c.r5.u64=0;__imp__NtReleaseSemaphore(c,b);check(c.r3.u32==0xC0000024);
 c.r3.u64=64;c.r4.u64=0;c.r5.u64=2;__imp__NtCreateEvent(c,b);check(c.r3.u32==0xC000000D);
 for(auto h:{manual,copy}){c.r3.u64=h;__imp__NtClose(c,b);check(c.r3.u32==0);}
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d event checks: manual/auto reset, wake all, state, duplicate lifetime and errors\n",checks);
}
