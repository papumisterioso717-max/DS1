# Primitive configuration register storage

The 21-word Type-0 packet at physical 0x1F8E031C spans 0x2280..0x2294. Existing point-size and visibility-query state are retained. New primitiveConfig entries correspond in order to 0x2281..0x2292: point min/max, line control, line stipple, output path, HOS control/max/min/reuse, group primitive/first-decrement/decrement/vector0/vector1/vector-format0/vector-format1, UNKNOWN_2290, UNKNOWN_2291 and multipass pixel-shader control. VGT_ENHANCE at 0x2294 has a separate field.

Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc . Unknown words remain opaque; no guessed behavior is attached. Values are stored unchanged through the register storage path. This does not implement tessellation, primitives, visibility queries or rasterization. Unsupported draw paths continue to stop.

Tests cover all 21 fields, exact bits, isolated overwrite, ring writeback and reset. The old point-neighbor rejection is replaced by full-block acceptance. The visibility-range rejection moves to the new unsupported boundary 0x2295.

Normal startup passes the block and stops at PENDING_GPU_REGISTER_PACKET index=2300, physical 0x1F8E0374. The next packet spans 38 words starting at 0x2300; it remains rejected before application. No image is produced.
