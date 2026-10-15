# Video calibration lock import

The previous INVALID_SYNC address 0xC1010100 was not an allocation to recreate at that address. The executable import inventory identifies slot 0x82000670 as xboxkrnl ordinal 449, VdHSIOCalibrationLock. Startup had left that slot unresolved.

The diagnostic host now allocates physical storage for a 28-byte RTL critical section, initializes it with spin count 10000, and publishes its guest address in the import slot before entering the title. It uses the existing physical allocator and RTL initializer. The startup helper uses its own PPC context, leaving the main thread context untouched. Allocation failure aborts startup.

Reference: Xenia RegisterVideoExports, https://github.com/xenia-project/xenia/blob/master/src/xenia/kernel/xboxkrnl/xboxkrnl_video.cc . The initialization policy follows that implementation; this is not GPU calibration emulation.

The integration test checks import replacement, critical-section fields, recursive acquisition/release using a guest KTHREAD, shared physical alias access and cleanup. The normal title run passes the former synchronization failure and stops at PENDING_IMPORT __imp__VdRetrainEDRAMWorker, guest LR 0x8240CB10, exit 20. The added allocation shifts later physical addresses; completion signal locations must be read from the new diagnostic log, not old captures.

Next: investigate the EDRAM retraining contract and the arguments/commands emitted by the title. GPU calibration and rendering remain unsupported; no game image is displayed.
