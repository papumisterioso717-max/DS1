# Viewport and color configuration state

The observed 21-word packet at physical 0x1F8E02C4 spans 0x2100..0x2114. Added fields are primitive restart index (0x2103), blend constants (0x2105..08), fog color (0x2109..0B), back/front stencil reference and masks (0x210C..0D), alpha reference (0x210E), and viewport scale/offset pairs (0x210F..14). Existing vertex bounds/offset and color mask remain unchanged. Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/register_table.inc .

Values are raw 32-bit words: floating-point payloads are not converted or canonicalized. This only preserves configuration, not primitive restart, blending, stencil tests or viewport rasterization. Tests submit the complete block using distinct NaN bit patterns, verify exact storage and writeback, and check viewport reset. The old vertex/color neighbor rejection fixtures are replaced by whole-block acceptance and rejection at 0x2115.

Normal startup passes this entire block and stops at PENDING_GPU_REGISTER_PACKET index=2281, physical 0x1F8E031C, in a 21-word packet starting at 0x2280. The remaining block is not partially applied. Rendering and game imagery remain absent.
