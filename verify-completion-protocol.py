"""Probe locally recompiled completion producer/readers with synthetic memory."""
import argparse,os,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--toolchain',type=Path,required=True);p.add_argument('--xenonrecomp',type=Path,required=True);args=p.parse_args()
root=Path(__file__).resolve().parent
source=(root/'ppc/ppc_recomp.70.cpp').read_text()
bodies=[]
for symbol in ('823EBF80','823EAEA8','823EB450'):
 start=source.index('PPC_FUNC_IMPL(__imp__sub_'+symbol+') {');end=source.index('\n}\n',start)+3
 body=source[start:end]
 if 'sub_' in body.split('{',1)[1]:raise ValueError('Probe expects reviewed leaf routines')
 bodies.append(body)
code='#include "ppc_recomp_shared.h"\n#include <cstdio>\n#include <cstring>\n'+''.join(bodies)+r'''
int main(){
 unsigned char memory[65536]{};auto*base=memory;PPCContext ctx{};
 const uint32_t object=0x1000, completion=0x8000, output=0x9000;
 PPC_STORE_U32(object+10896,completion);PPC_STORE_U32(object+48,0xBF8E00D4);
 PPC_STORE_U32(object+10908,3);PPC_STORE_U32(object+14920,0);
 PPC_STORE_U32(completion,1);PPC_STORE_U32(completion+4,0x11223346);
 ctx.r1.u32=0xF000;ctx.r3.u32=object;ctx.r4.u32=output-4;
 __imp__sub_823EBF80(ctx,base);
 const uint32_t expected[]={0xC0003B00,0x7FFF,0xC0025800,3,0x8006,0xBF8E00D4,0xC0025800,3,0x8002,3};
 for(unsigned i=0;i<10;++i)if(PPC_LOAD_U32(output+i*4)!=expected[i])return 1;
 if(ctx.r3.u32!=output+36 || PPC_LOAD_U32(object+10908)!=5)return 2;
 if(PPC_LOAD_U32(completion)!=1 || PPC_LOAD_U32(completion+4)!=0x11223346)return 3;
 ctx.r3.u32=object;ctx.r4.u32=3;__imp__sub_823EAEA8(ctx,base);
 if(ctx.r3.u32!=1)return 4;
 // Synthetic completion only, not a runtime GPU event.
 PPC_STORE_U32(completion,3);
 ctx.r3.u32=object;ctx.r4.u32=3;__imp__sub_823EAEA8(ctx,base);
 if(ctx.r3.u32!=0)return 5;
 ctx.r3.u32=object;ctx.r4.u32=0xBF8E00D4;ctx.r5.u32=0;
 __imp__sub_823EB450(ctx,base);if(ctx.r3.u32!=1)return 6;
 PPC_STORE_U32(completion+4,0xBF8E00D4);
 ctx.r3.u32=object;ctx.r4.u32=0xBF8E00D4;ctx.r5.u32=0;
 __imp__sub_823EB450(ctx,base);if(ctx.r3.u32!=0)return 7;
 puts("PASS actual completion producer: invalidation + two events, sequence increments by two, no premature completion write");
 puts("PASS actual sequence and tagged-pointer readers with synthetic completion values");
}
'''
build=root/'build';build.mkdir(exist_ok=True);src=build/'completion-probe.cpp';src.write_text(code);exe=build/'CompletionProbe.exe'
env=os.environ.copy();env['PATH']=str(args.toolchain)+os.pathsep+env['PATH']
subprocess.run([str(v) for v in [args.toolchain/'clang++.exe','-std=c++17','-O0','-mavx2','-msse4.1','-I',root/'ppc','-I',args.xenonrecomp/'thirdparty/simde','-I',args.xenonrecomp/'XenonUtils',src,'-o',exe]],check=True,env=env)
subprocess.run([str(exe)],check=True,env=env)
