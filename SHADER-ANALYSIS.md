# Captured first-draw ALU inspection

Read-only analysis, 2026-10-14. No shader or draw execution.

| Stage / slot | Effective output |
| --- | --- |
| Vertex / 3 | No destination write; scalar RETAIN_PREV |
| Vertex / 4 | Export 0 (interpolator): constant zero in xyzw |
| Vertex / 5 | Export 62 (position): constant zero in xyz, one in w |
| Vertex / 6 | No destination write; scalar RETAIN_PREV |
| Pixel / 1 | Export 0 (color): MAX(r0, r0) in xyzw |

All five encode vector MAX and scalar RETAIN_PREV. Export mask overlap selects constant one; the export zero-fill flag selects zero for remaining components. The MAX result is unused in the four vertex slots. No memory export destinations occur in these captured instructions. This does not establish all GPU side effects or justify skipping DRAW_INDX_2. Pixel input linkage and rasterization remain unimplemented. No image has been rendered.

The analyzer recognizes only a limited unpredicated MAX/RETAIN_PREV subset with direct temporary sources. Other operations remain explicitly unrecognized; FETCH is not decoded. Constant addressing is not resolved. Source 3 is displayed as an encoded field but MAX uses sources 1 and 2; RETAIN_PREV has no source input. Recognition describes the operation and output routing, not execution or complete shader validation.

Validation: 10 existing flow assertions/cases and 13 synthetic ALU assertions/cases passed. The local capture reproduces the five outputs above. Raw shader captures remain local.

Layout reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/ucode.h

## Offline export evaluation

`evaluate-shader-exports.py` now evaluates the limited export-only subset using caller-supplied temporary inputs. Missing input components stay unknown (JSON null); they are never silently zero-initialized. The captured vertex exports evaluate to interpolator (0,0,0,0) and position (0,0,0,1) without inputs. The pixel export remains four unknown components until r0 is supplied. This does not establish the actual hardware input linkage.

The tool rejects unsupported operations, temporary writes, scalar result writes, clamps and unsupported export destinations. It uses finite Python numbers for this limited offline model, not a bit-exact implementation of Xenos floating point; NaN and infinity inputs are rejected. No GPU execution, interpolation, clipping, rasterization or framebuffer output was added. The runtime still stops at the first DRAW_INDX_2.

Validation: 17 offline evaluator assertions/cases pass, alongside the previous 23 decoder checks. Local capture results are in first-draw-exports.json (not committed).

## Offline single-point linkage

`link-point-shaders.py` accepts only program 0x1000000E, context 0, one point and the inspected export pattern. This configuration has one interpolator, one pixel register, no parameter generation, and position-only vertex export mode. The model supplies vertex interpolator 0 to pixel r0: (0,0,0,0). The pixel result is consequently (0,0,0,0) IF a pixel invocation occurs. It does not determine sample coverage, clipping, point expansion, or framebuffer effects. The captured color mask is zero. This is not a visible game frame or permission to skip the draw.

The CLI verifies that the shader words and draw identity match the supplied diagnostic log before using its program/context state. Unsupported configurations and missing pixel inputs are rejected. Nine synthetic linkage checks passed; the local capture produces first-draw-link.json. No runtime change was made.

Register and linkage references:
- https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/registers.h
- https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/shader_translator.cc (GetInterpolatorInputMask)
