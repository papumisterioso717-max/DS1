#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/objects.h"
#include <cstdio>
#include <cstdlib>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL reference %d\n",checks);exit(1);}}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
int main(int argc,char**argv){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);PPCContext c{};c.r13.u64=128;
 put(b,128+0x100,4096);registerCurrentGuestThread(b,128);
 c.r3.u64=0xfffffffe;c.r4.u64=64;c.r5.u64=2;__imp__NtDuplicateObject(c,b);check(c.r3.u32==0);const uint32_t h=get(b,64);
 auto ref=[&](uint32_t handle,uint32_t type,uint32_t out){c.r3.u64=handle;c.r4.u64=type;c.r5.u64=out;__imp__ObReferenceObjectByHandle(c,b);return c.r3.u32;};
 put(b,68,0xaabbccdd);check(ref(h,123,68)==0xC0000024);check(get(b,68)==0xaabbccdd);
 check(ref(h,0,0xffffffff)==0xC000000D);check(ref(0x12345678,0,68)==0xC0000008);
 check(ref(h,0x60030000,68)==0&&get(b,68)==4096);check(ref(h,0,72)==0&&get(b,72)==4096);
 c.r3.u64=h;__imp__NtClose(c,b);check(c.r3.u32==0);check(ref(h,0,68)==0xC0000008);
 c.r3.u64=4096;__imp__ObDereferenceObject(c,b);check(c.r3.u32==0);
 c.r3.u64=4096;__imp__ObDereferenceObject(c,b);check(c.r3.u32==0);
 if(argc>1){c.r3.u64=4096;__imp__ObDereferenceObject(c,b);return 1;}
 check(ref(0xfffffffe,0,68)==0);c.r3.u64=4096;__imp__ObDereferenceObject(c,b);check(c.r3.u32==0);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d object-reference checks: type, pointer, lifetime, close and balanced release\n",checks);
}
