#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <future>
#include <chrono>
static int checks=0;
void check(bool v){++checks;if(!v){fprintf(stderr,"FAIL semaphore %d\n",checks);exit(1);}}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
int main(){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);PPCContext c{};
 auto create=[&](int count,int limit){c.r3.u64=64;c.r4.u64=0;c.r5.s64=count;c.r6.s64=limit;__imp__NtCreateSemaphore(c,b);return c.r3.u32;};
 check(create(-1,2)==0xC000000D&&get(b,64)==0);check(create(3,2)==0xC000000D);check(create(0,0)==0xC000000D);
 check(create(1,2)==0);const uint32_t h=get(b,64);check(h!=0);
 auto wait=[&](uint32_t handle,uint32_t ptr){c.r3.u64=handle;c.r4.u64=0;c.r5.u64=0;c.r6.u64=ptr;__imp__NtWaitForSingleObjectEx(c,b);return c.r3.u32;};
 check(wait(h,128)==0);check(wait(h,128)==0x102);
 auto release=[&](int count){c.r3.u64=h;c.r4.s64=count;c.r5.u64=68;__imp__NtReleaseSemaphore(c,b);return c.r3.u32;};
 check(release(2)==0&&get(b,68)==0);check(release(1)==0xC0000047);check(release(0)==0xC000000D);
 check(wait(h,128)==0&&wait(h,128)==0&&wait(h,128)==0x102);
 // An empty semaphore must block until another thread releases one count.
 std::promise<void> started;auto ready=started.get_future();std::promise<uint32_t> result;auto completion=result.get_future();
 std::thread worker([&]{PPCContext w{};w.r3.u64=h;started.set_value();__imp__NtWaitForSingleObjectEx(w,b);result.set_value(w.r3.u32);});
 ready.wait();check(completion.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);
 check(release(1)==0);check(completion.wait_for(std::chrono::seconds(2))==std::future_status::ready);check(completion.get()==0);worker.join();
 const uint64_t relative=uint64_t(-100000ll);put(b,128,relative>>32);put(b,132,uint32_t(relative));check(wait(h,128)==0x102);
 put(b,128,0);put(b,132,1);check(wait(h,128)==0x102);
 c.r3.u64=h;__imp__NtClose(c,b);check(c.r3.u32==0);check(wait(h,128)==0xC0000008);
 c.r3.u64=h;__imp__NtClose(c,b);check(c.r3.u32==0xC0000008);
 check(create(0,1)==0&&get(b,64)!=h);c.r3.u64=get(b,64);__imp__NtClose(c,b);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d semaphore checks: counts, limits, blocking, timeout, stale handles and closure\n",checks);
}
