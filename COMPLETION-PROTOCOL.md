# CPU/GPU completion protocol

The locally recompiled leaf routine 0x823EBF80 constructs INVALIDATE_STATE and two EVENT_WRITE_SHD packets. It reads a completion-page pointer at object+10896, current buffer position at +48, sequence at +10908 and tag bits at +14920. The first event targets completion-page+4 and writes the buffer position combined with its two tag bits. The second targets completion-page+0 and writes the sequence. Both request event 3 and big-endian writes. The routine advances the object's sequence by two.

The real preview agrees: first event writes BF8E00D4 to physical 1FCEE004; the next queued event writes 3 to 1FCEE000. Only the first event was reached in the experimental run; the second is a read-only preview, not an executed event. Ring-pointer writeback uses the separate address 1FCEE03C.

Readers found in generated code:
- 0x823EAEA8 compares a requested sequence with the completion-page value using unsigned differences from the current sequence. In the probe, a request for 3 with current sequence 5 returns 1 when completion is 1 and 0 when completion is 3.
- 0x823EB450 reads completion-page+4 and evaluates pointer/tag differences. With identical request arguments, the probe returns 1 for an old differently tagged value and 0 for the matching completed value. This is a verified test scenario, not a complete wraparound/ordering specification.
- 0x823EB490 contains a retry path reading completion-page+4; its wait path was inspected but not executed in the probe.

The producer also contains a conditional CPU-side completion-write branch when object+21532 is zero and flag bit 1 at +10941 is set. Its purpose and whether the real device uses that mode are not established. The probe keeps the flag clear and verifies the producer does not prematurely modify either completion word. Do not force this guest flag to bypass GPU work.

Validation: verify-completion-protocol.py extracts and executes the three reviewed leaf routines from local generated sources. It verifies all ten emitted words, the returned output position, sequence advancement, preservation of both completion words during generation, and reader responses before/after synthetic test-memory completion. No game code is embedded in the committed probe script; generated harness files remain under build/.

This establishes the observable CPU synchronization contract more precisely. The runtime still must not signal completion after unverified draw bypasses. No runtime behavior or rendering capability changed in this investigation.

## Supported synchronous event path (2026-10-15)

The runtime now supports the literal PS_DONE_TS event (initiator exactly 3) when no experimental draw has been bypassed since engine initialization. Accepted work in this diagnostic is synchronous; unsupported drawing still stops before reaching completion. The event validates the physical destination and write protection, applies all four byte-order modes, publishes one aligned 32-bit word with InterlockedExchange, and records its initiator/count. Counter events and other initiators remain unsupported. Experimental bypass history blocks completion and is not cleared by ring reconfiguration.

Validation: full build/test suite passed; after changing publication to an atomic word, the GPU suites were rerun on the final code. Tests cover four byte orders, neighboring memory, reset, malformed length, counter/unsupported initiator, unmapped/out-of-range and read-only destination rejection, and refusal after a bypassed draw. Normal title run still stops at the first draw. The experimental title run still stops at the captured event after 24 bypasses, without a completion write. This adds the validated event-writing primitive, not a working renderer or a completed title submission.
