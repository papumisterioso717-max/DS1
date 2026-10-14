import runpy
from pathlib import Path
decode=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-gpu-ring.py'))['decode']
r=decode([0xC0003600,0x10081,0x2000,0x12345678])
assert len(r)==2 and r[0]['opcode']=='0x36' and r[1]['register_index']=='0x2000'
assert r[1]['payload']==['0x12345678'] and r[1]['complete_in_snapshot']
assert not decode([0xC0013600,1])[0]['complete_in_snapshot']
assert decode([])==[]
assert len(decode([0x80000000,0]))==2
print('PASS 5 PM4 preview assertions')
