"""Read-only Xenos CF analysis. Does not execute or prove shaders side-effect-free.
Layout reference: xenia/src/xenia/gpu/ucode.h.
"""
import json, sys
from pathlib import Path

def decode_alu(words):
    """Decode output routing; recognize only unpredicated MAX/RETAIN_PREV.

    Export overlap writes one, and scalar_dest_rel enables zero fill.
    This is an inspection tool, not a shader interpreter.
    """
    if len(words)!=3 or any(type(w) is not int or not 0<=w<=0xffffffff for w in words):
        raise ValueError('ALU requires three unsigned words')
    a,b,c=words
    export=bool(a&0x8000); vm=(a>>16)&15; sm=(a>>20)&15
    vector=(c>>24)&31; scalar=a>>26
    sources=[]
    for shift,swshift,negshift,selshift in ((16,16,26,31),(8,8,25,30),(0,0,24,29)):
        reg=(c>>shift)&255; temp=bool(c&(1<<selshift)); sw=(b>>swshift)&255
        sources.append({'kind':'temporary' if temp else 'constant','index':reg&63 if temp else reg,
            'relative':bool(reg&64) if temp else None,'absolute':bool(reg&128) if temp else bool(a&128),
            'negate':bool(b&(1<<negshift)), 'swizzle':''.join('xyzw'[((sw>>(2*i))+i)&3] for i in range(4))})
    supported=vector==2 and scalar==50 and not (b&(1<<28)) and all(s['kind']=='temporary' and not s['relative'] for s in sources[:2])
    return {'vector_operation':'MAX' if vector==2 else 'UNKNOWN', 'vector_opcode':vector,
        'scalar_operation':'RETAIN_PREV' if scalar==50 else 'UNKNOWN','scalar_opcode':scalar,
        'recognized_semantics':supported,'predicated':bool(b&(1<<28)),
        'export':export,'vector_destination':a&63,'scalar_destination':a&63 if export else (a>>8)&63,
        'vector_destination_relative':not export and bool(a&64),
        'scalar_destination_relative':not export and bool(a&0x4000),
        'vector_clamp':bool(a&(1<<24)),'scalar_clamp':bool(a&(1<<25)),
        'vector_result_mask':vm&~sm if export else vm,'scalar_result_mask':sm&~vm if export else sm,
        'constant_zero_mask':(~(vm|sm)&15) if export and a&0x4000 else 0,
        'constant_one_mask':vm&sm if export else 0,'sources':sources}

def analyze(words):
    if not words or len(words)%3: raise ValueError('Shader must contain complete 96-bit instruction groups')
    if any(not isinstance(w,int) or w<0 or w>0xffffffff for w in words): raise ValueError('Invalid word')
    result=[]; boundary=len(words)//3; ended=False
    for group in range(len(words)//3):
        if group>=boundary: break
        a,b,c=words[group*3:group*3+3]
        for half,(lo,hi) in enumerate(((a,b&65535),((b>>16)|((c&65535)<<16),c>>16))):
            op=hi>>12; item={'cf_index':group*2+half,'opcode':op}
            if op==0: item['operation']='NOP'
            elif op==12:
                item.update(operation='ALLOC',allocation_type=(hi>>9)&3,encoded_size=lo&7)
            elif op in (1,2):
                address=lo&4095; count=(lo>>12)&7; sequence=(lo>>16)&4095
                if address<group+1 or address+count>len(words)//3: raise ValueError('EXEC range outside instruction region')
                boundary=min(boundary,address)
                item.update(operation='EXEC_END' if op==2 else 'EXEC',address=address,count=count,
                    instructions=[{'slot':address+i,'kind':'FETCH' if (sequence>>(i*2))&1 else 'ALU','serialize':bool((sequence>>(i*2+1))&1)} for i in range(count)])
            else: raise ValueError('Unsupported control flow opcode '+str(op))
            result.append(item)
            if op==2: ended=True; break
        if ended: break
    if not ended: raise ValueError('No unconditional EXEC_END found')
    cf_groups=(len(result)+1)//2
    if any(item.get('address',cf_groups)<cf_groups for item in result): raise ValueError('EXEC overlaps control flow')
    alu=[]; fetch=False
    for item in result:
        for instruction in item.get('instructions',[]):
            if instruction['kind']=='FETCH': fetch=True; continue
            slot=instruction['slot']
            alu.append({'slot':slot, **decode_alu(words[slot*3:slot*3+3])})
    return {'control_flow':result,'alu':alu,
        'instruction_semantics_analyzed':bool(alu) and not fetch and all(i['recognized_semantics'] for i in alu),
        'safe_to_skip_draw':False}

if __name__=='__main__':
    capture=json.loads(Path(sys.argv[1]).read_text())
    result={'scope':'linear flow and limited MAX/RETAIN_PREV output routing; no execution or draw safety proof','shaders':[analyze([int(w,16) for w in words]) for words in capture['shaders']]}
    print(json.dumps(result,indent=2))
