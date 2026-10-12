#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "objects.h"
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdio>
#include <cstdlib>
namespace {
constexpr uint32_t invalid=0xC000000D,badHandle=0xC0000008;
enum class ObjectKind {Semaphore,Thread,Event};
struct KernelObject {HANDLE handle;ObjectKind kind;uint32_t guestAddress=0;std::mutex eventMutex;std::condition_variable eventChanged;bool manual=false,signaled=false;KernelObject(HANDLE h,ObjectKind k):handle(h),kind(k){}~KernelObject(){if(handle)CloseHandle(handle);}};
std::mutex objectsMutex;
std::map<uint32_t,std::shared_ptr<KernelObject>> objects;
std::map<std::pair<uint8_t*,uint32_t>,std::shared_ptr<KernelObject>> threads;
struct Reference {std::shared_ptr<KernelObject> object;uint32_t count;};
std::map<std::pair<uint8_t*,uint32_t>,Reference> references;
std::map<std::pair<uint8_t*,uint32_t>,std::shared_ptr<KernelObject>> kernelEvents;
uint32_t nextHandle=0xD0000000;
bool valid(uint8_t*b,uint32_t a,uint32_t n,bool write){
 if(!a||uint64_t(a)+n>0x100000000ull)return false;
 for(uint32_t i=0;i<n;++i){MEMORY_BASIC_INFORMATION m{};
  if(!VirtualQuery(b+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
     (write&&!(m.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))))return false;}
 return true;
}
uint32_t get(uint8_t*b,uint32_t a){return(uint32_t(b[a])<<24)|(uint32_t(b[a+1])<<16)|(uint32_t(b[a+2])<<8)|b[a+3];}
void put(uint8_t*b,uint32_t a,uint32_t v){b[a]=v>>24;b[a+1]=v>>16;b[a+2]=v>>8;b[a+3]=v;}
std::shared_ptr<KernelObject> lookup(uint32_t handle){std::lock_guard<std::mutex> guard(objectsMutex);auto it=objects.find(handle);return it==objects.end()?nullptr:it->second;}
[[noreturn]] void pending(const char* mode,PPCContext&ctx){fprintf(stderr,"PENDING_OBJECT_MODE %s guest_lr=0x%08X\n",mode,uint32_t(ctx.lr));std::_Exit(20);}
}
PPC_FUNC(__imp__NtCreateSemaphore){
 const uint32_t out=ctx.r3.u32,attributes=ctx.r4.u32;const int32_t count=ctx.r5.s32,limit=ctx.r6.s32;
 if(!valid(base,out,4,true)){ctx.r3.u64=invalid;return;}
 if(attributes){
  if(!valid(base,attributes,12,false)){ctx.r3.u64=invalid;return;}
  if(get(base,attributes)||get(base,attributes+4)||get(base,attributes+8))pending("named semaphore or attributes",ctx);
 }
 if(count<0||limit<=0||count>limit){put(base,out,0);ctx.r3.u64=invalid;return;}
 HANDLE native=CreateSemaphoreW(nullptr,count,limit,nullptr);
 if(!native){put(base,out,0);ctx.r3.u64=0xC0000017;return;}
 auto object=std::make_shared<KernelObject>(native,ObjectKind::Semaphore);
 {std::lock_guard<std::mutex> guard(objectsMutex);
  if(nextHandle>0xDFFFFFFC){ctx.r3.u64=0xC0000017;return;}
  const uint32_t h=nextHandle;nextHandle+=4;objects.emplace(h,object);put(base,out,h);
  fprintf(stderr,"NtCreateSemaphore handle=0x%08X count=%d limit=%d\n",h,count,limit);
 }
 ctx.r3.u64=0;
}
PPC_FUNC(__imp__NtReleaseSemaphore){
 auto object=lookup(ctx.r3.u32);const uint32_t out=ctx.r5.u32;const int32_t count=ctx.r4.s32;
 if(!object){ctx.r3.u64=badHandle;return;}
 if(object->kind!=ObjectKind::Semaphore){ctx.r3.u64=0xC0000024;return;}
 if(count<=0||(out&&!valid(base,out,4,true))){ctx.r3.u64=invalid;return;}
 LONG previous=0;
 if(!ReleaseSemaphore(object->handle,count,&previous)){ctx.r3.u64=GetLastError()==ERROR_TOO_MANY_POSTS?0xC0000047:invalid;return;}
 if(out)put(base,out,uint32_t(previous));ctx.r3.u64=0;
}
namespace {
void waitObject(PPCContext& ctx,uint8_t* base,const std::shared_ptr<KernelObject>& object,uint32_t mode,uint32_t alertable,uint32_t ptr){
 if(mode>1){ctx.r3.u64=invalid;return;}
 if(alertable)pending("alertable wait requires guest APC support",ctx);
 DWORD ms=INFINITE;
 if(ptr){
  if(!valid(base,ptr,8,false)){ctx.r3.u64=invalid;return;}
  const uint64_t bits=(uint64_t(get(base,ptr))<<32)|get(base,ptr+4);
  uint64_t ticks;
  if(bits&0x8000000000000000ull)ticks=(~bits)+1;
  else {FILETIME ft;GetSystemTimeAsFileTime(&ft);const uint64_t now=(uint64_t(ft.dwHighDateTime)<<32)|ft.dwLowDateTime;ticks=bits>now?bits-now:0;}
  const uint64_t millis=ticks/10000+(ticks%10000!=0);
  if(millis>=INFINITE)pending("timeout beyond host finite-wait range",ctx);
  ms=DWORD(millis);
 }
 if(object->kind==ObjectKind::Event){
  std::unique_lock<std::mutex> lock(object->eventMutex);
  bool ready;
  if(ms==INFINITE){object->eventChanged.wait(lock,[&]{return object->signaled;});ready=true;}
  else ready=object->eventChanged.wait_for(lock,std::chrono::milliseconds(ms),[&]{return object->signaled;});
  if(ready&&!object->manual){object->signaled=false;if(object->guestAddress)put(base,object->guestAddress+4,0);}
  ctx.r3.u64=ready?0:0x102;return;
 }
 DWORD result=WaitForSingleObject(object->handle,ms);
 ctx.r3.u64=result==WAIT_OBJECT_0?0:result==WAIT_TIMEOUT?0x102:badHandle;
}
std::shared_ptr<KernelObject> kernelObject(uint8_t* base,uint32_t address,PPCContext& ctx){
 std::lock_guard<std::mutex> guard(objectsMutex);
 for(const auto& entry:threads)if(entry.first.first==base&&entry.second->guestAddress==address&&address)return entry.second;
 if(!valid(base,address,16,true)||(address&3))pending("invalid kernel object address",ctx);
 // Guest-initialized dispatcher events: type at +0, BE signal state at +4.
 if(base[address]>1)pending("unsupported kernel dispatcher object type",ctx);
 const auto key=std::make_pair(base,address);auto it=kernelEvents.find(key);
 if(it!=kernelEvents.end())return it->second;
 auto object=std::make_shared<KernelObject>(nullptr,ObjectKind::Event);
 object->guestAddress=address;object->manual=base[address]==0;object->signaled=get(base,address+4)!=0;
 kernelEvents.emplace(key,object);
 fprintf(stderr,"KERNEL_EVENT_REGISTER address=%08X manual=%d signaled=%d\n",address,object->manual,object->signaled);
 return object;
}
}
PPC_FUNC(__imp__NtWaitForSingleObjectEx){
 auto object=lookup(ctx.r3.u32);if(!object){ctx.r3.u64=badHandle;return;}
 waitObject(ctx,base,object,ctx.r4.u32,ctx.r5.u32,ctx.r6.u32);
}
PPC_FUNC(__imp__KeWaitForSingleObject){
 auto object=kernelObject(base,ctx.r3.u32,ctx);
 waitObject(ctx,base,object,ctx.r5.u32,ctx.r6.u32,ctx.r7.u32);
}
PPC_FUNC(__imp__KeSetEvent){
 if(ctx.r5.u32)pending("atomic set-and-wait sequence",ctx);
 auto object=kernelObject(base,ctx.r3.u32,ctx);
 if(object->kind!=ObjectKind::Event)pending("KeSetEvent on non-event",ctx);
 std::lock_guard<std::mutex> lock(object->eventMutex);const bool previous=object->signaled;
 object->signaled=true;put(base,object->guestAddress+4,1);
 if(object->manual)object->eventChanged.notify_all();else object->eventChanged.notify_one();
 ctx.r3.u64=previous?1:0;
}
PPC_FUNC(__imp__KeResetEvent){
 auto object=kernelObject(base,ctx.r3.u32,ctx);
 if(object->kind!=ObjectKind::Event)pending("KeResetEvent on non-event",ctx);
 std::lock_guard<std::mutex> lock(object->eventMutex);const bool previous=object->signaled;
 object->signaled=false;put(base,object->guestAddress+4,0);ctx.r3.u64=previous?1:0;
}
PPC_FUNC(__imp__NtClose){
 std::lock_guard<std::mutex> guard(objectsMutex);auto it=objects.find(ctx.r3.u32);
 if(it==objects.end()){ctx.r3.u64=badHandle;return;}objects.erase(it);ctx.r3.u64=0;
}

