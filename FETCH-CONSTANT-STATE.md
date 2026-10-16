# Fetch descriptor state

Type-0 register writes now retain SHADER_CONSTANT_FETCH_00_0 through SHADER_CONSTANT_FETCH_31_5, the 192 words at 0x4800..0x48BF. Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc . Each group contains six words; this is raw descriptor storage, without interpreting addresses or formats.

Descriptors may contain invalid resource addresses without causing host reads: storing a register does not perform a texture/vertex fetch. Shader execution and rendering remain unsupported beyond the existing constrained no-output classification. Unknown registers remain rejected before a packet's write loop.

Tests cover first/last groups, distinct words, untouched intervening descriptors, unchanged resource memory, ring writeback and shutdown reset. Separate cases check the 0x48C0 boundary and a truncated six-word packet.

Normal startup passes the six-word descriptor packet and stops at PENDING_GPU_REGISTER_PACKET index=5000, physical 0x1F8E0560, a three-word packet beginning at 0x5000. It is rejected before partial application. No image is produced.
