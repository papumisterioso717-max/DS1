#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <future>
#include <chrono>
int main(int argc,char**){
 int n=0;auto check=[&](bool v){++n;if(!v){fprintf(stderr,"FAIL kernel event %d\n",n);exit(1);}};
 auto*b=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);check(b!=nullptr);
 // Two inline dispatcher headers; sentinel list fields must remain unchanged.
 b[64]=1;b[80]=0;b[72]=0xA5;b[88]=0x5A;
 auto wait=[&](uint32_t a,uint32_t timeout){PPCContext c{};c.r3.u64=a;c.r4.u64=3;c.r5.u64=1;c.r7.u64=timeout;__imp__KeWaitForSingleObject(c,b);return c.r3.u32;};
 auto set=[&](uint32_t a){PPCContext c{};c.r3.u64=a;__imp__KeSetEvent(c,b);return c.r3.u32;};
 auto reset=[&](uint32_t a){PPCContext c{};c.r3.u64=a;__imp__KeResetEvent(c,b);return c.r3.u32;};
 if(argc>1){b[96]=5;wait(96,128);return 1;}
 check(wait(64,128)==0x102);check(set(64)==0);check(b[71]==1);check(set(64)==1);check(wait(64,128)==0);check(b[71]==0);check(wait(64,128)==0x102);
 check(set(80)==0);check(wait(80,128)==0);check(wait(80,128)==0);check(reset(80)==1);check(reset(80)==0);check(wait(80,128)==0x102);
 // An actual blocked waiter must wake, then auto-reset must consume the signal.
 std::promise<uint32_t> done;auto f=done.get_future();
 std::thread t([&]{done.set_value(wait(64,0));});
 check(f.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);set(64);
 check(f.wait_for(std::chrono::seconds(2))==std::future_status::ready);check(f.get()==0);t.join();check(wait(64,128)==0x102);
 // Relative -10000 ticks = 1 ms; timeout bytes use Xbox big endian.
 const uint64_t bits=uint64_t(-10000ll);for(int i=0;i<8;++i)b[144+i]=uint8_t(bits>>(56-i*8));
 check(wait(64,144)==0x102);check(b[72]==0xA5&&b[88]==0x5A);
 check(VirtualFree(b,0,MEM_RELEASE));printf("PASS %d kernel event checks: poll, timeout, wake, reset, state and preserved fields\n",n);
}
