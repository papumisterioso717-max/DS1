import runpy
from pathlib import Path
m=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-draw-target.py'))
r={k:0 for k in (0x2000,0x2001,0x2002,0x2104,0x210C,0x2200,0x2201,0x2202,0x2204,0x2205,0x2206,0x2208,0x2301,0x2302,0x2312)}
r.update({0x2000:0x0A020280,0x2001:720,0x2200:0x8777,0x210C:0xFFFF00})
d=m['decode'](r)
assert d['surface']['samples']==4 and d['surface']['pitch_tiles_32bpp']==16
assert d['surface']['height_pixels'] is None and d['color0']['base_tile']==720
assert d['depth']['comparison']=='always' and d['depth']['write_enabled']
assert d['stencil_front']['depth_pass']=='replace' and d['stencil_front']['write_mask']==255
assert not d['draw_executed'] and not d['coverage_known']
assert not d['rasterizer']['msaa_enabled'] and d['surface']['samples']==4
r[0x2205]=0x18003
r[0x2302]=5
d=m['decode'](r)
assert d['rasterizer']['msaa_enabled'] and d['rasterizer']['window_offset_enabled']
assert d['rasterizer']['cull_front'] and d['rasterizer']['cull_back']
assert d['rasterizer']['pixel_center']=='half_integer' and d['rasterizer']['rounding_code']==2
for samples,tiles in ((0,8),(1,8),(2,16)):
    r[0x2000]=640|(samples<<16)
    assert m['decode'](r)['surface']['pitch_tiles_32bpp']==tiles
r[0x2001]=(32<<20)|2048|2047
assert m['decode'](r)['color0']['exponent_bias']==-32
assert m['decode'](r)['color0']['base_tile']==2047
log=''.join(f'GPU_REGISTER index={k:04X} value={v:08X}\n' for k,v in r.items())
assert m['inspect_log'](log+'GPU_DRAW_CAPTURE raw=0\nGPU_REGISTER index=2000 value=00000000')['surface']['pitch_pixels']==640
for bad in ({},dict(r,**{})):
    if bad: bad[0x2000]=0x30000
    try:m['decode'](bad)
    except ValueError:pass
    else:raise AssertionError('Missing or reserved state accepted')
print('PASS draw target decoding, pitch, signed bias, provenance and rejection checks')
