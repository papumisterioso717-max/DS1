#define NOMINMAX
#include <windows.h>
#include "../runtime/physical.h"
#include "../runtime/video-lock.h"
#include <cstdio>
#include <cstdlib>
static int checks;
void check(bool ok){++checks;if(!ok)exit(1);}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
int main(){
 auto*b=reserveGuestMemory();check(b!=nullptr);
 check(VirtualAlloc(b+0x10000,0x4000,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 const uint32_t slot=0x10000;
 put(b,slot,0xC1010100);check(initializeVideoCalibrationLock(b,slot));
 const uint32_t lock=get(b,slot);check(lock!=0&&lock!=0xC1010100);
 check(b[lock]==1&&b[lock+1]==40);check(get(b,lock+16)==0xFFFFFFFF);
 check(get(b,lock+20)==0&&get(b,lock+24)==0);
 put(b,0x11100,0x12000);PPCContext c{};c.r13.u64=0x11000;c.r3.u64=lock;
 __imp__RtlEnterCriticalSection(c,b);check(get(b,lock+20)==1&&get(b,lock+24)==0x12000);
 __imp__RtlEnterCriticalSection(c,b);check(get(b,lock+20)==2);
 __imp__RtlLeaveCriticalSection(c,b);__imp__RtlLeaveCriticalSection(c,b);
 check(get(b,lock+16)==0xFFFFFFFF&&get(b,lock+20)==0&&get(b,lock+24)==0);
 c.r3.u64=lock;__imp__MmGetPhysicalAddress(c,b);check(get(b,0xA0000000+c.r3.u32+16)==0xFFFFFFFF);
 c.r4.u64=lock;__imp__MmFreePhysicalMemory(c,b);check(releaseGuestMemory(b));
 printf("PASS %d video calibration lock integration checks\n",checks);
}