void registerCurrentGuestThread(uint8_t*base,uint32_t pcr){
 HANDLE native=nullptr;
 if(!DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&native,0,FALSE,DUPLICATE_SAME_ACCESS)){
  fprintf(stderr,"Unable to register native thread handle\n");std::_Exit(22);
 }
 auto object=std::make_shared<KernelObject>(native,ObjectKind::Thread);
 if(valid(base,pcr+0x100,4,false))object->guestAddress=get(base,pcr+0x100);
 std::lock_guard<std::mutex> guard(objectsMutex);
 if(!threads.emplace(std::make_pair(base,pcr),object).second){fprintf(stderr,"Thread already registered\n");std::_Exit(22);}
}
PPC_FUNC(__imp__NtDuplicateObject){
 const uint32_t source=ctx.r3.u32,out=ctx.r4.u32,options=ctx.r5.u32;
 if(options&~3u)pending("unsupported duplicate options",ctx);
 std::lock_guard<std::mutex> guard(objectsMutex);
 std::shared_ptr<KernelObject> object;
 if(source==0xFFFFFFFE){auto it=threads.find({base,ctx.r13.u32});if(it!=threads.end())object=it->second;}
 else {auto it=objects.find(source);if(it!=objects.end())object=it->second;}
 auto closeSource=[&]{if(options&1)objects.erase(source);};
 if(!object){if(out&&valid(base,out,4,true))put(base,out,0xffffffff);closeSource();ctx.r3.u64=badHandle;return;}
 if(out&&!valid(base,out,4,true)){closeSource();ctx.r3.u64=invalid;return;}
 if(!out){closeSource();ctx.r3.u64=(options&1)?0:invalid;return;}
 if(nextHandle>0xDFFFFFFC){closeSource();ctx.r3.u64=0xC0000017;return;}
 const uint32_t handle=nextHandle;nextHandle+=4;objects.emplace(handle,object);
 put(base,out,handle);closeSource();ctx.r3.u64=0;
 fprintf(stderr,"NtDuplicateObject source=0x%08X target=0x%08X kind=%s\n",source,handle,object->kind==ObjectKind::Thread?"thread":object->kind==ObjectKind::Event?"event":"semaphore");
}

