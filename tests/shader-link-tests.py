import runpy
from pathlib import Path
link=runpy.run_path(str(Path(__file__).resolve().parents[1]/'link-point-shaders.py'))['link_point']
# Synthetic two-instruction VS: export zero interpolator then homogeneous position.
vs=[0x2001,0x2000,0, 0xC800C000,0,0xE2000000, 0xC888C03E,0,0xE2000000]
ps=[0x1001,0x2000,0,0xC80F8000,0,0xE2000000]
r=link(vs,ps,0x1000000E,0,1,1)
assert r['pixel_color_0']==[0.,0.,0.,0.] and r['position']==[0.,0.,0.,1.]
assert not r['draw_executed'] and not r['framebuffer_written']
assert r['configuration']['pixel_register_count']==1
for config in [(0x1004000E,0,1,1),(0x1000000E,1,1,1),(0x1000000E,0,4,1),(0x1000000E,0,1,2),(0x1000010E,0,1,1)]:
    try: link(vs,ps,*config)
    except ValueError: pass
    else: raise AssertionError('Unsupported configuration accepted')
# Missing r1 must not silently become zero.
missing=ps.copy(); missing[-1]=0xE2010100
try: link(vs,missing,0x1000000E,0,1,1)
except ValueError: pass
else: raise AssertionError('Unknown input accepted')
print('PASS 9 point linkage assertions/cases')
