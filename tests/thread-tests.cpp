#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/threads.h"
#include <atomic>
#include <initializer_list>
#include <cstdio>
#include <cstdlib>
static std::atomic<int> calls{0};static int checks=0;
void check(bool x){++checks;if(!x){fprintf(stderr,"FAIL thread check %d\n",checks);exit(1);}}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
uint32_t get(uint8_t*b,uint32_t a){return uint32_t(b[a])<<24|uint32_t(b[a+1])<<16|uint32_t(b[a+2])<<8|b[a+3];}
void worker(PPCContext&c,uint8_t*b){
 if(diagnosticContext!=&c||get(b,get(b,c.r13.u32))!=0x11223344||(c.r1.u32&0xF)!=0)std::abort();
 calls.fetch_add(1);c.r3.u64=0x1234;__imp__ExTerminateThread(c,b);std::abort();
}
void startup(PPCContext&c,uint8_t*b){if(c.r3.u32!=PPC_CODE_BASE||c.r4.u32!=0x12345678)std::abort();calls.fetch_add(1);c.r3.u64=99;}
int main(){
 auto*b=(uint8_t*)VirtualAlloc(nullptr,PPC_MEMORY_SIZE,MEM_RESERVE,PAGE_NOACCESS);check(b!=nullptr);
 check(VirtualAlloc(b+0x10000,0x10000,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 check(VirtualAlloc(b+PPC_IMAGE_BASE+PPC_IMAGE_SIZE,0x10000,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 PPC_LOOKUP_FUNC(b,PPC_CODE_BASE)=worker;PPC_LOOKUP_FUNC(b,PPC_CODE_BASE+4)=startup;
 put(b,0x10100,0x11223344);configureGuestThreads(b,4,4,0x10100,4,0x10000);
 PPCContext c{};auto create=[&](uint32_t trampoline,uint32_t flags){c.r3.u64=0x10000;c.r4.u64=0;c.r5.u64=0x10004;c.r6.u64=trampoline;c.r7.u64=PPC_CODE_BASE;c.r8.u64=0x12345678;c.r9.u64=flags;__imp__ExCreateThread(c,b);return c.r3.u32;};
 auto wait=[&](uint32_t h,uint32_t timeout){c.r3.u64=h;c.r4.u64=0;c.r5.u64=0;c.r6.u64=timeout;__imp__NtWaitForSingleObjectEx(c,b);return c.r3.u32;};
 check(create(0,1)==0);uint32_t h=get(b,0x10000);check(get(b,0x10004)==2);check(calls==0);check(wait(h,0x10010)==0x102);
 check(get(b,0x61000100)==0x61001000);check(get(b,0x61001068)==0x61002000);check(b[0x610010BC]==1);
 MEMORY_BASIC_INFORMATION m{};VirtualQuery(b+0x71000000,&m,sizeof(m));check(m.State==MEM_RESERVE);
 c.r3.u64=h;c.r4.u64=0x10008;c.r5.u64=2;__imp__NtDuplicateObject(c,b);check(c.r3.u32==0);uint32_t copy=get(b,0x10008);
 c.r3.u64=h;__imp__NtClose(c,b);check(c.r3.u32==0);
 c.r3.u64=copy;c.r4.u64=0x1000C;__imp__NtResumeThread(c,b);check(c.r3.u32==0&&get(b,0x1000C)==1);
 put(b,0x10010,0xFFFFFFFF);put(b,0x10014,uint32_t(-20000000));check(wait(copy,0x10010)==0);check(calls==1);
 check(get(b,0x61001140)==0x1234);check(get(b,0x61001004)==1);check(b[0x610010BC]==0);
 check(create(PPC_CODE_BASE+4,0)==0);h=get(b,0x10000);check(wait(h,0x10010)==0);check(calls==2);check(get(b,0x61011140)==0);
 for(auto handle:{h,copy}){c.r3.u64=handle;__imp__NtClose(c,b);check(c.r3.u32==0);}
 c.r3.u64=0x10000;c.r4.u64=0;c.r5.u64=0;c.r6.u64=0;c.r7.u64=1;c.r9.u64=0;__imp__ExCreateThread(c,b);check(c.r3.u32==0xC000000D);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d thread checks: suspended creation, TLS, stack guard, resume, exit, startup and shared handle lifetime\n",checks);
}
