# GameCube Flipper MMIO apertures — CubEX360

Status: **address decoder implemented; device behavior not implemented by this file.**

The decoder is based on YAGCD chapter 4 (memory map) and chapter 5 (hardware registers). It recognizes the following conservative block windows:

| Region | Base | Size | Notes |
|---|---:|---:|---|
| CP | 0xCC000000 | 0x80 | Common access width 16-bit |
| PE | 0xCC001000 | 0x100 | Common access width 16-bit |
| VI | 0xCC002000 | 0x100 | Mixed 16/32-bit registers |
| PI | 0xCC003000 | 0x100 | 32-bit registers |
| MI | 0xCC004000 | 0x80 | Includes protection/interrupt registers |
| DSP | 0xCC005000 | 0x200 | DSP mailbox, ARAM and audio DMA controls |
| DI | 0xCC006000 | 0x40 | DVD interface |
| SI | 0xCC006400 | 0x100 | Includes 0x80-byte SI I/O buffer at +0x80 |
| EXI | 0xCC006800 | 0x40 | Three EXI channels |
| AI | 0xCC006C00 | 0x20 | Audio streaming interface |
| GX FIFO | 0xCC008000 | 0x4 | Write-gather FIFO port; writes of varying widths |

## Important implementation boundary

`cubex_flipper_decode_address()` identifies an aperture and returns its region-relative offset. It does **not** mean every byte in an aperture is a defined register, nor that reads/writes have hardware semantics. Reserved offsets, register width, read-only/write-only behavior, write-one-to-clear bits, side effects, FIFO streaming and interrupts must be handled by each device implementation.

The existing generic 64-word MMIO bank remains a prototype and is not yet replaced by per-device register models. The decoder is not yet connected to CPU-visible device handlers. In particular, the GX FIFO requires write-gather semantics and must not be treated as an ordinary storage register.

## References

- YAGCD, chapter 4 — memory map: https://hitmen.c02.at/files/yagcd/yagcd/chap4.html
- YAGCD, chapter 5 — hardware registers, including CP, PE, VI, PI, MI, DSP, DI, SI, EXI, AI and GX FIFO: https://hitmen.c02.at/files/yagcd/yagcd/chap5.html
