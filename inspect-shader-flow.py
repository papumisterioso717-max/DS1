"""Read-only Xenos CF analysis. Does not execute or prove shaders side-effect-free.
Layout reference: xenia/src/xenia/gpu/ucode.h.
"""
import json, sys
from pathlib import Path

def decode_fetch(words):
    """Inspect fetch bitfields only; mini-fetch addresses need preceding state.

    Layout: Xenia ucode.h VertexFetchInstruction. No memory is accessed.
    """
    if len(words)!=3 or any(type(w) is not int or not 0<=w<=0xffffffff for w in words):
        raise ValueError('FETCH requires three unsigned words')
    a,b,c=words
    opcode=a&31
    if opcode:
        return {'opcode':opcode,'operation':'UNSUPPORTED_FETCH','decoded':False}
    def signed(value,bits):
        return value-(1<<bits) if value&(1<<(bits-1)) else value
    mini=bool(b&(1<<30))
    return {'opcode':0,'operation':'VERTEX_FETCH','decoded':True,
        'mini':mini,'destination':(a>>12)&63,'destination_relative':bool(a&(1<<18)),
        'destination_swizzle':['xyzw01?_'[(b>>(i*3))&7] for i in range(4)],
        'source':None if mini else (a>>5)&63,
        'source_component':None if mini else 'xyzw'[a>>30],
        'source_relative':None if mini else bool(a&(1<<11)),
        'fetch_constant_index':None if mini else ((a>>20)&31)*3+((a>>25)&3),
        'stride_dwords':None if mini else c&255,
        'offset_dwords':signed((c>>8)&0x7fffff,23),
        'format_code':(b>>16)&63,'signed':bool(b&(1<<12)),
        'normalized':not bool(b&(1<<13)),
        'signed_fraction_mode':(b>>14)&1,
        'index_rounded':None if mini else bool(b&(1<<15)),
        'exponent_adjust':signed((b>>24)&63,6),
        'predicated':bool(b&(1<<31)),'predicate_condition':bool(c&(1<<31)),
        'address_source':'previous_full_fetch' if mini else 'descriptor_and_source_register',
        'executed':False}

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
    alu=[]; fetch=False; fetches=[]
    for item in result:
        for instruction in item.get('instructions',[]):
            if instruction['kind']=='FETCH':
                fetch=True
                slot=instruction['slot']
                fetches.append({'slot':slot, **decode_fetch(words[slot*3:slot*3+3])})
                continue
            slot=instruction['slot']
            alu.append({'slot':slot, **decode_alu(words[slot*3:slot*3+3])})
    return {'control_flow':result,'alu':alu,'fetch':fetches,
        'instruction_semantics_analyzed':bool(alu) and not fetch and all(i['recognized_semantics'] for i in alu),
        'safe_to_skip_draw':False}

if __name__=='__main__':
    capture=json.loads(Path(sys.argv[1]).read_text())
    result={'scope':'linear flow and limited MAX/RETAIN_PREV output routing; no execution or draw safety proof','shaders':[analyze([int(w,16) for w in words]) for words in capture['shaders']]}
    print(json.dumps(result,indent=2))
