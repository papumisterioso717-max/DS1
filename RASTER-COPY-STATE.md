# Raster and copy configuration storage

The 38-word Type-0 packet at physical 0x1F8E0374 spans 0x2300..0x2325. The diagnostic now retains this block as raw configuration, preserving the existing vertex-control (0x2302) and AA-mask (0x2312) fields. Other words use rasterCopyConfig indexed by register minus 0x2300; its slots 2 and 18 are unused to avoid duplicate authoritative state.

The block includes line/AA settings, guard-band adjustments, shader constants/configuration, opaque/debug words, vertex reuse/deallocation, copy control/destination/format, clear values and sample-count configuration. Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc . These are stored words, not executed rendering or copy actions. Unknown fields remain opaque. Writes do not dereference destination registers, copy memory or produce completion events.

No-output draw classification now additionally requires sample-count control 0x2324 to be zero. A request enabling it stops rather than silently omitting potential effects. Tests cover the entire block, preserved existing fields, unchanged destination memory, no completion events, writeback and reset. Boundary rejection moves to 0x2326. A new negative draw case covers sample-count enable.

Normal startup passes the 38-word block and further configuration packets. It stops at PENDING_GPU_REGISTER_PACKET index=4800, physical 0x1F8E0544, a six-word packet beginning at 0x4800. No image is produced.
