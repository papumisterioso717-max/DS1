#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include "notifications.h"
#include <map>
#include <mutex>
#include <cstdio>
#include <cstdlib>
namespace {
std::mutex mutex;
std::map<uint8_t*,std::vector<TitleTerminationNotification>> registrations;
[[noreturn]] void invalid(){fprintf(stderr,"INVALID_TERMINATION_REGISTRATION\n");std::_Exit(22);}
uint32_t read(uint8_t*b,uint32_t a){return uint32_t(b[a])<<24|uint32_t(b[a+1])<<16|uint32_t(b[a+2])<<8|b[a+3];}
}
std::vector<TitleTerminationNotification> queryTitleTerminationNotifications(uint8_t*b){std::lock_guard<std::mutex> lock(mutex);auto it=registrations.find(b);return it==registrations.end()?std::vector<TitleTerminationNotification>{}:it->second;}
PPC_FUNC(__imp__ExRegisterTitleTerminateNotification){
 const uint32_t a=ctx.r3.u32;const bool create=ctx.r4.u32!=0;
 if(!a||uint64_t(a)+16>0x100000000ull)invalid();
 for(unsigned i=0;i<16;++i){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(base+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))invalid();}
 TitleTerminationNotification item{read(base,a),read(base,a+4)};
 if(create&&((item.routine&3)||item.routine<PPC_CODE_BASE||uint64_t(item.routine)>=uint64_t(PPC_CODE_BASE)+PPC_CODE_SIZE))invalid();
 std::lock_guard<std::mutex> lock(mutex);auto& entries=registrations[base];
 if(create)entries.push_back(item);
 else for(auto it=entries.begin();it!=entries.end();++it)if(it->routine==item.routine){entries.erase(it);break;}
 fprintf(stderr,"TITLE_TERMINATION_REGISTRATION create=%u callback=%08X priority=%u count=%zu dispatch=pending\n",create,item.routine,item.priority,entries.size());
 // Registration only. Fatal diagnostic exits do not dispatch guest callbacks.
}
