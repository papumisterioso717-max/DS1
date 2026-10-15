#include "../runtime/gpu-no-output.h"
#include <cstdio>
#include <cstdlib>
int main(){
 unsigned n=0;auto check=[&](bool v){++n;if(!v){printf("FAIL no-output %u\n",n);exit(1);}};
 const std::vector<uint32_t> shader={0x1001,0x2200,0,(50u<<26)|0xC000,0,(7u<<29)|(2u<<24)};
 check(diagnosticShaderHasOnlyOrdinaryExports(shader,false));
 check(diagnosticShaderHasOnlyOrdinaryExports(shader,true));
 for(auto change:std::vector<std::pair<unsigned,uint32_t>>{{0,0x11001},{0,0x1000},{0,0x1009},{1,0x3200},{1,0x1200},{3,(50u<<26)|0xC020},{3,(50u<<26)|0xC03D},{3,(50u<<26)|0x10000},{4,1},{5,(7u<<29)|(3u<<24)},{5,(7u<<29)|(2u<<24)|0x400000}}){
  auto bad=shader;bad[change.first]=change.second;check(!diagnosticShaderHasOnlyOrdinaryExports(bad,true));
 }
 check(!diagnosticShaderHasOnlyOrdinaryExports({},false));
 DiagnosticGpuState state{};state.modeControl=4;state.sqProgramControl=0x1000000E;
 state.shaderWords[0]=shader;state.shaderWords[1]=shader;
 check(diagnosticNoOutputDraw(state,0x10081,1));
 for(uint32_t* field:{&state.surfaceInfo,&state.colorMask,&state.depthControl,&state.hizControl,&state.vizQueryConfig,&state.sqContextMisc}){
  *field=1;check(!diagnosticNoOutputDraw(state,0x10081,1));*field=0;
 }
 check(!diagnosticNoOutputDraw(state,0x10081,2));
 check(!diagnosticNoOutputDraw(state,0x20081,1));
 printf("PASS %u no-output classification checks\n",n);
}
