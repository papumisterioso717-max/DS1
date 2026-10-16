"""Fetch field boundaries and conservative flow classification."""
import runpy
from pathlib import Path
m=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-shader-flow.py'))
decode=m['decode_fetch']
# Full fetch with distinct source/destination, final valid descriptor and signed fields.
a=(17<<5)|(1<<11)|(23<<12)|(1<<18)|(1<<19)|(31<<20)|(2<<25)|(3<<30)
b=0x688|(1<<12)|(1<<13)|(1<<14)|(1<<15)|(38<<16)|(32<<24)|(1<<31)
c=255|(0x400000<<8)|(1<<31)
r=decode([a,b,c])
assert r['source']==17 and r['destination']==23 and r['source_component']=='w'
assert r['fetch_constant_index']==95 and r['stride_dwords']==255
assert r['source_relative'] and r['destination_relative'] and r['index_rounded']
assert r['exponent_adjust']==-32 and r['offset_dwords']==-4194304
assert r['predicated'] and r['predicate_condition'] and not r['executed']
assert r['destination_swizzle']==list('xyzw')
r=decode([a,b|(1<<30),0x3fffff00])
assert r['mini'] and r['offset_dwords']==4194303
assert all(r[k] is None for k in ('source','source_component','source_relative','stride_dwords','fetch_constant_index','index_rounded'))
assert decode([1,0,0])['decoded'] is False
for bad in ([],[0,0],[-1,0,0],[True,0,0],[0,0,1<<32]):
    try: decode(bad)
    except ValueError: pass
    else: raise AssertionError('Invalid fetch accepted')
# An EXEC_END containing only a fetch must never become safe to skip.
r=m['analyze']([0x11001,0x2000,0,0,0,0])
assert len(r['fetch'])==1 and not r['instruction_semantics_analyzed'] and not r['safe_to_skip_draw']
print('PASS shader fetch: signed boundaries, mini inheritance, unsupported opcode, validation and safety')
