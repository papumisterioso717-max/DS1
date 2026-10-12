#define NOMINMAX
#include <windows.h>
#include "../runtime/memory.h"
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstdlib>
static int checks=0;
void check(bool ok){++checks;if(!ok){fprintf(stderr,"FAIL memory check %d\n",checks);exit(1);}}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
int main(){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,0x100000000ull,MEM_RESERVE,PAGE_NOACCESS));check(b!=nullptr);
 deadspace::VirtualMemory m(b);uint32_t a=0,s=1;
 check(m.allocate(a,s,0x2000,4,0)==0);check(a==0x10000000&&s==4096);
 MEMORY_BASIC_INFORMATION q{};VirtualQuery(b+a,&q,sizeof(q));check(q.State==MEM_RESERVE);
 uint32_t fixed=a;s=4096;check(m.allocate(a,s,0x1000,4,0)==0);
 check(b[a]==0&&b[a+4095]==0);b[a]=123;
 check(m.allocate(a,s,0x1000,4,0)==0);check(b[a]==123);
 check(m.allocate(a,s,0x2000,4,0)==0xC0000018);
 uint32_t other=0,os=5000;check(m.allocate(other,os,0x3000,4,0)==0);check(other==fixed+4096&&os==8192);
 uint32_t top=0,ts=1;check(m.allocate(top,ts,0x103000,4,0)==0);check(top==0x1ffff000);
 uint32_t bad=0,bs=0;check(m.allocate(bad,bs,0x3000,4,0)==0xC000000D);
 bs=0xffffffff;check(m.allocate(bad,bs,0x3000,4,0)==0xC0000017);
 bs=1;check(m.allocate(bad,bs,0x80003000,4,0)==0xC0000002);
 check(m.allocate(bad,bs,0x3000,0x40,0)==0xC0000002);
 bad=0x82000000;check(m.allocate(bad,bs,0x3000,4,0)==0xC0000018);
 bad=other+4096;bs=8192;check(m.allocate(bad,bs,0x1000,4,0)==0xC0000018);
 s=4096;check(m.release(a,s,0x4000,0)==0);VirtualQuery(b+a,&q,sizeof(q));check(q.State==MEM_RESERVE);
 check(m.allocate(a,s,0x1000,4,0)==0);check(b[a]==0);
 s=0;check(m.release(a,s,0x8000,0)==0);check(s==4096);
 a=0;s=4096;check(m.allocate(a,s,0x3000,2,0)==0);check(a==fixed);
 VirtualQuery(b+a,&q,sizeof(q));check(q.Protect==PAGE_READONLY);
 s=0;check(m.release(a,s,0x8000,0)==0);
 uint32_t large=0,ls=100000;
 check(m.allocate(large,ls,0x60002000,4,0)==0);check((large&65535)==0&&ls==131072);
 uint32_t part=large+1,ps=1;check(m.allocate(part,ps,0x1000,4,0)==0);check(part==large&&ps==65536);
 VirtualQuery(b+part,&q,sizeof(q));check(q.State==MEM_COMMIT);
 VirtualQuery(b+part+65536,&q,sizeof(q));check(q.State==MEM_RESERVE);
 ls=0;check(m.release(large,ls,0x8000,0)==0);
 // Release before using a separate manager for the ABI test.
 os=0;check(m.release(other,os,0x8000,0)==0);ts=0;check(m.release(top,ts,0x8000,0)==0);
 check(VirtualAlloc(b+0x60000000,4096,MEM_COMMIT,PAGE_READWRITE)!=nullptr);
 PPCContext c{};put(b,0x60000000,0);put(b,0x60000004,8193);
 c.r3.u64=0x60000000;c.r4.u64=0x60000004;c.r5.u64=0x3000;c.r6.u64=4;
 __imp__NtAllocateVirtualMemory(c,b);check(c.r3.u32==0);check(get(b,0x60000000)==fixed);check(get(b,0x60000004)==12288);
 c.r3.u64=0xffffffff;c.r4.u64=0x60000004;__imp__NtAllocateVirtualMemory(c,b);check(c.r3.u32==0xC000000D);
 VirtualFree(b,0,MEM_RELEASE);printf("PASS %d memory checks\n",checks);
}
