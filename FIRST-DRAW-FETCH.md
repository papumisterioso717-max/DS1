# First active draw: fetch inspection

The offline shader-flow inspector now reports vertex-fetch fields, including signed offsets, signed exponent adjustment and full versus mini fetches. Mini fetches deliberately leave inherited address fields null. Texture and other fetch opcodes remain explicitly unsupported. This does not execute shaders or make a draw safe to skip.

Layout reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/ucode.h (VertexFetchInstruction).

Local diagnostic capture for the active draw at 0x1F8E0594:

- Vertex instruction slot 3: full fetch, descriptor 0, index r0.x, destination r1.xyz1, stride 7 dwords (28 bytes), offset 0, format code 57.
- Slot 4: mini fetch, reusing the preceding full-fetch address, destination r0.xyzw, offset 3 dwords (12 bytes), format code 38.
- Both fetches are unpredicated, with exponent adjustment zero.
- Vertex ALU exports r0 to interpolator 0 and r1 to position export 62 through the recognized MAX routing.
- The captured pixel shader has no fetch instructions and exports r0 through MAX to color output 0.

The local JSON report is work/first-draw-shader-analysis.json. Raw shader captures are not included in the publication checkout.

Next: capture bounded vertex data, validate descriptor format/endian interpretation, then evaluate positions and colors. Surface layout, depth, sample behavior and presentation still require implementation. The runtime remains at PENDING_GPU_DRAW, renderer absent; this change produces no game image.

## Bounded resource capture (2026-10-16)

The pending-draw diagnostic now previews valid vertex descriptors only, skipping texture groups. It caps each descriptor at 64 dwords and the entire draw at 256 dwords, and checks the complete preview range before reading. Unmapped previews are logged and skipped. It does not change guest memory, register state or draw completion.

The normal run captured descriptor 0 at physical 0x1F8F007C: 21 dwords, endian mode 2 (8-in-32). Format codes 57 and 38 are float3 and float4 respectively, according to Xenia xenos.h: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/xenos.h .

Decoded records (28 bytes each):

| Vertex | Position xyz | Additional xyzw |
| --- | --- | --- |
| 0 | -0.5, -0.5, 1 | 0, 0, 0, 0 |
| 1 | 639.5, -0.5, 1 | 0, 0, 0, 0 |
| 2 | 639.5, 359.5, 1 | 0, 0, 0, 0 |

These data suggest a screen-space clearing operation, but surface/depth/sample semantics must be established before executing it. They are not a rendered frame. Local report: work/first-draw-vertices.json.

Validation: rebuilt GPU runtime and diagnostic executable; GPU suites passed (including 137 processor checks). Normal title execution captured all 84 bytes and still exited 20 at PENDING_GPU_DRAW renderer=absent.

## Target state inspection

Run `python inspect-draw-target.py diagnostic.log` to decode only register writes before the pending draw. Missing required fields and reserved MSAA encoding are rejected. The result does not infer surface height from pitch or treat tail commands as executed.

Observed target: pitch 640 pixels, 4 samples, 16 tiles per row for 32-bit storage. Color target 0 uses format 8_8_8_8 at tile 720 with all channels enabled. Depth is D24S8 at tile 0, comparison ALWAYS, writes enabled. Front stencil comparison is ALWAYS and the depth-pass action replaces it with reference zero, using write mask 255. Backface stencil selection, alpha testing and alpha-to-mask are disabled. Viewport scale/offset enable bits are zero.

This supports the clearing-operation hypothesis but does not establish final coverage or execute it. AA configuration remains raw; sample coverage, complete EDRAM addressing, shader output routing to targets and presentation remain required. The observed 640-pixel pitch does not establish the final display resolution.

Validation: draw-target-tests.py passes checks for MSAA pitch, signed bias, tile base, depth/stencil fields, missing state, reserved encoding and exclusion of later writes. Local result: work/first-draw-target.json.
