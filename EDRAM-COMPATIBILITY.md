# EDRAM retraining diagnostic compatibility

This change implements a restricted compatibility path, not EDRAM training or rendering. Xenia's VdRetrainEDRAMWorker and VdRetrainEDRAM return zero without generating commands (https://github.com/xenia-project/xenia/blob/master/src/xenia/kernel/xboxkrnl/xboxkrnl_video.cc). Our worker accepts argument zero only. The retraining entry accepts the observed initial empty request: sizes 4096 and 2048, readable cursor/state/scratch ranges, and zero cursor/state words. Other requests stop as unsupported. Inputs are left untouched and no GPU completion is invented.

Local translated sub_8240CAD8 calls the worker at return address 0x8240CB10. It then passes two zero stack words and a 2048-byte scratch range to retraining. A zero return branches to 0x8240CBA4, releases VdHSIOCalibrationLock, leaves the critical region and returns. Nonzero would enter command generation and submission; that path is unsupported here.

Normal startup now passes this path and stops with exit 20 at VdIsHSIOTrainingSucceeded, guest LR 0x823F82AC. This is compatibility progression, not proof of hardware training. The next query must be assessed before implementing a response; no renderer or game image exists.

Tests verify zero return, unchanged argument registers/LR and unchanged guest buffers. Four subprocess checks cover a different worker argument, nonempty cursor, invalid pointer and unsupported capacity. Full build/runtime suites and the original local XEX diagnostic run are required validation.

## HSIO virtual status update

VdIsHSIOTrainingSucceeded now returns boolean 1, matching Xenia's virtual hardware stub. The diagnostic prints profile=virtual-compatibility and hardware-training=absent. This reports a compatibility policy, not measured training success. The title's caller at LR 0x823F82AC converts zero into a latched failure flag and enters a reporting path; nonzero continues initialization. Tests cover return width, argument/LR preservation and unchanged guest memory. There are now 9 EDRAM/HSIO checks plus four unsupported-request cases.

The normal diagnostic advances into another GPU submission and stops at unsupported Type-3 opcode 0x60 (PM4_SET_BIN_MASK_LO), physical 0x1F8E0100, exit 20. This packet is not discarded or marked complete. Next work is implementing bin mask/selection state and assessing predicated packets before accepting dependent rendering. Opcode reference: https://github.com/xenia-project/xenia/blob/master/src/xenia/gpu/xenos.h . No renderer or game image is present.
