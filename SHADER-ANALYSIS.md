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
