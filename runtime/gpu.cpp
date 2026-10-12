#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "gpu.h"
#include <map>
#include <mutex>
#include <cstdio>
namespace {
std::mutex mutex;
std::map<uint8_t*,DiagnosticGpuState> engines;
bool readable(uint8_t*b,uint32_t a,uint32_t n){
 if(!a||uint64_t(a)+n>0x100000000ull)return false;
 for(uint32_t i=0;i<n;++i){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(b+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;}
 return true;
}
}
DiagnosticGpuState queryDiagnosticGpu(uint8_t*b){std::lock_guard<std::mutex> guard(mutex);auto it=engines.find(b);return it==engines.end()?DiagnosticGpuState{}:it->second;}
PPC_FUNC(__imp__VdInitializeEngines){
 const DiagnosticGpuState next{true,ctx.r3.u32,ctx.r4.u32,ctx.r5.u32,ctx.r6.u32,ctx.r7.u32};
 // Capture the request only. Microcode execution and callbacks need a GPU backend.
 if(!readable(base,next.pfp,4)||!readable(base,next.me,4)||
    (next.callback&&((next.callback&3)||next.callback<PPC_CODE_BASE||uint64_t(next.callback)>=uint64_t(PPC_CODE_BASE)+PPC_CODE_SIZE))){ctx.r3.u64=0;return;}
 std::lock_guard<std::mutex> guard(mutex);engines[base]=next;
 fprintf(stderr,"GPU_DIAGNOSTIC_INIT flags=%08X callback=%08X argument=%08X pfp=%08X me=%08X renderer=absent microcode=not-executed\n",next.flags,next.callback,next.argument,next.pfp,next.me);
 // Compatibility return used by Xenia's VdInitializeEngines stub, not GPU readiness.
 ctx.r3.u64=1;
}
PPC_FUNC(__imp__VdShutdownEngines){std::lock_guard<std::mutex> guard(mutex);engines.erase(base);fprintf(stderr,"GPU_DIAGNOSTIC_SHUTDOWN\n");}
