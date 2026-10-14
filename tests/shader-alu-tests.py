import runpy
from pathlib import Path
api=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-shader-flow.py'))
decode=api['decode_alu']
# Synthetic routing examples: overlapping export masks are constants, not ALU output.
r=decode([(50<<26)|(1<<15)|(1<<14)|62|(8<<16)|(8<<20),0,(2<<24)|(7<<29)])
assert (r['constant_zero_mask'],r['constant_one_mask'],r['vector_result_mask'],r['scalar_result_mask'])==(7,8,0,0)
assert r['vector_destination']==62 and r['scalar_destination']==62
r=decode([(50<<26)|(1<<15)|(15<<16),0,(2<<24)|(7<<29)])
assert r['vector_result_mask']==15 and r['constant_zero_mask']==0 and r['recognized_semantics']
r=decode([(50<<26)|(8<<16)|(8<<20),0,(2<<24)|(7<<29)])
assert (r['vector_result_mask'],r['scalar_result_mask'],r['constant_one_mask'])==(8,8,0)
r=decode([(50<<26)|(1<<15)|(1<<14),0,(2<<24)|(7<<29)])
assert r['constant_zero_mask']==15
r=decode([50<<26,(1<<26)|(0x55<<16),(2<<24)|(7<<29)|(0x85<<16)])
assert r['sources'][0]==dict(kind='temporary',index=5,relative=False,absolute=True,negate=True,swizzle='yzwx')
assert not decode([50<<26,1<<28,(2<<24)|(7<<29)])['recognized_semantics']
assert not decode([0,0,0])['recognized_semantics']
assert not decode([50<<26,0,(2<<24)])['recognized_semantics']
for w in ([],[0,0],[-1,0,0],[0,0,1<<32]):
 try: decode(w)
 except ValueError: pass
 else: raise AssertionError('Invalid ALU accepted')
print('PASS 13 ALU routing assertions/cases')
