#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "../runtime/notifications.h"
#include <cstdio>
#include <cstdlib>
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
int main(int argc,char**){int checks=0;auto check=[&](bool v){++checks;if(!v)exit(1);};auto*b=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);check(b!=nullptr);PPCContext c{};
 if(argc>1){c.r3.u64=0xFFFFFFFC;__imp__ExRegisterTitleTerminateNotification(c,b);return 1;}
 auto reg=[&](uint32_t address,bool add){c.r3.u64=address;c.r4.u64=add;__imp__ExRegisterTitleTerminateNotification(c,b);check(c.r3.u32==address);};
 put(b,64,PPC_CODE_BASE);put(b,68,10);put(b,80,PPC_CODE_BASE+4);put(b,84,20);b[72]=0xAA;
 check(queryTitleTerminationNotifications(b).empty());reg(64,true);reg(80,true);reg(64,true);auto entries=queryTitleTerminationNotifications(b);check(entries.size()==3&&entries[0].priority==10&&entries[1].routine==PPC_CODE_BASE+4);check(b[72]==0xAA);
 reg(64,false);entries=queryTitleTerminationNotifications(b);check(entries.size()==2&&entries[0].priority==20);reg(64,false);reg(64,false);check(queryTitleTerminationNotifications(b).size()==1);reg(80,false);check(queryTitleTerminationNotifications(b).empty());VirtualFree(b,0,MEM_RELEASE);printf("PASS %d notification checks: endian, duplicates, removal and preserved fields\n",checks);}
