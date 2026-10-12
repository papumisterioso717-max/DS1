#include "ppc_recomp_shared.h"
#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdlib>
// Guest RTL critical sections occupy 28 bytes; native Windows objects cannot
// be placed here. Initialization follows Xenia xboxkrnl_rtl.cc layout.
static void initializeGuestCriticalSection(uint8_t* base,uint32_t a,uint32_t spin) {
 if(!a||uint64_t(a)+28>0x100000000ull){fprintf(stderr,"INVALID_CRITICAL_SECTION\n");std::_Exit(22);}
 for(uint32_t offset=0;offset<28;++offset){
  MEMORY_BASIC_INFORMATION info{};
  if(!VirtualQuery(base+a+offset,&info,sizeof(info))||info.State!=MEM_COMMIT||
     (info.Protect&PAGE_GUARD)||!(info.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE))){
   fprintf(stderr,"INVALID_CRITICAL_SECTION address=0x%08X\n",a);std::_Exit(22);
  }
 }
 base[a]=1;
 const uint64_t packed=(uint64_t(spin)+255)/256;
 base[a+1]=uint8_t(packed>255?255:packed);
 // Header signal state, lock count (-1), recursion count and owner.
 for(uint32_t i=4;i<8;++i)base[a+i]=0;
 for(uint32_t i=16;i<20;++i)base[a+i]=255;
 for(uint32_t i=20;i<28;++i)base[a+i]=0;
}
PPC_FUNC(__imp__XNotifyGetNext) { fprintf(stderr, "PENDING_IMPORT __imp__XNotifyGetNext guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamNotifyCreateListener) { fprintf(stderr, "PENDING_IMPORT __imp__XamNotifyCreateListener guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamLoaderLaunchTitle) { fprintf(stderr, "PENDING_IMPORT __imp__XamLoaderLaunchTitle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowSigninUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowSigninUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowSigninUIp) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowSigninUIp guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowFriendsUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowFriendsUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowPlayersUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowPlayersUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowMessagesUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowMessagesUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowKeyboardUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowKeyboardUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowQuickChatUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowQuickChatUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowVoiceMailUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowVoiceMailUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowGamerCardUIForXUID) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowGamerCardUIForXUID guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowAchievementsUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowAchievementsUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowPlayerReviewUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowPlayerReviewUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowMarketplaceUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowMarketplaceUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowDeviceSelectorUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowDeviceSelectorUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowMessageBoxUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowMessageBoxUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowMessageComposeUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowMessageComposeUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowGameInviteUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowGameInviteUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowFriendRequestUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowFriendRequestUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowDirtyDiscErrorUI) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowDirtyDiscErrorUI guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__Refresh) { fprintf(stderr, "PENDING_IMPORT __imp__Refresh guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMsgStartIORequest) { fprintf(stderr, "PENDING_IMPORT __imp__XMsgStartIORequest guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserGetName) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserGetName guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserGetSigninState) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserGetSigninState guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserAreUsersFriends) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserAreUsersFriends guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserCheckPrivilege) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserCheckPrivilege guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamGetSystemVersion) { fprintf(stderr, "PENDING_IMPORT __imp__XamGetSystemVersion guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserCreateStatsEnumerator) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserCreateStatsEnumerator guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamReadTileToTexture) { fprintf(stderr, "PENDING_IMPORT __imp__XamReadTileToTexture guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamParseGamerTileKey) { fprintf(stderr, "PENDING_IMPORT __imp__XamParseGamerTileKey guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamWriteGamerTile) { fprintf(stderr, "PENDING_IMPORT __imp__XamWriteGamerTile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserCreateAchievementEnumerator) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserCreateAchievementEnumerator guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserGetXUID) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserGetXUID guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserGetSigninInfo) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserGetSigninInfo guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XGetGameRegion) { fprintf(stderr, "PENDING_IMPORT __imp__XGetGameRegion guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamShowMessageBoxUIEx) { fprintf(stderr, "PENDING_IMPORT __imp__XamShowMessageBoxUIEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XGetLanguage) { fprintf(stderr, "PENDING_IMPORT __imp__XGetLanguage guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamLoaderTerminateTitle) { fprintf(stderr, "PENDING_IMPORT __imp__XamLoaderTerminateTitle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamGetExecutionId) { fprintf(stderr, "PENDING_IMPORT __imp__XamGetExecutionId guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlInitializeCriticalSection) { initializeGuestCriticalSection(base,ctx.r3.u32,0); }
PPC_FUNC(__imp__RtlInitializeCriticalSectionAndSpinCount) { initializeGuestCriticalSection(base,ctx.r3.u32,ctx.r4.u32);ctx.r3.u64=0; }
PPC_FUNC(__imp__ObOpenObjectByPointer) { fprintf(stderr, "PENDING_IMPORT __imp__ObOpenObjectByPointer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ObLookupThreadByThreadId) { fprintf(stderr, "PENDING_IMPORT __imp__ObLookupThreadByThreadId guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeQueryBasePriorityThread) { fprintf(stderr, "PENDING_IMPORT __imp__KeQueryBasePriorityThread guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeSetDisableBoostThread) { fprintf(stderr, "PENDING_IMPORT __imp__KeSetDisableBoostThread guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtSuspendThread) { fprintf(stderr, "PENDING_IMPORT __imp__NtSuspendThread guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtQueueApcThread) { fprintf(stderr, "PENDING_IMPORT __imp__NtQueueApcThread guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XexCheckExecutablePrivilege) {
 const uint32_t bit=ctx.r3.u32;
 ctx.r3.u64=0;
 if(bit>=32)return;
 // The verified original XEX header is copied here by the diagnostic loader.
 const uint32_t h=0x60002000;
 auto read=[&](uint32_t a){return(uint32_t(base[a])<<24)|(uint32_t(base[a+1])<<16)|(uint32_t(base[a+2])<<8)|base[a+3];};
 const uint32_t count=read(h+20);
 if(count>128){fprintf(stderr,"INVALID_XEX_HEADER\n");std::_Exit(22);}
 for(uint32_t i=0;i<count;++i){const uint32_t item=h+24+8*i;
  if(read(item)==0x00030000){ctx.r3.u64=(read(item+4)>>bit)&1;return;}}
}
PPC_FUNC(__imp__DbgPrint) { fprintf(stderr, "PENDING_IMPORT __imp__DbgPrint guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp____C_specific_handler) { fprintf(stderr, "PENDING_IMPORT __imp____C_specific_handler guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtYieldExecution) { fprintf(stderr, "PENDING_IMPORT __imp__NtYieldExecution guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ObOpenObjectByName) { fprintf(stderr, "PENDING_IMPORT __imp__ObOpenObjectByName guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlInitAnsiString) { fprintf(stderr, "PENDING_IMPORT __imp__RtlInitAnsiString guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtPulseEvent) { fprintf(stderr, "PENDING_IMPORT __imp__NtPulseEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtSignalAndWaitForSingleObjectEx) { fprintf(stderr, "PENDING_IMPORT __imp__NtSignalAndWaitForSingleObjectEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtWaitForMultipleObjectsEx) { fprintf(stderr, "PENDING_IMPORT __imp__NtWaitForMultipleObjectsEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlRaiseException) {
 const uint32_t a=ctx.r3.u32;
 if(a&&uint64_t(a)+36<=0x100000000ull){
  bool readable=true;for(uint32_t i=0;i<36;++i){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(base+a+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))readable=false;}
  if(readable){auto read=[&](uint32_t o){return(uint32_t(base[a+o])<<24)|(uint32_t(base[a+o+1])<<16)|(uint32_t(base[a+o+2])<<8)|base[a+o+3];};fprintf(stderr,"GUEST_EXCEPTION code=%08X flags=%08X count=%u info=%08X,%08X,%08X,%08X\n",read(0),read(4),read(16),read(20),read(24),read(28),read(32));
   // Legacy debugger-only thread-name notification; never swallow other exceptions.
   if(read(0)==0x406D1388&&read(4)==0&&read(16)==4&&read(20)==0x1000&&read(32)==0){
    const uint32_t name=read(24);char text[256]{};bool terminated=false;
    for(uint32_t i=0;name&&i<sizeof(text)-1&&uint64_t(name)+i<0x100000000ull;++i){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(base+name+i,&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))break;char ch=char(base[name+i]);if(!ch){terminated=true;break;}text[i]=(ch>=32&&ch<127)?ch:'?';}
    if(terminated){fprintf(stderr,"GUEST_THREAD_NAME id=%u name=%s\n",read(28),text);return;}
   }
}
 }
 fprintf(stderr,"PENDING_IMPORT __imp__RtlRaiseException guest_lr=0x%08X\n",uint32_t(ctx.lr));std::_Exit(20);
}
PPC_FUNC(__imp__DbgBreakPoint) { fprintf(stderr, "PENDING_IMPORT __imp__DbgBreakPoint guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlNtStatusToDosError) { fprintf(stderr, "PENDING_IMPORT __imp__RtlNtStatusToDosError guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XexUnloadImage) { fprintf(stderr, "PENDING_IMPORT __imp__XexUnloadImage guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XexUnloadImageAndExitThread) { fprintf(stderr, "PENDING_IMPORT __imp__XexUnloadImageAndExitThread guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XexGetProcedureAddress) { fprintf(stderr, "PENDING_IMPORT __imp__XexGetProcedureAddress guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XexLoadImage) { fprintf(stderr, "PENDING_IMPORT __imp__XexLoadImage guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XexGetModuleSection) { fprintf(stderr, "PENDING_IMPORT __imp__XexGetModuleSection guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtOpenFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtOpenFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtSetInformationFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtSetInformationFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtQueryInformationFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtQueryInformationFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtQueryVolumeInformationFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtQueryVolumeInformationFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtQueryDirectoryFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtQueryDirectoryFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtReadFileScatter) { fprintf(stderr, "PENDING_IMPORT __imp__NtReadFileScatter guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtReadFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtReadFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtCreateFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtCreateFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlUnicodeToMultiByteN) { fprintf(stderr, "PENDING_IMPORT __imp__RtlUnicodeToMultiByteN guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeDelayExecutionThread) { fprintf(stderr, "PENDING_IMPORT __imp__KeDelayExecutionThread guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlImageXexHeaderField) {
 const uint32_t h=ctx.r3.u32, key=ctx.r4.u32;
 auto read=[&](uint32_t a){return (uint32_t(base[a])<<24)|(uint32_t(base[a+1])<<16)|(uint32_t(base[a+2])<<8)|base[a+3];};
 if(h!=0x60002000){fprintf(stderr,"Unsupported XEX header address\n");std::_Exit(22);}
 ctx.r3.u64=0;
 const uint32_t count=read(h+20);
 if(count>128){std::_Exit(22);}
 for(uint32_t i=0;i<count;++i){const uint32_t item=h+24+i*8;
   if(read(item)==key){ctx.r3.u64=(key&255)<=1?item+4:h+read(item+4);break;}}
}
PPC_FUNC(__imp__HalReturnToFirmware) { fprintf(stderr, "PENDING_IMPORT __imp__HalReturnToFirmware guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlFillMemoryUlong) { fprintf(stderr, "PENDING_IMPORT __imp__RtlFillMemoryUlong guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeBugCheckEx) { fprintf(stderr, "PENDING_IMPORT __imp__KeBugCheckEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
// This host runs the title as X_PROCTYPE_USER (Xenia kernel_state.h).
PPC_FUNC(__imp__KeGetCurrentProcessType) { ctx.r3.u64 = 1; }
PPC_FUNC(__imp__RtlCompareMemoryUlong) { fprintf(stderr, "PENDING_IMPORT __imp__RtlCompareMemoryUlong guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlCompareMemory) { fprintf(stderr, "PENDING_IMPORT __imp__RtlCompareMemory guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtQueryVirtualMemory) { fprintf(stderr, "PENDING_IMPORT __imp__NtQueryVirtualMemory guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KiApcNormalRoutineNop) { fprintf(stderr, "PENDING_IMPORT __imp__KiApcNormalRoutineNop guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeAcquireSpinLockAtRaisedIrql) { fprintf(stderr, "PENDING_IMPORT __imp__KeAcquireSpinLockAtRaisedIrql guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KfAcquireSpinLock) { fprintf(stderr, "PENDING_IMPORT __imp__KfAcquireSpinLock guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KfReleaseSpinLock) { fprintf(stderr, "PENDING_IMPORT __imp__KfReleaseSpinLock guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeReleaseSpinLockFromRaisedIrql) { fprintf(stderr, "PENDING_IMPORT __imp__KeReleaseSpinLockFromRaisedIrql guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdIsHSIOTrainingSucceeded) { fprintf(stderr, "PENDING_IMPORT __imp__VdIsHSIOTrainingSucceeded guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdSetDisplayMode) { fprintf(stderr, "PENDING_IMPORT __imp__VdSetDisplayMode guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdGetCurrentDisplayInformation) { fprintf(stderr, "PENDING_IMPORT __imp__VdGetCurrentDisplayInformation guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__sprintf) { fprintf(stderr, "PENDING_IMPORT __imp__sprintf guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp___vsnprintf) { fprintf(stderr, "PENDING_IMPORT __imp___vsnprintf guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdGetCurrentDisplayGamma) { fprintf(stderr, "PENDING_IMPORT __imp__VdGetCurrentDisplayGamma guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdEnableDisableClockGating) { fprintf(stderr, "PENDING_IMPORT __imp__VdEnableDisableClockGating guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdQuerySystemCommandBuffer) { fprintf(stderr, "PENDING_IMPORT __imp__VdQuerySystemCommandBuffer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdSetSystemCommandBuffer) { fprintf(stderr, "PENDING_IMPORT __imp__VdSetSystemCommandBuffer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdSwap) { fprintf(stderr, "PENDING_IMPORT __imp__VdSwap guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeQueryPerformanceFrequency) { ctx.r3.u64 = PPC_TIMEBASE_FREQUENCY; }
PPC_FUNC(__imp__VdPersistDisplay) { fprintf(stderr, "PENDING_IMPORT __imp__VdPersistDisplay guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdGetSystemCommandBuffer) { fprintf(stderr, "PENDING_IMPORT __imp__VdGetSystemCommandBuffer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ObCreateSymbolicLink) { fprintf(stderr, "PENDING_IMPORT __imp__ObCreateSymbolicLink guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ObDeleteSymbolicLink) { fprintf(stderr, "PENDING_IMPORT __imp__ObDeleteSymbolicLink guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdRetrainEDRAMWorker) { fprintf(stderr, "PENDING_IMPORT __imp__VdRetrainEDRAMWorker guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeEnterCriticalRegion) { fprintf(stderr, "PENDING_IMPORT __imp__KeEnterCriticalRegion guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeLeaveCriticalRegion) { fprintf(stderr, "PENDING_IMPORT __imp__KeLeaveCriticalRegion guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdRetrainEDRAM) { fprintf(stderr, "PENDING_IMPORT __imp__VdRetrainEDRAM guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeUnlockL2) { fprintf(stderr, "PENDING_IMPORT __imp__KeUnlockL2 guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeWaitForSingleObject) { fprintf(stderr, "PENDING_IMPORT __imp__KeWaitForSingleObject guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeResetEvent) { fprintf(stderr, "PENDING_IMPORT __imp__KeResetEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeSetEvent) { fprintf(stderr, "PENDING_IMPORT __imp__KeSetEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeLockL2) { fprintf(stderr, "PENDING_IMPORT __imp__KeLockL2 guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdCallGraphicsNotificationRoutines) { fprintf(stderr, "PENDING_IMPORT __imp__VdCallGraphicsNotificationRoutines guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__VdInitializeScalerCommandBuffer) { fprintf(stderr, "PENDING_IMPORT __imp__VdInitializeScalerCommandBuffer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeBugCheck) { fprintf(stderr, "PENDING_IMPORT __imp__KeBugCheck guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserReadProfileSettings) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserReadProfileSettings guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserWriteProfileSettings) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserWriteProfileSettings guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentCreateEx) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentCreateEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentDelete) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentDelete guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentInstall) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentInstall guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentClose) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentClose guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentFlush) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentFlush guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentSetThumbnail) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentSetThumbnail guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentGetThumbnail) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentGetThumbnail guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentGetCreator) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentGetCreator guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentGetLicenseMask) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentGetLicenseMask guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentCreateEnumerator) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentCreateEnumerator guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentCreateDeviceEnumerator) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentCreateDeviceEnumerator guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentGetDeviceState) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentGetDeviceState guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentGetDeviceData) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentGetDeviceData guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentGetDeviceName) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentGetDeviceName guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentLaunchImage) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentLaunchImage guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamContentResolve) { fprintf(stderr, "PENDING_IMPORT __imp__XamContentResolve guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamEnumerate) { fprintf(stderr, "PENDING_IMPORT __imp__XamEnumerate guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMsgCancelIORequest) { fprintf(stderr, "PENDING_IMPORT __imp__XMsgCancelIORequest guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamInputGetCapabilities) { fprintf(stderr, "PENDING_IMPORT __imp__XamInputGetCapabilities guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamInputGetState) { fprintf(stderr, "PENDING_IMPORT __imp__XamInputGetState guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamInputSetState) { fprintf(stderr, "PENDING_IMPORT __imp__XamInputSetState guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamInputGetKeystrokeEx) { fprintf(stderr, "PENDING_IMPORT __imp__XamInputGetKeystrokeEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamTaskShouldExit) { fprintf(stderr, "PENDING_IMPORT __imp__XamTaskShouldExit guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamTaskCloseHandle) { fprintf(stderr, "PENDING_IMPORT __imp__XamTaskCloseHandle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamTaskSchedule) { fprintf(stderr, "PENDING_IMPORT __imp__XamTaskSchedule guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlUnwind) { fprintf(stderr, "PENDING_IMPORT __imp__RtlUnwind guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeEnableFpuExceptions) { fprintf(stderr, "PENDING_IMPORT __imp__KeEnableFpuExceptions guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlCaptureContext) { fprintf(stderr, "PENDING_IMPORT __imp__RtlCaptureContext guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XAudioGetSpeakerConfig) { fprintf(stderr, "PENDING_IMPORT __imp__XAudioGetSpeakerConfig guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAGetOutputBufferWriteOffset) { fprintf(stderr, "PENDING_IMPORT __imp__XMAGetOutputBufferWriteOffset guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAReleaseContext) { fprintf(stderr, "PENDING_IMPORT __imp__XMAReleaseContext guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMASetInputBuffer1) { fprintf(stderr, "PENDING_IMPORT __imp__XMASetInputBuffer1 guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMASetOutputBufferValid) { fprintf(stderr, "PENDING_IMPORT __imp__XMASetOutputBufferValid guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAIsOutputBufferValid) { fprintf(stderr, "PENDING_IMPORT __imp__XMAIsOutputBufferValid guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAIsInputBuffer1Valid) { fprintf(stderr, "PENDING_IMPORT __imp__XMAIsInputBuffer1Valid guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMASetInputBuffer0) { fprintf(stderr, "PENDING_IMPORT __imp__XMASetInputBuffer0 guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMASetOutputBufferReadOffset) { fprintf(stderr, "PENDING_IMPORT __imp__XMASetOutputBufferReadOffset guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMACreateContext) { fprintf(stderr, "PENDING_IMPORT __imp__XMACreateContext guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAInitializeContext) { fprintf(stderr, "PENDING_IMPORT __imp__XMAInitializeContext guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMADisableContext) { fprintf(stderr, "PENDING_IMPORT __imp__XMADisableContext guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMASetInputBuffer0Valid) { fprintf(stderr, "PENDING_IMPORT __imp__XMASetInputBuffer0Valid guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAIsInputBuffer0Valid) { fprintf(stderr, "PENDING_IMPORT __imp__XMAIsInputBuffer0Valid guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAGetOutputBufferReadOffset) { fprintf(stderr, "PENDING_IMPORT __imp__XMAGetOutputBufferReadOffset guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMASetInputBuffer1Valid) { fprintf(stderr, "PENDING_IMPORT __imp__XMASetInputBuffer1Valid guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMAEnableContext) { fprintf(stderr, "PENDING_IMPORT __imp__XMAEnableContext guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMASetInputBufferReadOffset) { fprintf(stderr, "PENDING_IMPORT __imp__XMASetInputBufferReadOffset guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtWriteFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtWriteFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__MmSetAddressProtect) { fprintf(stderr, "PENDING_IMPORT __imp__MmSetAddressProtect guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeQuerySystemTime) { fprintf(stderr, "PENDING_IMPORT __imp__KeQuerySystemTime guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlTimeToTimeFields) { fprintf(stderr, "PENDING_IMPORT __imp__RtlTimeToTimeFields guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlFreeAnsiString) { fprintf(stderr, "PENDING_IMPORT __imp__RtlFreeAnsiString guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlUnicodeStringToAnsiString) { fprintf(stderr, "PENDING_IMPORT __imp__RtlUnicodeStringToAnsiString guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlInitUnicodeString) { fprintf(stderr, "PENDING_IMPORT __imp__RtlInitUnicodeString guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlMultiByteToUnicodeN) { fprintf(stderr, "PENDING_IMPORT __imp__RtlMultiByteToUnicodeN guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtFlushBuffersFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtFlushBuffersFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtQueryFullAttributesFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtQueryFullAttributesFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__StfsControlDevice) { fprintf(stderr, "PENDING_IMPORT __imp__StfsControlDevice guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__StfsCreateDevice) { fprintf(stderr, "PENDING_IMPORT __imp__StfsCreateDevice guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XeKeysConsoleSignatureVerification) { fprintf(stderr, "PENDING_IMPORT __imp__XeKeysConsoleSignatureVerification guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XeCryptSha) { fprintf(stderr, "PENDING_IMPORT __imp__XeCryptSha guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XeKeysConsolePrivateKeySign) { fprintf(stderr, "PENDING_IMPORT __imp__XeKeysConsolePrivateKeySign guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp___snprintf) { fprintf(stderr, "PENDING_IMPORT __imp___snprintf guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoDismountVolume) { fprintf(stderr, "PENDING_IMPORT __imp__IoDismountVolume guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__FscSetCacheElementCount) { fprintf(stderr, "PENDING_IMPORT __imp__FscSetCacheElementCount guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__FscGetCacheElementCount) { fprintf(stderr, "PENDING_IMPORT __imp__FscGetCacheElementCount guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlTimeFieldsToTime) { fprintf(stderr, "PENDING_IMPORT __imp__RtlTimeFieldsToTime guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoInvalidDeviceRequest) { fprintf(stderr, "PENDING_IMPORT __imp__IoInvalidDeviceRequest guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ObReferenceObject) { fprintf(stderr, "PENDING_IMPORT __imp__ObReferenceObject guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoCreateDevice) { fprintf(stderr, "PENDING_IMPORT __imp__IoCreateDevice guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoDeleteDevice) { fprintf(stderr, "PENDING_IMPORT __imp__IoDeleteDevice guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ExAllocatePoolTypeWithTag) { fprintf(stderr, "PENDING_IMPORT __imp__ExAllocatePoolTypeWithTag guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ExFreePool) { fprintf(stderr, "PENDING_IMPORT __imp__ExFreePool guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlCompareStringN) { fprintf(stderr, "PENDING_IMPORT __imp__RtlCompareStringN guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoCompleteRequest) { fprintf(stderr, "PENDING_IMPORT __imp__IoCompleteRequest guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__RtlUpcaseUnicodeChar) { fprintf(stderr, "PENDING_IMPORT __imp__RtlUpcaseUnicodeChar guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ObIsTitleObject) { fprintf(stderr, "PENDING_IMPORT __imp__ObIsTitleObject guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoCheckShareAccess) { fprintf(stderr, "PENDING_IMPORT __imp__IoCheckShareAccess guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoSetShareAccess) { fprintf(stderr, "PENDING_IMPORT __imp__IoSetShareAccess guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoRemoveShareAccess) { fprintf(stderr, "PENDING_IMPORT __imp__IoRemoveShareAccess guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__IoDismountVolumeByFileHandle) { fprintf(stderr, "PENDING_IMPORT __imp__IoDismountVolumeByFileHandle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtDeviceIoControlFile) { fprintf(stderr, "PENDING_IMPORT __imp__NtDeviceIoControlFile guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ExAllocatePool) { fprintf(stderr, "PENDING_IMPORT __imp__ExAllocatePool guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KePulseEvent) { fprintf(stderr, "PENDING_IMPORT __imp__KePulseEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeWaitForMultipleObjects) { fprintf(stderr, "PENDING_IMPORT __imp__KeWaitForMultipleObjects guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XAudioQueryDriverPerformance) { fprintf(stderr, "PENDING_IMPORT __imp__XAudioQueryDriverPerformance guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeRaiseIrqlToDpcLevel) { fprintf(stderr, "PENDING_IMPORT __imp__KeRaiseIrqlToDpcLevel guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KfLowerIrql) { fprintf(stderr, "PENDING_IMPORT __imp__KfLowerIrql guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeTryToAcquireSpinLockAtRaisedIrql) { fprintf(stderr, "PENDING_IMPORT __imp__KeTryToAcquireSpinLockAtRaisedIrql guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__ExAllocatePoolWithTag) { fprintf(stderr, "PENDING_IMPORT __imp__ExAllocatePoolWithTag guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeReleaseSemaphore) { fprintf(stderr, "PENDING_IMPORT __imp__KeReleaseSemaphore guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XAudioGetVoiceCategoryVolume) { fprintf(stderr, "PENDING_IMPORT __imp__XAudioGetVoiceCategoryVolume guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XAudioGetVoiceCategoryVolumeChangeMask) { fprintf(stderr, "PENDING_IMPORT __imp__XAudioGetVoiceCategoryVolumeChangeMask guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeResumeThread) { fprintf(stderr, "PENDING_IMPORT __imp__KeResumeThread guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__KeInitializeSemaphore) { fprintf(stderr, "PENDING_IMPORT __imp__KeInitializeSemaphore guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XAudioRegisterRenderDriverClient) { fprintf(stderr, "PENDING_IMPORT __imp__XAudioRegisterRenderDriverClient guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XAudioUnregisterRenderDriverClient) { fprintf(stderr, "PENDING_IMPORT __imp__XAudioUnregisterRenderDriverClient guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XAudioSubmitRenderDriverFrame) { fprintf(stderr, "PENDING_IMPORT __imp__XAudioSubmitRenderDriverFrame guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamCreateEnumeratorHandle) { fprintf(stderr, "PENDING_IMPORT __imp__XamCreateEnumeratorHandle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMsgStartIORequestEx) { fprintf(stderr, "PENDING_IMPORT __imp__XMsgStartIORequestEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMsgInProcessCall) { fprintf(stderr, "PENDING_IMPORT __imp__XMsgInProcessCall guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamGetPrivateEnumStructureFromHandle) { fprintf(stderr, "PENDING_IMPORT __imp__XamGetPrivateEnumStructureFromHandle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XMsgSystemProcessCall) { fprintf(stderr, "PENDING_IMPORT __imp__XMsgSystemProcessCall guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSAStartup) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSAStartup guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSACleanup) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSACleanup guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_socket) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_socket guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_closesocket) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_closesocket guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_shutdown) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_shutdown guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_ioctlsocket) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_ioctlsocket guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_setsockopt) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_setsockopt guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_getsockopt) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_getsockopt guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_getsockname) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_getsockname guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_getpeername) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_getpeername guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_bind) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_bind guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_connect) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_connect guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_listen) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_listen guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_accept) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_accept guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_select) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_select guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSAGetOverlappedResult) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSAGetOverlappedResult guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSACancelOverlappedIO) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSACancelOverlappedIO guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_recv) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_recv guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSARecv) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSARecv guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_recvfrom) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_recvfrom guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSARecvFrom) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSARecvFrom guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_send) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_send guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSASend) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSASend guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_sendto) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_sendto guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSASendTo) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSASendTo guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSAEventSelect) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSAEventSelect guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_inet_addr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_inet_addr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSAGetLastError) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSAGetLastError guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSASetLastError) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSASetLastError guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSACreateEvent) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSACreateEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSACloseEvent) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSACloseEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSASetEvent) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSASetEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSAResetEvent) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSAResetEvent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_WSAWaitForMultipleEvents) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_WSAWaitForMultipleEvents guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll___WSAFDIsSet) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll___WSAFDIsSet guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetStartup) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetStartup guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetCleanup) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetCleanup guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetRandom) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetRandom guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetCreateKey) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetCreateKey guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetRegisterKey) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetRegisterKey guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetUnregisterKey) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetUnregisterKey guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetXnAddrToInAddr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetXnAddrToInAddr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetServerToInAddr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetServerToInAddr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetTsAddrToInAddr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetTsAddrToInAddr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetInAddrToXnAddr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetInAddrToXnAddr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetInAddrToServer) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetInAddrToServer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetInAddrToString) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetInAddrToString guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetUnregisterInAddr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetUnregisterInAddr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetXnAddrToMachineId) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetXnAddrToMachineId guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetConnect) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetConnect guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetGetConnectStatus) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetGetConnectStatus guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetDnsLookup) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetDnsLookup guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetDnsRelease) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetDnsRelease guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetQosListen) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetQosListen guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetQosLookup) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetQosLookup guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetQosServiceLookup) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetQosServiceLookup guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetQosRelease) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetQosRelease guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetGetTitleXnAddr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetGetTitleXnAddr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetGetDebugXnAddr) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetGetDebugXnAddr guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetGetEthernetLinkStatus) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetGetEthernetLinkStatus guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetGetBroadcastVersionStatus) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetGetBroadcastVersionStatus guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetQosGetListenStats) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetQosGetListenStats guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetGetOpt) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetGetOpt guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NetDll_XNetSetOpt) { fprintf(stderr, "PENDING_IMPORT __imp__NetDll_XNetSetOpt guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamAlloc) { fprintf(stderr, "PENDING_IMPORT __imp__XamAlloc guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamFree) { fprintf(stderr, "PENDING_IMPORT __imp__XamFree guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XNetLogonGetMachineID) { fprintf(stderr, "PENDING_IMPORT __imp__XNetLogonGetMachineID guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XNetLogonGetTitleID) { fprintf(stderr, "PENDING_IMPORT __imp__XNetLogonGetTitleID guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserGetMembershipTierFromXUID) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserGetMembershipTierFromXUID guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamUserGetOnlineCountryFromXUID) { fprintf(stderr, "PENDING_IMPORT __imp__XamUserGetOnlineCountryFromXUID guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XexGetModuleHandle) { fprintf(stderr, "PENDING_IMPORT __imp__XexGetModuleHandle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtCreateMutant) { fprintf(stderr, "PENDING_IMPORT __imp__NtCreateMutant guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtReleaseMutant) { fprintf(stderr, "PENDING_IMPORT __imp__NtReleaseMutant guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamSessionRefObjByHandle) { fprintf(stderr, "PENDING_IMPORT __imp__XamSessionRefObjByHandle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamSessionCreateHandle) { fprintf(stderr, "PENDING_IMPORT __imp__XamSessionCreateHandle guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamVoiceHeadsetPresent) { fprintf(stderr, "PENDING_IMPORT __imp__XamVoiceHeadsetPresent guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamVoiceClose) { fprintf(stderr, "PENDING_IMPORT __imp__XamVoiceClose guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamVoiceSubmitPacket) { fprintf(stderr, "PENDING_IMPORT __imp__XamVoiceSubmitPacket guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__XamVoiceCreate) { fprintf(stderr, "PENDING_IMPORT __imp__XamVoiceCreate guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtCancelTimer) { fprintf(stderr, "PENDING_IMPORT __imp__NtCancelTimer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtSetTimerEx) { fprintf(stderr, "PENDING_IMPORT __imp__NtSetTimerEx guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
PPC_FUNC(__imp__NtCreateTimer) { fprintf(stderr, "PENDING_IMPORT __imp__NtCreateTimer guest_lr=0x%08X\n", uint32_t(ctx.lr)); std::_Exit(20); }
