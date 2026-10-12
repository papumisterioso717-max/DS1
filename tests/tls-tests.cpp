#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/tls.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL TLS %d\n",checks);exit(1);}}
uint32_t value(PPCContext&c,uint8_t*b,uint32_t slot){c.r3.u64=slot;__imp__KeTlsGetValue(c,b);return c.r3.u32;}
void set(PPCContext&c,uint8_t*b,uint32_t slot,uint32_t v){c.r3.u64=slot;c.r4.u64=v;__imp__KeTlsSetValue(c,b);check(c.r3.u32==1);}
int main(){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,0x10000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);
 for(int i=0;i<24;++i)b[0x100+i]=uint8_t(i+1);
 registerGuestTls(b,0x1000,0x2000,64,24,0x100,24);registerGuestTls(b,0x3000,0x4000,64,24,0x100,24);
 check(memcmp(b+0x2000,b+0x100,24)==0&&memcmp(b+0x4000,b+0x100,24)==0);
 PPCContext a{},c{};a.r13.u64=0x1000;c.r13.u64=0x3000;
 for(uint32_t i=0;i<64;++i){__imp__KeTlsAlloc(a,b);check(a.r3.u32==i);check(value(a,b,i)==0&&value(c,b,i)==0);}
 __imp__KeTlsAlloc(a,b);check(a.r3.u32==0xffffffff);
 set(a,b,7,0x12345678);set(c,b,7,0xabcdef01);check(value(a,b,7)==0x12345678&&value(c,b,7)==0xabcdef01);
 check(b[0x2018+28]==0x12&&b[0x2018+31]==0x78);
 // Direct guest writes and exported API reads observe the same memory.
 b[0x2018+28]=0x42;check(value(a,b,7)==0x42345678);
 a.r3.u64=7;__imp__KeTlsFree(a,b);check(a.r3.u32==1);check(value(a,b,7)==0&&value(c,b,7)==0);
 __imp__KeTlsAlloc(a,b);check(a.r3.u32==7&&value(a,b,7)==0);
 a.r3.u64=64;a.r4.u64=1;__imp__KeTlsSetValue(a,b);check(a.r3.u32==0);check(value(a,b,0xffffffff)==0);
 a.r3.u64=0xffffffff;__imp__KeTlsFree(a,b);check(a.r3.u32==0);
 check(memcmp(b+0x2000,b+0x100,24)==0);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d TLS checks: exhaustion, isolation, reuse, static data and guest-memory consistency\n",checks);
}
