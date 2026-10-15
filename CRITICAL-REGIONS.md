# Critical regions: diagnostic support

KeEnterCriticalRegion and KeLeaveCriticalRegion now maintain a nested signed counter per guest KTHREAD, located through r13/KPCR +0x100. Void exports preserve the caller registers. Invalid pointers, read-only storage, overflow and unbalanced leave stop with exit 22.

The +0xB0 counter follows Xenia's host-endian runtime field, not a claim that all Xbox thread fields are native endian. References: https://github.com/xenia-project/xenia/blob/master/src/xenia/kernel/xthread.h and https://github.com/xenia-project/xenia/blob/master/src/xenia/kernel/xthread.cc . APC queue exports still stop as unsupported; reaching nesting zero does not implement APC delivery.

Validation: full build and runtime suites passed; 9 focused checks cover nesting, independent guest threads, preserved argument/LR and restored memory. Four subprocess cases verify rejection of unbalanced leave, null PCR, overflow and read-only counter.

Normal diagnostic with original local XEX advances beyond the previous pending import and exits 22: INVALID_SYNC unmapped address=0xC1010100. The next call in sub_8240CAD8 is RtlEnterCriticalSection, return address 0x8240CB00, loading its pointer from guest 0x82000670. This call-site identification comes from translated code; the failure log itself does not report LR. Next work: trace allocation/physical aliases and initialization of this critical section before changing synchronization behavior.

There is still no renderer or game image. No game executable, translated game source or captures are included in this snapshot.

Update: the unmapped lock blocker is resolved by VIDEO-CALIBRATION-LOCK.md; the earlier run above is historical. Current blocker: VdRetrainEDRAMWorker at 0x8240CB10.
