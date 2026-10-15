# Shader interpolator and wrapping configuration

The five-word Type-0 packet at physical 0x1F8E0268 spans SQ_PROGRAM_CNTL, SQ_CONTEXT_MISC, SQ_INTERPOLATOR_CNTL, SQ_WRAPPING_0 and SQ_WRAPPING_1 (0x2180..0x2184). The last three fields are now retained independently, available through the same register access path, and cleared at shutdown. Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc .

This stores configuration; it does not execute interpolation or shaders. Existing draw classification and fail-fast behavior remain in place. The test checks the full packet with distinct values, existing/new fields, ring writeback and reset. The prior unsupported-range fixture now crosses 0x2184 into unsupported 0x2185.

Normal startup passes the five-word block (all five words are zero in this run). The next stop is PENDING_GPU_REGISTER_PACKET index=2001 at physical 0x1F8E0280, a 16-word packet starting at 0x2000. This concerns render-surface configuration; it is not applied partially. No image is rendered.
