"""Read-only target decode. No rasterization, sample coverage or EDRAM writes.

Field references: Xenia gpu/registers.h and gpu/xenos.h.
"""
import json
import re
import sys
from pathlib import Path

COMPARE=('never','less','equal','less_equal','greater','not_equal','greater_equal','always')
STENCIL=('keep','zero','replace','increment_clamp','decrement_clamp','invert','increment_wrap','decrement_wrap')

def decode(registers):
    required=(0x2000,0x2001,0x2002,0x2104,0x210C,0x2200,0x2201,0x2202,0x2204,0x2205,0x2206,0x2208,0x2301,0x2302,0x2312)
    for key in required:
        if key not in registers:
            raise ValueError(f'Missing observed register {key:04X}')
        if type(registers[key]) is not int or not 0<=registers[key]<=0xffffffff:
            raise ValueError('Register must be an unsigned 32-bit integer')
    s,c,d,z,st=(registers[k] for k in (0x2000,0x2001,0x2002,0x2200,0x210C))
    msaa=(s>>16)&3
    if msaa==3:
        raise ValueError('Reserved MSAA encoding')
    pitch=s&0x3fff
    cf=(c>>16)&15
    bias=(c>>20)&63
    blend=registers[0x2201]
    return {'surface':{'pitch_pixels':pitch,'samples':1<<msaa,
                'pitch_tiles_32bpp':(pitch*(2 if msaa==2 else 1)+79)//80,
                'height_pixels':None},
        'color0':{'base_tile':c&2047,'base_bit11':bool(c&2048),'format_code':cf,
                  'format':'8_8_8_8' if cf==0 else 'not_named',
                  'exponent_bias':bias-64 if bias&32 else bias,
                  'write_mask':registers[0x2104]&15,
                  'blend_fields':{'source_rgb':blend&31,'operation_rgb':(blend>>5)&7,
                                  'destination_rgb':(blend>>8)&31,'source_alpha':(blend>>16)&31,
                                  'operation_alpha':(blend>>21)&7,'destination_alpha':(blend>>24)&31}},
        'depth':{'base_tile':d&2047,'format':'D24FS8' if d&(1<<16) else 'D24S8',
                 'test_enabled':bool(z&2),'write_enabled':bool(z&4),'comparison':COMPARE[(z>>4)&7]},
        'stencil_front':{'enabled':bool(z&1),'comparison':COMPARE[(z>>8)&7],
                         'fail':STENCIL[(z>>11)&7],'depth_pass':STENCIL[(z>>14)&7],
                         'depth_fail':STENCIL[(z>>17)&7],
                         'reference':st&255,'read_mask':(st>>8)&255,'write_mask':(st>>16)&255},
        'backface_stencil_enabled':bool(z&128),
        'rasterizer':{'msaa_enabled':bool(registers[0x2205]&(1<<15)),
                      'window_offset_enabled':bool(registers[0x2205]&(1<<16)),
                      'cull_front':bool(registers[0x2205]&1),
                      'cull_back':bool(registers[0x2205]&2),
                      'clip_disabled':bool(registers[0x2204]&(1<<16)),
                      'pixel_center':'half_integer' if registers[0x2302]&1 else 'integer',
                      'rounding_code':(registers[0x2302]>>1)&3,
                      'quantization_code':(registers[0x2302]>>3)&7},
        'viewport_transform_bits':registers[0x2206]&63,
        'alpha_test_enabled':bool(registers[0x2202]&8),
        'alpha_to_mask_enabled':bool(registers[0x2202]&16),
        'mode_code':registers[0x2208]&7,
        'aa_config_raw':f'{registers[0x2301]:08X}',
        'aa_mask_raw':f'{registers[0x2312]:08X}',
        'draw_executed':False,'coverage_known':False,
        'unresolved':['sample locations and coverage','EDRAM addressing and storage',
                      'other render targets and backface state','resolve and presentation']}

def inspect_log(log):
    if log.count('GPU_DRAW_CAPTURE ')!=1:
        raise ValueError('Expected exactly one captured draw')
    # Lookahead after the pending draw has not executed and must never alter state.
    before=log.split('GPU_DRAW_CAPTURE ',1)[0]
    registers={int(k,16):int(v,16) for k,v in re.findall(
        r'GPU_REGISTER index=([0-9A-F]{4}) value=([0-9A-F]{8})',before)}
    result=decode(registers)
    result['observed_registers']={f'{k:04X}':f'{v:08X}' for k,v in registers.items()}
    return result

if __name__=='__main__':
    print(json.dumps(inspect_log(Path(sys.argv[1]).read_text()),indent=2))