PPC_FUNC(__imp__ObReferenceObjectByHandle){
 const uint32_t handle=ctx.r3.u32,type=ctx.r4.u32,out=ctx.r5.u32;
 std::lock_guard<std::mutex> guard(objectsMutex);
 std::shared_ptr<KernelObject> object;
 if(handle==0xfffffffe){auto it=threads.find({base,ctx.r13.u32});if(it!=threads.end())object=it->second;}
 else{auto it=objects.find(handle);if(it!=objects.end())object=it->second;}
 if(!object){ctx.r3.u64=badHandle;return;}
 if(object->kind!=ObjectKind::Thread)pending("guest structure for non-thread object",ctx);
 // Opaque ExThreadObjectType identity bound by the loader. No type descriptor
 // callbacks are implemented yet; the identity supports this comparison only.
 if(type&&type!=0x60030000){ctx.r3.u64=0xC0000024;return;}
 if(!object->guestAddress||!valid(base,object->guestAddress,0xAB0,false))pending("unregistered guest thread structure",ctx);
 if(out&&!valid(base,out,4,true)){ctx.r3.u64=invalid;return;}
 auto key=std::make_pair(base,object->guestAddress);auto it=references.find(key);
 if(it==references.end())references.emplace(key,Reference{object,1});
 else {if(it->second.count==0xffffffff){ctx.r3.u64=0xC0000017;return;}++it->second.count;}
 if(out)put(base,out,object->guestAddress);ctx.r3.u64=0;
 fprintf(stderr,"ObReferenceObjectByHandle handle=0x%08X thread=0x%08X\n",handle,object->guestAddress);
}
PPC_FUNC(__imp__ObDereferenceObject){
 std::lock_guard<std::mutex> guard(objectsMutex);
 auto it=references.find({base,ctx.r3.u32});
 if(it==references.end()){fprintf(stderr,"INVALID_OBJECT_DEREFERENCE address=0x%08X\n",ctx.r3.u32);std::_Exit(22);}
 if(--it->second.count==0)references.erase(it);ctx.r3.u64=0;
}

