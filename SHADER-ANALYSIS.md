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

## Point geometry inspection

`inspect-point-geometry.py` reads the last logged configuration writes before the single captured draw; it never substitutes zero for missing state. The inspected state disables viewport scaling/offset, uses a fixed 1x1 point (half-size 8/16 per axis), integer pixel centers, zero window offset, and window scissor [0,0,16,16). With position (0,0,0,1), the point bounds are [-0.5,-0.5,0.5,0.5]; the candidate pixel center is (0,0).

This is only a candidate after the window scissor. Global screen scissor and surface multisample state have not been captured, and hardware sample coverage is not implemented. Final coverage remains unknown; no shader invocation, framebuffer write or GPU completion is claimed. Next runtime work needs explicit capture/modeling of the missing state before integrating drawing.

Eight geometry checks and the nine linkage checks pass. The combined report remains local in first-draw-link.json.
Sources: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/registers.h and https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/xenos.h

## Runtime coverage-state capture

The runtime now stores RB_SURFACE_INFO (0x2000) and PA_SC_SCREEN_SCISSOR_TL/BR (0x200E/0x200F), tracks writes separately from initial storage, and emits GPU_DRAW_COVERAGE before stopping. The repeated real diagnostic reports all three stored words zero and written_mask=0: no guest writes to these registers were observed before the first draw. These are diagnostic defaults, not verified console reset values. This supersedes the earlier description of the state as merely missing from the capture: its lack of initialization is now explicit.

Do not infer an empty screen scissor or one-sample rendering from those zeros. Initialization semantics remain unresolved. The draw still stops with exit 20 and ring_read=22. No pixel execution or completion was added. Tests cover writes, wraparound, explicit zero provenance, reset and draw capture; 137 processor checks pass. Geometry/provenance tests total 12 checks.

Register indices: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc

## Initialization investigation and bounded lookahead

Xenia's RegisterFile constructor zeros its storage, and its VdInitializeEngines entry is a stub returning success. Neither establishes console reset values for these registers. No missing concrete register setup was identified in those reference entry points. Sources: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_file.cc and https://github.com/xenia-project/xenia/blob/master/src/xenia/kernel/xboxkrnl/xboxkrnl_video.cc

The diagnostic now previews at most 64 words after the stopped draw, limited to the current submitted list. It does not recurse into indirect buffers, execute commands, or advance completion. Unreadable preview memory stops only the preview. The real capture contains 46 remaining words: 23 additional identical DRAW_INDX_2 packets, each raw 0x00010081, with no intervening configuration writes. Together with the stopped draw this is a run of 24 identical draws. A setup/warm-up purpose is plausible but unproven; the preview does not justify skipping them.

`inspect-gpu-ring.py --draw-tail diagnostic.log` decodes the preview including payloads. Five parser checks pass, and the runtime draw test verifies that a following register write is captured without execution. The real diagnostic still exits 20 at the first draw, with ring_read=22. The local report is first-draw-tail.json.
