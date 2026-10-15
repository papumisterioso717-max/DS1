# Contiguous render configuration

The observed 12-word packet at 0x1F8E0234 spans 0x2200 through 0x220B. Previously the first missing field was 0x2201. The diagnostic now retains RB_BLENDCONTROL0..3, RB_COLORCONTROL and VGT_CURRENT_BIN_ID_MIN alongside its existing depth, hierarchical-depth, clip/transform and mode fields. Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc .

This is register storage only. Blending, color operations and rasterization are not implemented. Unknown register packets continue to stop before the write loop; 0x220C remains unsupported. The regression submits the entire observed range with distinct values, verifies both existing and new fields plus ring writeback, checks reset, and rejects a packet crossing the supported boundary.

The normal title run passes the full block and stops at a five-word packet beginning at 0x2180, physical 0x1F8E0268. First missing register: 0x2182 (SQ_INTERPOLATOR_CNTL); the following fields are SQ_WRAPPING_0/1. These remain unsupported. No image is produced.

The former clip/depth/mode-neighbor rejection fixtures are superseded: those adjacent registers are now supported. Coverage moves to the complete block acceptance test and the 0x220C boundary rejection.