// Diagnostic compatibility mapping follows Xenia XThread::SetPriority.
// This does not reproduce the Xbox scheduler or dynamic priority boosts.
PPC_FUNC(__imp__KeSetBasePriorityThread){
 const uint32_t address=ctx.r3.u32;const int32_t increment=ctx.r4.s32;
 std::lock_guard<std::mutex> guard(objectsMutex);
 std::shared_ptr<KernelObject> object;
 for(const auto& entry:threads)if(entry.first.first==base&&entry.second->guestAddress==address&&address){object=entry.second;break;}
 if(!object)pending("priority for unregistered thread",ctx);
 const int previous=GetThreadPriority(object->handle);
 if(previous==THREAD_PRIORITY_ERROR_RETURN)pending("native priority query failed",ctx);
 const int target=increment>34?THREAD_PRIORITY_HIGHEST:increment>17?THREAD_PRIORITY_ABOVE_NORMAL:
   increment<-34?THREAD_PRIORITY_LOWEST:increment<-17?THREAD_PRIORITY_BELOW_NORMAL:THREAD_PRIORITY_NORMAL;
 if(!SetThreadPriority(object->handle,target))pending("native priority update failed",ctx);
 ctx.r3.s64=previous;
 fprintf(stderr,"KeSetBasePriorityThread thread=0x%08X increment=%d native=%d previous=%d\n",address,increment,target,previous);
}

PPC_FUNC(__imp__NtCreateEvent){
 const uint32_t out=ctx.r3.u32,attributes=ctx.r4.u32,type=ctx.r5.u32;
 if(!valid(base,out,4,true)||type>1){ctx.r3.u64=invalid;return;}
 if(attributes){
  if(!valid(base,attributes,12,false)){ctx.r3.u64=invalid;return;}
  if(get(base,attributes)||get(base,attributes+4)||get(base,attributes+8))pending("named event or attributes",ctx);
 }
 auto object=std::make_shared<KernelObject>(nullptr,ObjectKind::Event);
 object->manual=type==0;object->signaled=ctx.r6.u32!=0;
 std::lock_guard<std::mutex> guard(objectsMutex);
 if(nextHandle>0xDFFFFFFC){ctx.r3.u64=0xC0000017;return;}
 const uint32_t h=nextHandle;nextHandle+=4;objects.emplace(h,object);put(base,out,h);ctx.r3.u64=0;
 fprintf(stderr,"NtCreateEvent handle=0x%08X manual=%d signaled=%d\n",h,object->manual,object->signaled);
}
PPC_FUNC(__imp__NtSetEvent){
 auto object=lookup(ctx.r3.u32);const uint32_t out=ctx.r4.u32;
 if(!object){ctx.r3.u64=badHandle;return;}
 if(object->kind!=ObjectKind::Event){ctx.r3.u64=0xC0000024;return;}
 if(out&&!valid(base,out,4,true)){ctx.r3.u64=invalid;return;}
 std::lock_guard<std::mutex> lock(object->eventMutex);
 if(out)put(base,out,object->signaled?1:0);
 object->signaled=true;
 if(object->manual)object->eventChanged.notify_all();else object->eventChanged.notify_one();
 ctx.r3.u64=0;
}
PPC_FUNC(__imp__NtClearEvent){
 auto object=lookup(ctx.r3.u32);
 if(!object){ctx.r3.u64=badHandle;return;}
 if(object->kind!=ObjectKind::Event){ctx.r3.u64=0xC0000024;return;}
 std::lock_guard<std::mutex> lock(object->eventMutex);object->signaled=false;ctx.r3.u64=0;
}

