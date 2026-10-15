# GPU bin mask and selection state

The processor now accepts unpredicated SET_BIN_MASK (0x50), SET_BIN_SELECT (0x51), SET_BIN_MASK_LO/HI (0x60/0x61) and SET_BIN_SELECT_LO/HI (0x62/0x63). Full updates consume high then low words. Half updates preserve the other half. Mask and selection are independent 64-bit fields and engine shutdown clears them.

Packet lengths are validated before any state mutation. Predicate-bit packets remain unsupported and stop with exit 20, regardless of the stored masks. This deliberately does not claim conditional execution, tiled rendering or framebuffer support. Tests cover all six updates, preservation of the opposite half, independent fields, reset, malformed length and rejection of predicated updates.

Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/command_processor.cc (Type-3 bin-state commands). A future predication implementation must test the mask/selection intersection before executing dependent packets; merely accepting these state writes does not implement that behavior.

Normal title validation: bin updates pass and execution reaches PENDING_GPU_COHERENCE_MODE status=82000000 at a WAIT_REG_MEM packet (physical 1F8E0154). This cache-coherence mode remains unsupported; the submission is not completed. No image is rendered.
