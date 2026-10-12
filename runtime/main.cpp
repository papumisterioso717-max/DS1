#define NOMINMAX
#include <windows.h>
#include "ppc_recomp_shared.h"
#include <image.h>
#include "tls.h"
#include "physical.h"
#include "objects.h"
#include "threads.h"
#include "timestamp.h"
#include <file.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
extern PPCFuncMapping PPCFuncMappings[];

static uint8_t* activeBase=nullptr;
static LONG CALLBACK fault(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==0xE06D7363||e->ExceptionRecord->ExceptionCode==0x20474343)return EXCEPTION_CONTINUE_SEARCH;
    fprintf(stderr,"GUEST_DIAGNOSTIC_FAULT code=0x%08lX guest_lr=0x%08X\n",e->ExceptionRecord->ExceptionCode,diagnosticContext?uint32_t(diagnosticContext->lr):0);
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION&&e->ExceptionRecord->NumberParameters>=2){
      const auto address=e->ExceptionRecord->ExceptionInformation[1];
      fprintf(stderr,"FAULT_ACCESS kind=%llu host=%p guest=0x%llX\n",(unsigned long long)e->ExceptionRecord->ExceptionInformation[0],(void*)address,(unsigned long long)(address-reinterpret_cast<uintptr_t>(activeBase)));
    }
    std::_Exit(21);
}
int main(int argc,char**argv){
 if(argc!=2){fprintf(stderr,"Usage: DeadSpaceDiagnostic.exe path-to-default.xex\nDiagnostic host only: missing Xbox services stop execution.\n");return 2;}
 auto file=LoadFile(argv[1]);if(file.size()<24||memcmp(file.data(),"XEX2",4)){fprintf(stderr,"Invalid or unreadable XEX2\n");return 3;}
 auto image=Image::ParseImage(file.data(),file.size());
 if(image.base!=PPC_IMAGE_BASE||image.size!=PPC_IMAGE_SIZE||image.entry_point!=0x82894210){fprintf(stderr,"This host requires the configured Dead Space executable.\n");return 4;}
 auto*base=reserveGuestMemory();
 if(!base){fprintf(stderr,"Unable to reserve guest address space: %lu\n",GetLastError());return 5;}
 auto commit=[&](size_t address,size_t size){if(!VirtualAlloc(base+address,size,MEM_COMMIT,PAGE_READWRITE)){fprintf(stderr,"Unable to commit guest range: %lu\n",GetLastError());std::_Exit(6);}};
 commit(PPC_IMAGE_BASE,PPC_IMAGE_SIZE);memcpy(base+PPC_IMAGE_BASE,image.data.get(),PPC_IMAGE_SIZE);
 commit(PPC_IMAGE_BASE+PPC_IMAGE_SIZE,PPC_CODE_SIZE*2);
 size_t mapped=0;for(auto*m=PPCFuncMappings;m->host;m++){if(m->guest<PPC_CODE_BASE||m->guest>=PPC_CODE_BASE+PPC_CODE_SIZE){fprintf(stderr,"Invalid function mapping\n");return 7;}PPC_LOOKUP_FUNC(base,m->guest)=m->host;++mapped;}
 // Minimal executable-module record. Guest pointers and fields are big endian.
 // Layout: xenia/src/xenia/kernel/xmodule.h X_LDR_DATA_TABLE_ENTRY.
 commit(0x60000000,0x10000);
 auto put=[&](uint32_t a,uint32_t v){base[a]=v>>24;base[a+1]=v>>16;base[a+2]=v>>8;base[a+3]=v;};
 const uint32_t headerSize=(uint32_t(file[8])<<24)|(uint32_t(file[9])<<16)|(uint32_t(file[10])<<8)|file[11];
 if(headerSize>0xE000||headerSize>file.size()){fprintf(stderr,"Unexpected XEX header size\n");return 8;}
 memcpy(base+0x60002000,file.data(),headerSize);
 put(0x8200087C,0x60000000); // imported address of module-handle variable
 put(0x60000000,0x60001000); // variable contains module record pointer
 put(0x60001018,PPC_IMAGE_BASE);put(0x6000101C,PPC_IMAGE_BASE);
 put(0x60001020,PPC_IMAGE_SIZE);put(0x60001038,PPC_IMAGE_SIZE);
 put(0x6000103C,0x82894210);put(0x60001058,0x60002000);
 commit(0x70000000,0x800000);
 // Main-thread identity and the verified KPCR/KTHREAD stack fields.
 // Layout reference: Xenia kernel/xthread.h. TLS and scheduling remain pending.
 commit(0x60010000,0x2000);
 put(0x60010030,0x60010000);put(0x60010070,0x70800000);put(0x60010074,0x70000000);
 put(0x60010100,0x60011000);
 put(0x6001105C,0x70800000);put(0x60011060,0x70000000);
 put(0x600110D0,0x70000000);put(0x6001114C,1);put(0x60011150,0x82894210);
 // Load the TLS descriptor from this executable rather than inventing values.
 auto readHeader=[&](uint32_t a){if(uint64_t(a)+4>headerSize){fprintf(stderr,"Invalid TLS header\n");std::_Exit(8);}return(uint32_t(file[a])<<24)|(uint32_t(file[a+1])<<16)|(uint32_t(file[a+2])<<8)|file[a+3];};
 uint32_t tlsOffset=0;
 for(uint32_t i=0;i<readHeader(20);++i)if(readHeader(24+8*i)==0x20104)tlsOffset=readHeader(28+8*i);
 if(!tlsOffset){fprintf(stderr,"Missing TLS descriptor\n");return 8;}
 const uint32_t slots=readHeader(tlsOffset),raw=readHeader(tlsOffset+4),dataSize=readHeader(tlsOffset+8),rawSize=readHeader(tlsOffset+12);
 if(uint64_t(dataSize)+uint64_t(slots)*4>0x10000||raw<PPC_IMAGE_BASE||uint64_t(raw)+rawSize>uint64_t(PPC_IMAGE_BASE)+PPC_IMAGE_SIZE){fprintf(stderr,"Unsupported TLS layout\n");return 8;}
 commit(0x60020000,0x10000);
 registerGuestTls(base,0x60010000,0x60020000,slots,dataSize,raw,rawSize);
 put(0x60011068,0x60020000);
 printf("TLS initialized: %u slots, %u static bytes.\n",slots,dataSize);
 PPCContext ctx{};ctx.r13.u64=0x60010000;ctx.r1.u64=0x707FFF00;ctx.fpscr.loadFromHost();diagnosticContext=&ctx;
 activeBase=base;
 AddVectoredExceptionHandler(1,fault);
 printf("Loaded XEX. %zu native function mappings. Entering guest 0x82894210.\n",mapped);fflush(stdout);
 // This diagnostic stage stops on the first unimplemented system service.
 // No speculative values are supplied for Xbox kernel state or TLS.
 commit(0x60030000,0x1000);
 put(0x82000944,0x60030000); // imported ExThreadObjectType identity
 uint32_t defaultStack=0;
 for(uint32_t i=0;i<readHeader(20);++i)if(readHeader(24+8*i)==0x20200)defaultStack=readHeader(28+8*i);
 configureGuestThreads(base,slots,dataSize,raw,rawSize,defaultStack);
 commit(0x60040000,0x1000);
 if(!startGuestTimestamp(base,0x820008E0,0x60040000)){fprintf(stderr,"Timestamp initialization failed\n");return 9;}
 registerCurrentGuestThread(base,ctx.r13.u32);
 _xstart(ctx,base);
 printf("Guest entry returned. This does not imply a playable game.\n");
 std::fflush(stdout);std::_Exit(0); // Worker threads may still own guest memory.
}
