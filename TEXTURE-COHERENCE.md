# Shared-memory texture coherence

COHER_STATUS_HOST bit 25 is tc_action_ena; bit 24 is vc_action_ena and bit 31 is pending status. Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/registers.h . The observed 0x82000000 request is therefore texture-cache coherence, with no destination or other action bits.

The synchronous diagnostic now accepts exactly 0x81000000 and 0x82000000 after validating the entire physical range. A CPU memory fence precedes clearing the status. This is appropriate only while resources are shared CPU memory with no separate texture cache or renderer; a rendering backend must replace it with actual cache/resource synchronization. Requests following experimental skipped draws are rejected. Other action modes remain unsupported.

Regression coverage exercises a texture request and wait, ring/read-pointer completion, preserved resource data, an unsupported action and an unmapped range. Existing vertex coherence and GPU suites remain required.

The title now completes the request for physical base 0x0AA00000, size 0x00398000, and advances to an unsupported register packet at physical 0x1F8E0234: first unknown register 0x2201. The packet has 12 values starting at 0x2200. Validation stops before applying that packet. No image or renderer exists.
