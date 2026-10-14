"""Run an isolated recompiled command builder from locally generated sources.
Requires the local PPC sources and compiler; embeds no game code in this script.
"""
import argparse, subprocess, os
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('--toolchain',type=Path,required=True)
parser.add_argument('--xenonrecomp',type=Path,required=True)
args=parser.parse_args()
root=Path(__file__).resolve().parent
source=(root/'ppc/ppc_recomp.71.cpp').read_text()
start=source.index('PPC_FUNC_IMPL(__imp__sub_823EDA70) {')
end=source.index('\n}\n',start)+3
body=source[start:end]
start_config=source.index('PPC_FUNC_IMPL(__imp__sub_823EDA10) {')
end_config=source.index('\n}\n',start_config)+3
body_config=source[start_config:end_config]
# This probe is restricted to the reviewed leaf routine with no external calls.
if 'sub_' in body.split('{',1)[1]: raise ValueError('Unexpected external call in builder')
harness=r'''
#include "ppc_recomp_shared.h"
#include <cstdio>
#include <cstring>
'''+body+body_config+r'''
int main(){
 unsigned char memory[1024]; memset(memory,0xA5,sizeof(memory));
 auto* base=memory; PPCContext ctx{}; ctx.r4.u32=128;
 __imp__sub_823EDA70(ctx,base);
 if(ctx.r3.u32!=48)return 1;
 for(unsigned i=0;i<24;++i){
  if(PPC_LOAD_U32(128+i*8)!=0xC0003600 || PPC_LOAD_U32(132+i*8)!=0x00010081)return 2;
 }
 for(unsigned i=0;i<sizeof(memory);++i)if((i<128||i>=320)&&memory[i]!=0xA5)return 3;
 memset(memory,0xA5,sizeof(memory)); ctx=PPCContext{}; ctx.r4.u32=64; ctx.r5.u32=128;
 __imp__sub_823EDA10(ctx,base);
 if(ctx.r3.u32!=13)return 4;
 if(PPC_LOAD_U32(128)!=0x5C8 || PPC_LOAD_U32(132)!=0x20000 || PPC_LOAD_U32(136)!=0x78D00)return 5;
 for(unsigned i=0;i<8;++i)if(PPC_LOAD_U32(140+i*4)!=0x40401)return 6;
 if(PPC_LOAD_U32(172)!=0xD00 || PPC_LOAD_U32(176)!=0x40400)return 7;
 for(unsigned i=0;i<sizeof(memory);++i)if((i<128||i>=180)&&memory[i]!=0xA5)return 8;
 puts("PASS actual GPR configuration builder: 13 matching words, surrounding memory preserved");
 puts("PASS actual recompiled builder: 24 matching packets, 48 words, surrounding memory preserved");
}
'''
build=root/'build';build.mkdir(exist_ok=True)
p=build/'draw-builder-probe.cpp';p.write_text(harness)
exe=build/'DrawBuilderProbe.exe'
env=os.environ.copy();env['PATH']=str(args.toolchain)+os.pathsep+env['PATH']
command=[args.toolchain/'clang++.exe','-std=c++17','-O0','-mavx2','-msse4.1','-I',root/'ppc','-I',args.xenonrecomp/'thirdparty/simde','-I',args.xenonrecomp/'XenonUtils',p,'-o',exe]
subprocess.run([str(v) for v in command],env=env,check=True)
subprocess.run([str(exe)],env=env,check=True)
