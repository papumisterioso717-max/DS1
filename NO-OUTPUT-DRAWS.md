# Classified no-output draw support — 2026-10-15

The normal diagnostic now processes the captured 24 initialization draws without the experimental bypass flag, then publishes the two literal shader-completion events. The title proceeds to KeEnterCriticalRegion at guest return address 0x8240CAF4 and stops with exit 20. No framebuffer or image is produced.

This is a constrained compatibility implementation, not execution of a general shader or a console timing model. Xenia's IsRasterizationPotentiallyDone treats zero surface pitch as no rasterization. We additionally require the full surface-info word to be zero, mode 4, zero color/depth/HiZ/query controls, the inspected program/context configuration, and exactly one auto-index point. This deliberately avoids deriving sample coverage from unverified hardware reset state.

Both uploaded shaders must pass a bounded classifier: complete 96-bit groups, unconditional linear NOP/ALLOC/EXEC/EXEC_END control flow, no instruction/control-flow overlap, only MAX and RETAIN_PREV, direct temporary sources, no predicates/modifiers, no FETCH, no temporary/scalar result writes, and only ordinary vertex interpolator/position or pixel color exports. Memory/depth exports, kill operations, unsupported control flow and malformed ranges are rejected. The source contains no copied game shader words. General draws retain the prior capture-and-stop path.

The accepted subset has no shader memory effects, and the zero-surface compatibility policy suppresses rasterization. It can therefore complete in this synchronous diagnostic without a framebuffer. Unsupported performance/visibility queries already stop before reaching this path. Previous experimental bypasses still taint completion and cannot be upgraded by this classifier.

Tests: 23 classifier checks including malformed programs and state changes; processor integration uploads synthetic shaders, processes a classified draw and verifies the event memory value/ring completion; negative cases reject color-enabled and memory-export programs. Existing GPU tests and the full build/runtime suite pass. The real normal run logs 24 GPU_NO_OUTPUT_DRAW entries, zero GPU_EXPERIMENTAL_DRAW_BYPASS entries, two GPU_SHADER_EVENT_COMPLETE entries, and the new CPU import blocker.

This supersedes earlier notes that all first draws must stop. The offline tools remain deliberately conservative and do not authorize arbitrary draw skipping.

References:
- https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/draw_util.cc (IsRasterizationPotentiallyDone)
- https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/ucode.h (instruction layout)
