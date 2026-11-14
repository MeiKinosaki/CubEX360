# Gekko exception audit and GX FIFO plan

Status: partial exception implementation is present in the current CPU interpreter. This audit separates implemented entry paths from architectural closure and validation.

## Audited current code

The current `core-src/src/cpu.c` contains:
- External interrupt entry gated by MSR[EE], saving SRR0/SRR1 and selecting 0x500 or 0xFFF00500.
- Decrementer interrupt entry gated by DEC pending and MSR[EE], selecting 0x900 or 0xFFF00900.
- System-call instruction handling at 0xC00 or 0xFFF00C00.
- Program-exception entry for illegal instructions and privileged-instruction violations at 0x700 or 0xFFF00700.
- Data-storage exception entry on translated/bus data-access failures at 0x300 or 0xFFF00300, recording DAR and a store indicator in DSISR.
- Alignment exception entry at 0x600 or 0xFFF00600.
- Instruction-fetch failure entry at 0x400 or 0xFFF00400.
- RFI handling that restores PC from SRR0 and MSR from the currently defined SRR1 mask.
- BAT lookup integrated into instruction fetch and scalar data access when MSR[IR]/MSR[DR] are enabled.

These are implementation paths, not proof of full PowerPC 750/Gekko conformance. In particular:
- BAT block matching is experimental; PP/WIMG permissions and full BAT edge cases still need verification.
- SDR1/page-table translation and TLB behavior are not implemented.
- Instruction-storage cause details and full fault-status encoding need a hardware-accurate audit.
- The SRR1 mask and RFI restoration need exhaustive tests against the Gekko/750 specification.
- Unsupported decoder forms still return CUBEX_CPU_UNSUPPORTED; they must not be counted as architecturally handled exceptions.
- The machine coordinator advances DEC and the generic timer in instruction-step units, not measured Gekko bus cycles.

The machine routes implemented MMIO behavior for CP, PE, and PI. The Flipper address decoder recognizes the GX FIFO aperture at 0xCC008000, but the aperture is not yet connected to a bounded FIFO device and command processor.

## Remaining exception-core closure criteria

1. Add targeted regression tests for each implemented exception vector and both MSR[IP] vector prefixes.
2. Verify synchronous SRR0 semantics (faulting instruction address versus retry/skip behavior) independently from asynchronous interrupt return addresses.
3. Verify RFI privilege behavior, SRR1 restoration mask, reserved bits, and MSR[ILE]/MSR[LE] transition against reference behavior.
4. Test DAR/DSISR values for load/store faults, and instruction-storage cause/status registers for fetch faults.
5. Test interrupt priority, EE masking, pending-bit clearing, and interactions between external and decrementer interrupts.
6. Test BAT translation with overlapping/edge-sized blocks, supervisor/user validity, instruction/data selection, and permission behavior once PP/WIMG is implemented.
7. Implement SDR1/page-table translation and TLB semantics before claiming MMU coverage.
8. Update save-state serialization whenever architectural CPU state is added; preserve backward compatibility.
9. Require host regression, ASan/UBSan, and target smoke CI before marking the exception core validated.

## GX FIFO staged plan

1. Introduce a bounded FIFO/gather-pipe abstraction with explicit capacity, read/write indices, overflow/underflow behavior, and reset semantics.
2. Map only the documented GX FIFO MMIO aperture and enforce access widths/alignment.
3. Route CPU writes to FIFO words into the queue; do not treat the aperture as ordinary RAM or a generic register bank.
4. Add deterministic command-stream parsing for documented GX command headers and payload lengths.
5. Keep command decoding separate from rasterization; first validate command ingestion and state updates.
6. Add tests for wraparound, full/empty transitions, big-endian word ordering, invalid command lengths, and save-state round trips.
7. Only later connect CP/PE events and GPU execution to the FIFO consumer.

## Version policy

Do not advance the project version based on this audit alone. The exception implementation remains partial until the remaining architectural behavior and tests above are completed. GX FIFO work can proceed as a separate subsystem, but it must not be described as implemented until CPU writes, queue behavior, command parsing, and tests are connected end-to-end.
