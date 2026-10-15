import runpy
from pathlib import Path
decode=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-shader-event.py'))['decode']
for mode,expected in enumerate((['78','56','34','12'],['56','78','12','34'],['12','34','56','78'],['34','12','78','56'])):
    r=decode(4,0x1000+mode,0x12345678)
    assert r['requested_bytes']==expected and r['physical_address']=='0x00001000'
assert decode(0x80000004,0x1002,123)['requested_bytes'] is None
assert not decode(4,0x1002,123)['completion_proven']
for value in (-1,1<<32,True):
    try:decode(value,0,0)
    except ValueError:pass
    else:raise AssertionError('Invalid event field accepted')
print('PASS 9 shader event decode assertions/cases')
