#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstring>
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
int main(int argc,char**argv){
 auto*b=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!b)return 1;
 put(b,64,0x406D1388);put(b,80,4);put(b,84,0x1000);put(b,88,128);put(b,92,2);strcpy((char*)b+128,"Worker");
 if(argc>1){if(argv[1][0]=='c')put(b,64,0xC0000005);else put(b,88,0xFFFFFFFF);}
 PPCContext c{};c.r3.u64=64;__imp__RtlRaiseException(c,b);
 if(argc>1)return 1;if(c.r3.u32!=64)return 1;VirtualFree(b,0,MEM_RELEASE);puts("PASS thread-name notification; context preserved");
}
