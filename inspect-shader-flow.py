"""Read-only Xenos CF analysis. Does not execute or prove shaders side-effect-free.
Layout reference: xenia/src/xenia/gpu/ucode.h.
"""
import json, sys
from pathlib import Path

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
    return {'control_flow':result,'instruction_semantics_analyzed':False,'safe_to_skip_draw':False}

if __name__=='__main__':
    capture=json.loads(Path(sys.argv[1]).read_text())
    result={'scope':'linear control flow only; ALU/fetch semantics pending','shaders':[analyze([int(w,16) for w in words]) for words in capture['shaders']]}
    print(json.dumps(result,indent=2))