uint32_t registerNativeGuestThread(uint8_t*base,uint32_t pcr,void*native){
 auto object=std::make_shared<KernelObject>(native,ObjectKind::Thread);object->guestAddress=get(base,pcr+0x100);
 std::lock_guard<std::mutex> guard(objectsMutex);
 if(nextHandle>0xDFFFFFFC||threads.count({base,pcr})){fprintf(stderr,"THREAD_REGISTRATION_FAILED\n");std::_Exit(22);}
 uint32_t handle=nextHandle;nextHandle+=4;objects.emplace(handle,object);threads.emplace(std::make_pair(base,pcr),object);return handle;
}
PPC_FUNC(__imp__NtResumeThread){
 auto object=lookup(ctx.r3.u32);uint32_t out=ctx.r4.u32;
 if(!object){ctx.r3.u64=badHandle;return;}
 if(object->kind!=ObjectKind::Thread){ctx.r3.u64=0xC0000024;return;}
 if(out&&!valid(base,out,4,true)){ctx.r3.u64=invalid;return;}
 // Only this diagnostic runtime changes native suspension counts.
 std::lock_guard<std::mutex> guard(objectsMutex);
 const uint32_t address=object->guestAddress;
 const uint8_t old=address?base[address+0xBC]:0;
 if(address)base[address+0xBC]=0;
 DWORD previous=ResumeThread(object->handle);
 if(previous==DWORD(-1)){if(address)base[address+0xBC]=old;ctx.r3.u64=invalid;return;}
 if(out)put(base,out,previous);ctx.r3.u64=0;
}

PPC_FUNC(__imp__KeSetAffinityThread){
 const uint32_t address=ctx.r3.u32,mask=ctx.r4.u32,out=ctx.r5.u32;
 if(!mask||(mask&~0x3Fu)||(mask&(mask-1))){ctx.r3.u64=invalid;return;}
 if(out&&!valid(base,out,4,true)){ctx.r3.u64=invalid;return;}
 std::lock_guard<std::mutex> guard(objectsMutex);
 std::shared_ptr<KernelObject> object;uint32_t pcr=0;
 for(const auto& item:threads)if(item.first.first==base&&item.second->guestAddress==address&&address){object=item.second;pcr=item.first.second;break;}
 if(!object){ctx.r3.u64=badHandle;return;}
 if(!valid(base,pcr+0x10C,1,true)||!valid(base,address+0xBF,1,true))pending("invalid affinity thread state",ctx);
 unsigned cpu=0;while(!(mask&(1u<<cpu)))++cpu;
 // Map six guest logical CPUs cyclically onto the process's available CPUs.
 // This is a diagnostic mapping, not an emulation of Xbox topology or timing.
 DWORD_PTR processMask=0,systemMask=0;
 if(!GetProcessAffinityMask(GetCurrentProcess(),&processMask,&systemMask)||!processMask)pending("host affinity unavailable",ctx);
 DWORD_PTR available[64]{};unsigned count=0;
 for(unsigned i=0;i<sizeof(DWORD_PTR)*8;++i)if(processMask&(DWORD_PTR(1)<<i))available[count++]=DWORD_PTR(1)<<i;
 const uint8_t old=base[pcr+0x10C];if(old>=6)pending("invalid previous guest CPU",ctx);
 if(!SetThreadAffinityMask(object->handle,available[cpu%count]))pending("native affinity update failed",ctx);
 base[pcr+0x10C]=uint8_t(cpu);base[address+0xBF]=uint8_t(cpu);
 if(out)put(base,out,1u<<old);ctx.r3.u64=0;
 fprintf(stderr,"KeSetAffinityThread thread=0x%08X guest_cpu=%u previous=%u host_mask=0x%llX\n",address,cpu,old,(unsigned long long)available[cpu%count]);
}
