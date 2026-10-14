import runpy
from pathlib import Path
analyze=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-shader-flow.py'))['analyze']
def pair(lo0,hi0,lo1,hi1): return [lo0,(hi0&65535)|((lo1&65535)<<16),(lo1>>16)|(hi1<<16)]
# First half EXEC_END executes slot 1, second half is padding.
w=pair(0x1001,0x2000,0,0)+[0,0,0]
r=analyze(w);assert r['control_flow'][0]['address']==1 and not r['safe_to_skip_draw']
# Second half EXEC_END, preceded by ALLOC; fetch and serialization flags.
w=pair(0,0xC400,0x31001,0x2000)+[0,0,0]
r=analyze(w);assert r['control_flow'][0]['allocation_type']==2
assert r['control_flow'][1]['instructions']==[{'slot':1,'kind':'FETCH','serialize':True}]
invalid=[[],[1],[-1,0,0],pair(0x1009,0x2000,0,0),pair(0,0x3000,0,0),[0,0,0],pair(0x1000,0x2000,0,0)]
for words in invalid:
 try: analyze(words)
 except ValueError: pass
 else: raise AssertionError('Malformed or unsupported shader accepted')
print('PASS 10 shader-flow assertions/cases')
