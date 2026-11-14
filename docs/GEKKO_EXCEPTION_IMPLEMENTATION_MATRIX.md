# Gekko exception implementation matrix

Status: source audit completed; implementation work remains open.

This matrix is based on the current default-branch source audit. It is not a claim that the listed exception handlers are implemented.

## Current CPU state/API observed

The public CPU state currently exposes GPRs, PC, LR, CTR, CR, XER, MSR, SRR0, SRR1, DEC, DEC-pending, and retired-instruction count. The result enum currently distinguishes OK, UNSUPPORTED, MEMORY_FAULT, and BAD_ARGUMENT.

The machine step currently:
1. Synchronizes device IRQ lines.
2. Checks DEC pending and attempts Decrementer entry.
3. Checks PI pending and attempts external interrupt entry.
4. Executes one instruction.
5. Decrements DEC by one instruction-step and advances the generic timer by one unit.

This is instruction-step timing, not cycle-accurate Gekko timing.

## Confirmed present in source

- External interrupt entry helper.
- Decrementer interrupt entry helper.
- Opcode 17 System Call path.
- PI pending line sourced from CP/PE event synchronization.
- CPU fetch/data operations currently use the memory bus callbacks.

## Concrete defects confirmed in current cpu.c

- RFI currently assigns `MSR = SRR1` directly and aligns SRR0 with `& ~3`. This is not a sufficient implementation of the architectural MSR restoration mask and should not be called validated RFI behavior.
- SC, external interrupt, and Decrementer paths share a simplified MSR transition helper pattern; the source currently does not show a separately tested synchronous-exception framework.
- Instruction fetch failure returns `CUBEX_CPU_MEMORY_FAULT` directly; it does not enter the Instruction Storage vector.
- Data load/store bus failures return `CUBEX_CPU_MEMORY_FAULT` directly; they do not populate DAR/DSISR or enter Data Storage.
- Misalignment is explicitly checked only in LMW/STMW paths in the audited decoder. Other halfword/word operations can reach the bus and return a generic memory fault instead of an Alignment exception.
- Unsupported opcodes return `CUBEX_CPU_UNSUPPORTED`; they do not raise Program Exception.
- The public CPU state has no DAR or DSISR fields.
- The machine advances DEC by one per instruction step, not by documented processor/bus cycles.

These are source observations, not assumptions about behavior hidden in an uninspected test.

## Remaining exception work

| Cause | Vector | Required work |
|---|---:|---|
| Instruction address alignment | 0x600 | Detect misaligned PC before fetch; save precise restart address; test |
| Data alignment | 0x600 | Detect access width/alignment; distinguish from bus/device faults |
| Data storage | 0x300 | Preserve effective fault address in DAR and classify DSISR cause |
| Instruction storage | 0x400 | Distinguish fetch translation/protection/bus failure from generic memory fault |
| Program | 0x700 | Add program-cause state for illegal instruction, privileged operation, and trap |
| System call | 0xC00 | Verify SRR0 and MSR semantics against 750CL/Gekko behavior |
| External | 0x500 | Verify pending/mask/EE gating and asynchronous SRR0 semantics |
| Decrementer | 0x900 | Verify edge/underflow behavior and priority against external interrupt |

## Required architectural state

Before claiming exception-core completion, audit whether the state and save-state formats include:
- DAR (Data Address Register)
- DSISR (Data Storage Interrupt Status Register)
- Any required program-exception cause bits
- Exception pending/latch state if it is not derivable from devices
- Correct SRR0/SRR1 save/restore semantics

Do not add placeholder registers and call the exception complete. Each register needs defined read/write behavior, reset value, exception-side effects, and save-state coverage.

## Test matrix

For every synchronous exception, test:
- Exact faulting PC and SRR0 value.
- SRR1 mask and MSR transition.
- Vector with MSR[IP] clear and set.
- Fault address/cause registers where applicable.
- No instruction retirement on a fault.
- Deterministic behavior if the handler returns with RFI.

For asynchronous exceptions, test:
- MSR[EE] clear blocks entry without losing the pending source.
- MSR[EE] set permits entry.
- Device mask/cause behavior remains correct.
- SRR0 identifies the next instruction boundary.
- Simultaneous DEC and PI requests follow a documented priority policy.

For RFI, test:
- Restoration mask, not a blind full-MSR copy.
- LE/ILE transition.
- PC restoration and alignment.
- No accidental retirement of RFI as a normal instruction if exception semantics require otherwise.

## GX FIFO gate

Do not start by mapping 0xCC008000 to an ordinary register. First implement a dedicated FIFO endpoint with:
- Big-endian 32-bit write semantics.
- Explicit full/empty and capacity behavior.
- A consumer boundary owned by CP/GX command ingestion.
- Deterministic save-state representation.
- Tests for wraparound, capacity, ordering, and reset.

Then route the FIFO MMIO aperture to this endpoint and add command-header parsing. FIFO ingestion is not rasterization and must not be reported as a functioning GPU.

## Version gate

Keep v1.93b in progress until exception behavior is implemented and validated. A documentation commit does not complete the milestone and does not change the version.
