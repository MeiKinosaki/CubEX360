# Gekko exception audit and GX FIFO plan

Status: engineering audit, not an implementation-complete claim.

## Audited current code

The current `cpu.c` contains:
- External interrupt entry helper, gated by MSR[EE], saving SRR0/SRR1 and selecting 0x500/0xFFF00500.
- Decrementer interrupt entry helper, gated by DEC pending and MSR[EE], selecting 0x900/0xFFF00900.
- System-call opcode (SC) handling at vector 0xC00/0xFFF00C00.
- The generic instruction decoder still returns CUBEX_CPU_UNSUPPORTED for unimplemented opcodes/XO forms.
- Fetch failures still return CUBEX_CPU_MEMORY_FAULT.
- The audited decoder does not yet demonstrate a complete program-exception path for illegal/privileged instructions, data-storage exceptions, instruction-storage exceptions, alignment exceptions, or a verified RFI path.

The machine coordinator checks DEC and PI pending interrupts before executing an instruction, but currently advances DEC and the generic timer by one unit per retired instruction. This is an instruction-step approximation, not Gekko bus-cycle timing.

The machine currently routes implemented MMIO behavior for CP, PE, and PI. The Flipper address decoder recognizes the GX FIFO aperture at 0xCC008000, but that aperture is not yet connected to a FIFO device or command processor.

## Exception-core closure criteria

1. Add architecturally distinct exception causes and vectors:
   - System call: 0xC00.
   - Program exception: 0x700, with cause classification (illegal, privileged, trap).
   - Alignment: 0x600.
   - Data storage: 0x300.
   - Instruction storage: 0x400.
   - External interrupt: 0x500.
   - Decrementer: 0x900.
2. Verify SRR0 semantics separately for synchronous faults and asynchronous interrupts.
3. Implement and test RFI restoration semantics, including the architectural MSR mask and endianness transition.
4. Preserve fault address and cause in the appropriate exception registers where required.
5. Ensure an unsupported decoder path is not silently conflated with a completed architectural exception.
6. Add tests for priority, EE masking, vector prefix (MSR[IP]), context save/restore, and faulting instruction retry/skip behavior.
7. Update save-state serialization for every new architectural register/state and preserve backward compatibility.
8. Require host, ASan/UBSan, and target smoke CI before marking the exception core validated.

## GX FIFO staged plan

1. Introduce a bounded FIFO/gather-pipe abstraction with explicit capacity, read/write indices, overflow/underflow behavior, and reset semantics.
2. Map only the documented GX FIFO MMIO aperture and enforce its access widths/alignment.
3. Route CPU writes to FIFO words into the queue; do not treat the aperture as ordinary RAM or a generic register bank.
4. Add deterministic command-stream parsing for the documented GX command header and payload lengths.
5. Keep command decoding separate from rasterization; first validate command ingestion and state updates.
6. Add tests for wraparound, full/empty transitions, big-endian word ordering, invalid command lengths, and save-state round trips.
7. Only later connect CP/PE events and GPU execution to the FIFO consumer.

## Version policy

Do not advance the project version based on this document alone. The current v1.93b milestone remains in progress until the exception-core closure criteria are implemented and validated. GX FIFO work begins after that gate is met.
