# Logical EDRAM addressing

`runtime/edram-layout.h` implements a bounded logical address model for 32-bit samples. Input coordinates are storage sample coordinates, not rasterizer sample IDs. It uses tile-major storage and row-major sample slots within each tile. Depth exchanges the two horizontal halves. Tile addressing wraps at the EDRAM boundary. Unsupported pitches and invalid bases are rejected without modifying the output address.

Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/xenos.h (EDRAM layout documentation). This is a host storage convention for the documented layout, not a physical dump of the GPU's memory banks.

For the candidate 640x360 area at 4x sampling, the storage footprint is 1280x720 samples: 16 tiles per row, 45 tile rows, 720 tiles total per 32-bit plane. The observed depth base 0 and color base 720 therefore describe adjacent, non-overlapping regions for that area. This calculation assumes the candidate area; surface pitch alone does not provide its height or prove coverage.

Tests enumerate both candidate planes to verify unique addresses and no overlap, plus tile edges, depth half exchange, wraparound, large coordinate arithmetic and invalid inputs. All 3,686,417 checks pass. The normal build now runs this test.

The helper is not connected to pending-draw execution yet. No color/depth values are written. Sample coverage, sample-ID placement, shader evaluation, storage format conversion, resolve and presentation remain separate work. The pending draw must continue to stop until those requirements are implemented.

## Logical storage

`runtime/edram-storage.h` adds host-side raw packed-word storage over this layout. Each word has a known-bit mask. Reads requiring unwritten bits fail without changing the caller's output. Masked writes preserve other bits and accumulate validity. Invalid addresses fail before mutation. Color and depth share backing memory, including wraparound and depth half-tile addressing. Each instance is independent. Storage consumes 10 MiB for values plus 10 MiB for validity masks.

`edram-storage-tests.cpp` passes 3,686,417 assertions, mostly exhaustive writes and reads across two synthetic planes. It also checks partial masks, unknown reads, zero masks, aliasing, wraparound, invalid addresses and instance isolation. These counts describe synthetic checks, not game frames or progress toward playability.

The normal build includes the new test. Neither helper is yet instantiated by the GPU command processor. Pixel format packing, sample coverage, depth/stencil operations and actual draw execution are still pending. This change does not alter the game's pending-draw exit or create an image.

## Limited depth/stencil writer

`runtime/depth-stencil.h` implements the captured ALWAYS/ALWAYS, KEEP/REPLACE/KEEP front-face operation for D24S8. It accepts already quantized 24-bit depth and caller-selected covered sample coordinates. Depth-write enable may vary; all other unsupported control changes are rejected before mutation. Stencil writes honor the eight-bit write mask. Existing unknown-bit tracking is preserved.

Packing reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/dxbc_shader_translator_om.cc (depth extraction at bit 8 and low-byte stencil merge). The captured depth value 1, once quantized to maximum D24, and stencil reference zero produce packed 0xFFFFFF00 under full writes.

Validation: 73 checks pass, including initialization without old contents, partial stencil preservation, depth-write disabled, unknown-bit preservation, all unsupported single-bit control changes, oversized depth and invalid addresses. This helper remains isolated from command execution. It is not a general depth/stencil engine; float quantization, coverage, backface state and other comparison/operation modes remain unsupported.

## Covered-sample integration

`runtime/covered-sample-writer.h` joins logical storage and the restricted depth/stencil operation with packed color writes. Its caller must provide established coverage as storage sample coordinates, packed color, and a packed bit mask. It does not infer coverage, blend, convert formats or mark a GPU packet complete.

Every coordinate in both planes is validated before any write. For this restricted synchronous path, invalid later coordinates and unsupported state reject the whole batch without partial mutation. The writer applies depth/stencil then color for each sample; callers must establish appropriate non-overlapping targets for the clear path.

Validation: 21 integration checks pass using synthetic coverage, including untouched samples, invalid later coordinates, invalid color base, unsupported control and partial color/stencil masks with depth writes disabled. The test is included in build.py.

Runtime integration remains blocked on generating justified coverage for the captured rectangle and validating all relevant render state. This is a tested storage/output component, not an executed title draw. No new game run or rendered frame is claimed in this step.

## Rectangle coverage prototype

`runtime/rectangle-coverage.h` computes coverage for an explicitly supplied axis-aligned rectangle in 1/16-pixel units, integer pixel centers, scissor bounds and four explicit sample offsets. It emits logical storage sample coordinates and uses inclusive top/left and exclusive bottom/right edges. It bounds allocation/work and leaves output unchanged on invalid input. It does not assemble rectangle-list vertices or decode guest AA registers.

The integration test uses the captured rectangle bounds (-0.5,-0.5) to (639.5,359.5) with an explicitly selected test pattern. It generates 921,600 samples, writes both logical planes and checks the final sample as well as an untouched sample outside the area. Twenty checks pass, including exact-edge equality, scissor restriction, empty geometry and rejection without partial output. build.py includes this test.

Pattern context: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/draw_util.cc documents the host D3D sample pattern. A host pattern must not be represented as a verified Xenos hardware pattern. Guest AA/vertex state still requires validation before runtime dispatch. The integration test is an offline prototype with supplied geometry and color/depth, not shader execution or a rendered game frame. The real command processor remains stopped at the pending draw.

## Rasterizer-state audit

The captured PA_SU_SC_MODE_CNTL is 0x00010000: rasterizer MSAA enable is OFF, while RB_SURFACE_INFO still specifies four-sample storage. These must not be conflated. The updated target inspector reports both independently. Pixel centers are integer (PA_SU_VTX_CNTL=4), clip is disabled, window offset is enabled, and both cull bits are off.

Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/registers.h (PA_SU_SC_MODE_CNTL and PA_SU_VTX_CNTL).

The coverage tests now enumerate all 225 strictly interior sixteenth-pixel offsets, including zero, on half-pixel-aligned rectangle boundaries. Each produces the same complete pixel set. All 4,070 checks pass. This establishes offset independence for that bounded geometric case, not the hardware rule for distributing center coverage to storage samples. AA masks/configuration, offsets, rectangle assembly and shader output validation still need to be resolved before command completion.

## Real scissor bounds and full regression

Fixed the coverage work budget to intersect conservative geometry bounds with the scissor before allocation/iteration. The actual 8192x8192 scissor no longer rejects the small captured rectangle. Signed fixed-point bounds use explicit floor division; disjoint negative rectangles yield empty coverage. Large genuinely covered regions still fail before output mutation.

Updated coverage tests pass 925,673 checks, including the real scissor dimensions and output coordinates bounded to the candidate rectangle. A complete build and its existing test suites passed (work/graphics-integration-build.log). The updated coverage test was rebuilt and run separately after the bounds change. A fresh normal title run still exits 20 at the original pending draw (work/graphics-integration-run.log), with no image or draw completion.

Research clarification: Xenia's D3D12 pipeline_cache.cc selects host sample count from RB_SURFACE_INFO. That implementation choice does not independently establish the hardware effect of PA_SU_SC_MODE_CNTL.msaa_enable being zero. Keep the raw-state distinction without interpreting it as proof of center-sample replication.
Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/d3d12/pipeline_cache.cc .
