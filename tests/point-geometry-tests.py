import runpy
from pathlib import Path
inspect=runpy.run_path(str(Path(__file__).resolve().parents[1]/'inspect-point-geometry.py'))['inspect_geometry']
regs={0x2204:0x10000,0x2205:0x10000,0x2206:0x300,0x2280:0x80008,0x2302:4,0x2080:0,0x2081:0,0x2082:0x100010}
def log(r): return ''.join(f'GPU_REGISTER index={i:04X} value={v:08X}\n' for i,v in r.items())+'GPU_DRAW_CAPTURE raw=00010081\n'
r=inspect(log(regs),[0,0,0,1])
assert r['candidate_pixel_centers']==[[0,0]] and r['point_size_pixels']==[1,1]
assert not r['final_coverage_known'] and not r['draw_executed']
# Last register write before the draw wins; later writes cannot affect it.
assert inspect('GPU_REGISTER index=2280 value=00000000\n'+log(regs)+'GPU_REGISTER index=2280 value=00000000\n',[0,0,0,1])==r
for bad in ('',log(regs)*2,log({k:v for k,v in regs.items() if k!=0x2206}),log(dict(regs,**{})).replace('value=00000300','value=00000301'),log(regs).replace('value=00100010','value=00000000')):
    try: inspect(bad,[0,0,0,1])
    except ValueError: pass
    else: raise AssertionError('Incomplete or unsupported state accepted')
print('PASS 8 geometry assertions/cases')
# Provenance distinguishes explicit zero from an unobserved default.
for mask in (0,7):
    snapshot=f'GPU_DRAW_COVERAGE surface=00000000 screen_tl=00000000 screen_br=00000000 written_mask={mask} defaults=diagnostic-zero-not-hardware\n'
    result=inspect(log(regs)+snapshot,[0,0,0,1])
    assert all(item['guest_write_observed']==bool(mask) for item in result['runtime_coverage_snapshot'].values())
    assert not result['final_coverage_known']
print('PASS 4 coverage provenance assertions')
