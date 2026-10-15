# Render surface configuration

The Type-0 packet at physical 0x1F8E0280 spans 0x2000..0x200F. Added storage covers RB_COLOR_INFO, RB_DEPTH_INFO and RB_COLOR1/2/3_INFO (0x2001..0x2005). Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc . The existing surface descriptor, coherence destination addresses and screen scissors retain their original handling.

These words are stored without interpreting them as host pointers or allocating render targets. This does not implement color/depth rendering. Unsupported draw configurations still stop. Tests submit 16 distinct words, verify new and existing fields, ring writeback and shutdown reset. The existing destination-neighbor rejection still checks the 0x2010 boundary.

Normal startup passes this block. The next stop is PENDING_GPU_REGISTER_PACKET index=2103, physical 0x1F8E02C4, in a 21-word packet beginning at 0x2100. The new packet is rejected before applying any values. No image is produced.
