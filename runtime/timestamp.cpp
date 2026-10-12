#define NOMINMAX
#include <windows.h>
#include "timestamp.h"
#include <chrono>
#include <mutex>
#include <cstring>
namespace {
std::mutex lifecycle;
HANDLE timer=nullptr;
struct Clock {volatile LONG* millis;std::chrono::steady_clock::time_point epoch;} clockState;
bool writable(uint8_t*b,uint32_t a,uint32_t size){
 if(!a||(a&3)||uint64_t(a)+size>0x100000000ull)return false;
 for(uint32_t i=0;i<size;++i){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(b+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))||!(m.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE)))return false;}
 return true;
}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
VOID CALLBACK update(void*opaque,BOOLEAN){
 auto& s=*static_cast<Clock*>(opaque);
 const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-s.epoch).count();
 InterlockedExchange(s.millis,LONG(__builtin_bswap32(uint32_t(elapsed))));
}
}
bool startGuestTimestamp(uint8_t*b,uint32_t slot,uint32_t storage){
 std::lock_guard<std::mutex> guard(lifecycle);
 if(timer||!writable(b,slot,4)||!writable(b,storage,24)||
    (uint64_t(slot)<uint64_t(storage)+24&&uint64_t(storage)<uint64_t(slot)+4))return false;
 // Xenia compatibility layout: milliseconds at +16, other fields zero.
 // Only the aligned 32-bit counter changes after publication.
 memset(b+storage,0,24);
 clockState={reinterpret_cast<volatile LONG*>(b+storage+16),std::chrono::steady_clock::now()};
 if(!CreateTimerQueueTimer(&timer,nullptr,update,&clockState,1,1,WT_EXECUTEINTIMERTHREAD)){timer=nullptr;return false;}
 put(b,slot,storage);return true;
}
void stopGuestTimestamp(){
 std::lock_guard<std::mutex> guard(lifecycle);
 if(timer){DeleteTimerQueueTimer(nullptr,timer,INVALID_HANDLE_VALUE);timer=nullptr;}
}
