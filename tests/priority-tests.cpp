#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/objects.h"
#include <cstdio>
#include <cstdlib>
void check(bool b){if(!b){fprintf(stderr,"FAIL priority test\n");exit(1);}}
int main(int argc,char**){
 auto*b=static_cast<uint8_t*>(VirtualAlloc(nullptr,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(b!=nullptr);
 b[128+0x100+2]=0x10;registerCurrentGuestThread(b,128);PPCContext c{};
 if(argc>1){c.r3.u64=4097;c.r4.u64=0;__imp__KeSetBasePriorityThread(c,b);return 1;}
 int original=GetThreadPriority(GetCurrentThread());check(original!=THREAD_PRIORITY_ERROR_RETURN);
 const int inputs[]={0,17,18,34,35,-17,-18,-34,-35,0};
 const int expected[]={0,0,1,1,2,0,-1,-1,-2,0};int previous=original;
 for(int i=0;i<10;++i){c.r3.u64=4096;c.r4.s64=inputs[i];__imp__KeSetBasePriorityThread(c,b);check(c.r3.s64==previous);check(GetThreadPriority(GetCurrentThread())==expected[i]);previous=expected[i];}
 check(SetThreadPriority(GetCurrentThread(),original));VirtualFree(b,0,MEM_RELEASE);
 puts("PASS 10 priority transitions: signed return, boundaries and native thread state");
}
