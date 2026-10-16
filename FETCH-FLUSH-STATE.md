# Fetch-flush register compatibility

The three-word Type-0 packet at physical 0x1F8E0560 writes SHADER_CONSTANT_FLUSH_FETCH_0..2, registers 0x5000..0x5002. Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc . The diagnostic retains the exact words independently from descriptor storage.

The host has no derived descriptor cache or GPU backend to invalidate. This is opaque register compatibility only; hardware flush semantics are not claimed. The words do not overwrite fetch constants, dereference resource addresses, write resource memory or generate completion events. A future graphics backend must implement cache invalidation before relying on these fields.

Tests combine a descriptor upload and the three-register write, checking exact bits, unchanged descriptors/resource memory, ring writeback and reset. Separate subprocess cases reject crossing 0x5003 and truncated payloads.

Normal startup now passes this packet and further depth/mode configuration. The next stop is a predicated DRAW_INDX_2 packet (header C0003601), physical 0x1F8E0594. Predicate handling remains unsupported, so no draw or ring completion is invented. Depth control at this point is 0x00008777; the prior constrained no-output draw policy does not cover this state. No image is rendered.
