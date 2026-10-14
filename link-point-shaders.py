"""Offline single-point VS-to-PS linkage model; never submits a GPU draw.

Register layout: xenia/src/xenia/gpu/registers.h; interpolator input selection:
xenia/src/xenia/gpu/shader_translator.cc, Shader::GetInterpolatorInputMask.
Only the captured configuration is supported. No coverage or framebuffer model.
"""
import json
import re
import runpy
import sys
from pathlib import Path
evaluate = runpy.run_path(str(Path(__file__).with_name('evaluate-shader-exports.py')))['evaluate']


def link_point(vertex, pixel, program, context, primitive, count):
    if any(type(v) is not int or not 0 <= v <= 0xffffffff for v in (program, context, primitive, count)):
        raise ValueError('Invalid configuration word')
    fields = {'vertex_register_count': (program & 63) + 1,
              'pixel_register_count': ((program >> 8) & 63) + 1,
              'interpolator_count': ((program >> 20) & 15) + 1,
              'vertex_export_mode': (program >> 24) & 7,
              'pixel_export_mode': (program >> 27) & 15,
              'parameter_generation': bool(program & (1 << 18))}
    # Deliberately allow only the inspected state, not unmodeled point-sprite,
    # screen-coordinate, generated-index, or alternate export configurations.
    if program != 0x1000000E or context != 0 or primitive != 1 or count != 1:
        raise ValueError('Unsupported linkage configuration')
    vs = evaluate(vertex)
    if set(vs['exports']) != {0, 62} or vs['unknown_components']:
        raise ValueError('Expected known interpolator 0 and position only')
    if vs['exports'][62] != [0., 0., 0., 1.]:
        raise ValueError('Unsupported position for captured-point model')
    # A single point has one vertex; its constant interpolator does not require
    # triangle barycentrics. This models values IF a pixel invocation occurs.
    ps = evaluate(pixel, {0: vs['exports'][0]})
    if set(ps['exports']) != {0} or ps['unknown_components']:
        raise ValueError('Unsupported pixel outputs or missing input registers')
    return {'configuration': fields, 'position': vs['exports'][62],
            'pixel_input_r0': vs['exports'][0], 'pixel_color_0': ps['exports'][0],
            'scope': 'Values for a hypothetical covered sample; coverage not determined',
            'draw_executed': False, 'framebuffer_written': False}


if __name__ == '__main__':
    capture = json.loads(Path(sys.argv[1]).read_text(encoding='utf-8-sig'))
    log = Path(sys.argv[2]).read_text(encoding='utf-8-sig')
    states = re.findall(r'GPU_DRAW_STATE .*?program=([0-9A-F]{8}) context=([0-9A-F]{8})', log)
    shaders = [[int(w, 16) for w in shader] for shader in capture['shaders']]
    logged = re.findall(r'GPU_SHADER_WORD stage=(\d+) index=(\d+) value=([0-9A-F]{8})', log)
    expected = [(str(s), str(i), f'{word:08X}') for s, words in enumerate(shaders) for i, word in enumerate(words)]
    draws = re.findall(r'GPU_DRAW_CAPTURE raw=([0-9A-F]{8}) primitive=(\d+) source=(\d+) major=\d+ index32=\d+ count=(\d+)', log)
    if len(states) != 1 or logged != expected or draws != [(capture['raw'], str(capture['primitive']), str(capture['source']), str(capture['count']))]:
        raise ValueError('Capture and diagnostic log do not identify the same single draw')
    result = link_point(*shaders, *(int(v,16) for v in states[0]), capture['primitive'], capture['count'])
    inspect_geometry = runpy.run_path(str(Path(__file__).with_name('inspect-point-geometry.py')))['inspect_geometry']
    result['geometry'] = inspect_geometry(log, result['position'])
    print(json.dumps(result, indent=2))
