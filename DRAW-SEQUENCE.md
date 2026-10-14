# Provenance of the repeated draw sequence

Static inspection of locally recompiled routines, plus isolated execution of two leaf builders. Original function names are unknown; addresses are identifiers, not recovered symbols.

- 0x823EDA70 emits 24 identical two-word DRAW_INDX_2 packets and returns 48 words. The local probe executes the actual generated routine and checks every packet and memory outside its 192-byte output.
- 0x823EDA10 builds a WAIT_UNTIL and SQ_GPR_MANAGEMENT sequence. With input r4=64 it emits 13 words matching the captured list at physical 0x1FCE9D40: idle wait, eight writes of 0x00040401, then 0x00040400. The probe checks its return length, all output words, and surrounding memory.
- 0x823EDAA8 allocates list storage, constructs configuration variants for inputs 16 through 112, and embeds the same 24-draw loop. It records the draw-list length/address in object offsets 14816/14820. This constructor was inspected statically, not executed by the isolated probe.
- 0x823EF448 updates configuration fields, appends indirect-buffer references for the base configuration, a selected GPR configuration, and finally the repeated-draw list from offsets 14816/14820. It then calls 0x823ED8F8, which marks local state fields for refresh. These relationships match the order of the three nested lists observed at runtime, but no dynamic call-site tracing was added.

The evidence links the repeated draws to a shader-register reconfiguration path. A pipeline transition/workaround purpose is an inference, not a recovered original function name or verified hardware contract. In particular, this does not prove they can be discarded, what completion guarantees the caller needs, or that the first draw generates a visible pixel. No runtime rendering behavior was changed.

Reproduce with verify-draw-builder.py --toolchain <LLVM-MinGW bin directory> --xenonrecomp <XenonRecomp checkout>. It requires locally generated ppc sources, extracts only the two reviewed leaf routines into build/, compiles an isolated probe and executes it. No game source or raw capture is included in the repository script. The generated probe and executable remain local.

Next investigation: validate the reconfiguration/completion requirements before adding a bounded implementation of this draw sequence. Do not infer hardware reset values from diagnostic zero storage.
