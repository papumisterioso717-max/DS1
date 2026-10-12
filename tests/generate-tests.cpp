#include "recompiler.h"
#include <fstream>
static void emit(Recompiler&r,const char* name,const std::vector<uint32_t>& words,uint32_t start=0x1000){
 r.image={};r.image.base=start;r.image.size=uint32_t(words.size()*4);r.image.data=std::make_unique<uint8_t[]>(r.image.size);
 for(size_t i=0;i<words.size();i++){auto w=words[i];for(int j=0;j<4;j++)r.image.data[i*4+j]=uint8_t(w>>(24-j*8));}
 r.image.Map(".text",0,r.image.size,SectionFlags_Code,r.image.data.get());r.image.symbols.emplace(name,start,r.image.size,Symbol_Function);
 if(!r.Recompile(Function{start,r.image.size}))throw std::runtime_error(name);
}
int main(int argc,char**argv){
 Recompiler r;r.out="#include \"ppc_context.h\"\nPPC_EXTERN_FUNC(shared_exit);\n";
 struct Op{const char*name;uint32_t base;bool scalar;};
 const Op ops[]={
 {"vaddsws",(4u<<26)|896,false},{"vadduhs",(4u<<26)|576,false},{"vcmpgtuw",(4u<<26)|646,false},{"vminsw",(4u<<26)|898,false},
 {"vpkswss128",(5u<<26)|640,false},{"vnor128",(5u<<26)|656,false},{"vpkuwus128",(5u<<26)|960,false},{"vsro128",(5u<<26)|976,false}};
 for(auto&op:ops)for(unsigned d:{3u,4u,5u}){std::string n=std::string("test_")+op.name+"_"+std::to_string(d);emit(r,n.c_str(),{op.base|(d<<21)|(4<<16)|(5<<11),0x4e800020});}
 emit(r,"test_vcmpgtuw_record",{(4u<<26)|646|(1<<10)|(3<<21)|(4<<16)|(5<<11),0x4e800020});
 for(unsigned sh:{0u,2u,31u})for(unsigned d:{3u,5u}){std::string n="test_convert_"+std::to_string(sh)+"_"+std::to_string(d);emit(r,n.c_str(),{(6u<<26)|624|(d<<21)|(sh<<16)|(5<<11),0x4e800020});}
 emit(r,"test_eqv",{(31u<<26)|(284<<1)|(4<<21)|(3<<16)|(5<<11),0x4e800020});
 emit(r,"test_eqv_record",{(31u<<26)|(284<<1)|1|(4<<21)|(3<<16)|(5<<11),0x4e800020});
 emit(r,"test_stfsu",{(53u<<26)|(3<<21)|(4<<16)|8,0x4e800020});
 emit(r,"test_lvebx",{(31u<<26)|(7<<1)|(3<<21)|(4<<16)|(5<<11),0x4e800020});
 emit(r,"test_lvebx_ra0",{(31u<<26)|(7<<1)|(3<<21)|(5<<11),0x4e800020});
 emit(r,"test_dcbst",{(31u<<26)|(54<<1)|(4<<16)|(5<<11),0x4e800020});
 r.config.computedJumpAddresses.insert(0x1004);
 emit(r,"test_computed",{0x60000000,0x4e800420,0x38600007,0x4e800020,0x38600009,0x4e800020});
 r.config.computedJumpAddresses.clear();
 // A separately emitted function supplies a known external switch target.
 r.image={};r.image.base=0x1000;r.image.size=8;r.image.data=std::make_unique<uint8_t[]>(8);const uint8_t bytes[]={0x60,0,0,0,0x4e,0x80,4,0x20};memcpy(r.image.data.get(),bytes,8);
 r.image.Map(".text",0,8,SectionFlags_Code,r.image.data.get());r.image.symbols.emplace("test_external",0x1000,8,Symbol_Function);r.image.symbols.emplace("shared_exit",0x1100,8,Symbol_Function);
 r.config.switchTables.emplace(0x1000,RecompilerSwitchTable{3,{0x1100}});r.Recompile(Function{0x1000,8});
 std::ofstream(argv[1])<<r.out;
}
