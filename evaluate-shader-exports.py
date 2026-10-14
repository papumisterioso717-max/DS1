"""Offline evaluator for captured export-only MAX/RETAIN_PREV shaders.

Unknown inputs remain unknown. No GPU state, linkage, or rasterization is modeled.
"""
import json
import math
import runpy
import sys
from pathlib import Path
analyze = runpy.run_path(str(Path(__file__).with_name('inspect-shader-flow.py')))['analyze']


def evaluate(words, inputs=None):
    flow = analyze(words)
    if not flow['instruction_semantics_analyzed']:
        raise ValueError('Unsupported shader instructions')
    registers = {}
    for index, values in (inputs or {}).items():
        if type(index) is not int or not 0 <= index < 64 or len(values) != 4:
            raise ValueError('Invalid input register')
        if any(v is not None and (type(v) not in (int, float) or not math.isfinite(v)) for v in values):
            raise ValueError('Only finite inputs or unknown components are supported')
        registers[index] = list(values)
    exports = {}
    for instruction in flow['alu']:
        vm = instruction['vector_result_mask']
        sm = instruction['scalar_result_mask']
        if instruction['vector_clamp'] or instruction['scalar_clamp'] or sm:
            raise ValueError('Clamp and scalar result writes are not supported')
        if not instruction['export']:
            if vm or sm:
                raise ValueError('Temporary destination writes are not supported')
            continue
        destination = instruction['vector_destination']
        if destination not in (*range(16), 62):
            raise ValueError('Unsupported export destination')
        values = exports.setdefault(destination, [None] * 4)
        sources = []
        for source in instruction['sources'][:2]:
            value = registers.get(source['index'], [None] * 4)
            value = [value['xyzw'.index(c)] for c in source['swizzle']]
            value = [None if v is None else (-1 if source['negate'] else 1) * (abs(v) if source['absolute'] else v) for v in value]
            sources.append(value)
        for lane in range(4):
            bit = 1 << lane
            if instruction['constant_zero_mask'] & bit:
                values[lane] = 0.0
            elif instruction['constant_one_mask'] & bit:
                values[lane] = 1.0
            elif vm & bit:
                a, b = sources[0][lane], sources[1][lane]
                values[lane] = None if a is None or b is None else (a if a >= b else b)
    return {'exports': exports, 'unknown_components': sum(v is None for values in exports.values() for v in values),
            'draw_executed': False, 'linkage_modeled': False}


if __name__ == '__main__':
    capture = json.loads(Path(sys.argv[1]).read_text(encoding='utf-8-sig'))
    print(json.dumps({'scope': 'Offline export evaluation with unknown inputs; no GPU execution',
        'shaders': [evaluate([int(w, 16) for w in words]) for words in capture['shaders']]}, indent=2))
