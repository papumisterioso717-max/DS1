import runpy
from pathlib import Path
api=runpy.run_path(str(Path(__file__).resolve().parents[1]/'evaluate-shader-exports.py'))
evaluate=api['evaluate']
def shader(a,b=0,c=(2<<24)|(7<<29)):
    return [0x1001,0x2000,0,a,b,c]
base=50<<26
zero=shader(base|0xC000)
pos=shader(base|0xC03E|(8<<16)|(8<<20))
color=shader(base|0x8000|(15<<16))
assert evaluate(zero)['exports']=={0:[0.,0.,0.,0.]}
assert evaluate(pos)['exports']=={62:[0.,0.,0.,1.]}
assert evaluate(color)['unknown_components']==4
assert evaluate(color,{0:[-2.,0.,3.,4.]})['exports'][0]==[-2.,0.,3.,4.]
assert evaluate(color,{0:[None,0.,3.,4.]})['unknown_components']==1
assert evaluate(shader(base))['exports']=={}
# Distinct inputs and swizzles exercise MAX, not just identity.
w=shader(base|0x8000|(15<<16),0x55<<16,(2<<24)|(7<<29)|(1<<8))
assert evaluate(w,{0:[1,2,3,4],1:[5,1,6,0]})['exports'][0]==[5,3,6,1]
invalid=[shader(base|(15<<16)),shader(base|0x8000|(1<<20)),shader(base|0x8000|(1<<24)),shader(base|0x8000|32),shader(base,1<<28),shader(base,0,0)]
for w in invalid:
    try: evaluate(w)
    except ValueError: pass
    else: raise AssertionError('Unsupported behavior accepted')
for value in ([1,2,3], [float('nan'),0,0,0], [float('inf'),0,0,0]):
    try: evaluate(color,{0:value})
    except ValueError: pass
    else: raise AssertionError('Invalid input accepted')
assert evaluate(color)['draw_executed'] is False
print('PASS 17 offline export evaluation assertions/cases')
