from pathlib import Path
import runpy
f=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-gpu-ring.py'))['decode']
assert f([])==[]
assert f([0])[0]['payload_words']==0
assert f([0x80000000])[0]['complete_in_snapshot']
assert not f([0x40000000,1])[0]['complete_in_snapshot']
assert f([0x00010001,1,2])[0]['payload_words']==2
p=f([0xC0114800]+[0]*18+[0xC0013F00,0,0])
assert len(p)==2 and p[0]['opcode']=='0x48' and p[1]['snapshot_word']==19 and p[1]['opcode']=='0x3F'
print('PASS 6 ring inspector cases')
