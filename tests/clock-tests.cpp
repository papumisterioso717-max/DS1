#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <cmath>
static int checks=0;
void check(bool b){++checks;if(!b){fprintf(stderr,"FAIL clock %d\n",checks);exit(1);}}
int main(){
 PPCContext c{};c.r3.u64=~uint64_t(0);__imp__KeQueryPerformanceFrequency(c,nullptr);check(c.r3.u64==50000000);
 auto last=ppc_read_timebase();for(int i=0;i<10000;++i){auto now=ppc_read_timebase();check(now>=last);last=now;}
 LARGE_INTEGER f,q0,q1;QueryPerformanceFrequency(&f);QueryPerformanceCounter(&q0);auto a=ppc_read_timebase();
 Sleep(50);auto b=ppc_read_timebase();QueryPerformanceCounter(&q1);
 double guest=double(b-a)/c.r3.u64,host=double(q1.QuadPart-q0.QuadPart)/f.QuadPart;
 check(guest>0.04);check(std::abs(guest-host)<0.01);
 bool ok[4]={true,true,true,true};std::thread workers[4];
 for(int i=0;i<4;++i)workers[i]=std::thread([&,i]{auto previous=ppc_read_timebase();for(int n=0;n<10000;++n){auto next=ppc_read_timebase();if(next<previous)ok[i]=false;previous=next;}});
 for(int i=0;i<4;++i){workers[i].join();check(ok[i]);}
 printf("PASS %d clock checks; guest %.6f seconds vs host %.6f seconds; 4 monotonic reader threads\n",checks,guest,host);
}
