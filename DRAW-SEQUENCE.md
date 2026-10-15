# Provenance of the repeated draw sequence

Static inspection of locally recompiled routines, plus isolated execution of two leaf builders. Original function names are unknown; addresses are identifiers, not recovered symbols.

- 0x823EDA70 emits 24 identical two-word DRAW_INDX_2 packets and returns 48 words. The local probe executes the actual generated routine and checks every packet and memory outside its 192-byte output.
- 0x823EDA10 builds a WAIT_UNTIL and SQ_GPR_MANAGEMENT sequence. With input r4=64 it emits 13 words matching the captured list at physical 0x1FCE9D40: idle wait, eight writes of 0x00040401, then 0x00040400. The probe checks its return length, all output words, and surrounding memory.
- 0x823EDAA8 allocates list storage, constructs configuration variants for inputs 16 through 112, and embeds the same 24-draw loop. It records the draw-list length/address in object offsets 14816/14820. This constructor was inspected statically, not executed by the isolated probe.
- 0x823EF448 updates configuration fields, appends indirect-buffer references for the base configuration, a selected GPR configuration, and finally the repeated-draw list from offsets 14816/14820. It then calls 0x823ED8F8, which marks local state fields for refresh. These relationships match the order of the three nested lists observed at runtime, but no dynamic call-site tracing was added.

The evidence links the repeated draws to a shader-register reconfiguration path. A pipeline transition/workaround purpose is an inference, not a recovered original function name or verified hardware contract. In particular, this does not prove they can be discarded, what completion guarantees the caller needs, or that the first draw generates a visible pixel. No runtime rendering behavior was changed.

Reproduce with verify-draw-builder.py --toolchain <LLVM-MinGW bin directory> --xenonrecomp <XenonRecomp checkout>. It requires locally generated ppc sources, extracts only the two reviewed leaf routines into build/, compiles an isolated probe and executes it. No game source or raw capture is included in the repository script. The generated probe and executable remain local.

Next investigation: validate the reconfiguration/completion requirements before adding a bounded implementation of this draw sequence. Do not infer hardware reset values from diagnostic zero storage.

## Opt-in zero-surface investigation

Reference: Xenia D3D12CommandProcessor::IssueDraw returns success without drawing when surface_pitch is zero, and explicitly notes an unresolved memexport concern. https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/d3d12/d3d12_command_processor.cc

The diagnostic has an opt-in probe, DS1_PROBE_ZERO_SURFACE=1. It accepts at most 24 one-point auto-index draws with raw 0x00010081, a zero surface-info word, mode 4, zero color/depth controls and zero visibility-query control. Every bypass logs rendering absent and side effects unverified. It is an investigation policy, not a supported renderer or proof of hardware equivalence; it must not be enabled for general shader execution. In particular, it does not analyze arbitrary shader memory exports. Default behavior is unchanged.

A real experimental run bypassed 24 commands, then observed window scissor reset, state invalidation and EVENT_WRITE_SHD (opcode 0x58, three payload words) at physical 0x1F8E00E0. It stopped there with exit 20. Local log: zero-surface-probe.log. No event completion is fabricated. A separate normal run still stops at the first draw with exit 20 and no bypass messages. This separates experimental reachability from verified runtime support.

Validation includes a synthetic candidate rejected by default and accepted only with the explicit environment flag, tracking/reset checks, and the repeated real runs. Next task: inspect the event payload and implement only justified completion behavior; do not treat the experimental bypass as rendering progress.

## Shader completion event capture

EVENT_WRITE_SHD is now recognized and captured without signaling completion. It requires exactly three payload words; malformed packets are rejected. The experimental run observes initiator 3 (PS_DONE_TS), encoded address 0x1FCEE006, physical address 0x1FCEE004, byte-order mode 2 and literal value 0xBF8E00D4. No counter is requested. The four requested bytes are BF 8E 00 D4. The destination differs from the ring read-pointer writeback (0x1FCEE03C).

`inspect-shader-event.py` decodes all four byte-order modes and leaves counter-generated values unknown. Nine decoder checks pass; runtime tests verify capture and malformed length rejection. No event memory write is made, especially after experimental draw bypasses. The observed event provides a concrete synchronization target to trace next, not proof of completed GPU work. The normal-mode first-draw stop remains unchanged.

Reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/command_processor.cc (ExecutePacketType3_EVENT_WRITE_SHD), event identifiers in https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/xenos.h.
