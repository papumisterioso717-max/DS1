#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <thread>
#include <vector>
#include <cstdio>
#include <cstdlib>
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
static int checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL sync %d\n",checks);exit(1);}}
int main(){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,0x10000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);
 PPCContext contexts[4]{};
 for(unsigned i=0;i<4;++i){contexts[i].r13.u64=0x1000+i*0x200;put(b,uint32_t(contexts[i].r13.u64)+0x100,0x4000+i*0x1000);}
 auto& c=contexts[0];c.r3.u64=64;__imp__RtlInitializeCriticalSection(c,b);
 __imp__RtlEnterCriticalSection(c,b);check(get(b,64+0x10)==0&&get(b,64+0x14)==1&&get(b,64+0x18)==0x4000);
 __imp__RtlEnterCriticalSection(c,b);check(get(b,64+0x10)==1&&get(b,64+0x14)==2);
 contexts[1].r3.u64=64;__imp__RtlTryEnterCriticalSection(contexts[1],b);check(contexts[1].r3.u64==0);
 c.r3.u64=64;__imp__RtlTryEnterCriticalSection(c,b);check(c.r3.u64==1&&get(b,64+0x14)==3);
 c.r3.u64=64;for(int i=0;i<3;++i)__imp__RtlLeaveCriticalSection(c,b);
 check(get(b,64+0x10)==0xffffffff&&get(b,64+0x14)==0&&get(b,64+0x18)==0);
 uint32_t counter=0;std::vector<std::thread> workers;
 for(unsigned i=0;i<4;++i)workers.emplace_back([&,i]{auto& t=contexts[i];for(int j=0;j<10000;++j){t.r3.u64=64;__imp__RtlEnterCriticalSection(t,b);++counter;__imp__RtlLeaveCriticalSection(t,b);}});
 for(auto&t:workers)t.join();check(counter==40000);
 check(get(b,64+0x10)==0xffffffff&&get(b,64+0x14)==0&&get(b,64+0x18)==0);
 // A statically initialized lock is accepted without an initialization call.
 put(b,128+0x10,0xffffffff);c.r3.u64=128;__imp__RtlEnterCriticalSection(c,b);check(get(b,128+0x18)==0x4000);__imp__RtlLeaveCriticalSection(c,b);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d sync assertions; 40000 protected increments across four host threads\n",checks);
}
