"""Inspect captured point geometry; candidates are not final sample coverage."""
import re


def inspect_geometry(log, position):
    if log.count('GPU_DRAW_CAPTURE ') != 1:
        raise ValueError('Expected exactly one captured draw')
    before = log.split('GPU_DRAW_CAPTURE ', 1)[0]
    registers = {int(i,16): int(v,16) for i,v in re.findall(r'GPU_REGISTER index=([0-9A-F]{4}) value=([0-9A-F]{8})', before)}
    required = (0x2204,0x2205,0x2206,0x2280,0x2302,0x2080,0x2081,0x2082)
    if any(i not in registers for i in required):
        raise ValueError('Missing geometry configuration; defaults are not inferred')
    if position != [0,0,0,1] or any(registers[i] != v for i,v in
        ((0x2204,0x10000),(0x2205,0x10000),(0x2206,0x300),(0x2280,0x80008),(0x2302,4),(0x2080,0))):
        raise ValueError('Unsupported point geometry configuration')
    tl,br=registers[0x2081],registers[0x2082]
    if tl != 0 or br != 0x100010:
        raise ValueError('Unsupported window scissor')
    return {'point_size_pixels':[1.0,1.0], 'center':[0.0,0.0],
        'bounds_before_global_clipping':[-0.5,-0.5,0.5,0.5],
        'window_scissor':[0,0,16,16], 'pixel_center_convention':'integer',
        'candidate_pixel_centers':[[0,0]], 'viewport_transform_enabled':False,
        'final_coverage_known':False,
        'unresolved':['global screen scissor not captured','surface multisample state not captured','hardware sample coverage not implemented'],
        'draw_executed':False}
