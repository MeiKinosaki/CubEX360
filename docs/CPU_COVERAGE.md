# CubEX360 — inventário de cobertura da CPU Gekko (v1.61b)

Este inventário é uma leitura qualitativa do interpretador em `core-src/src/cpu.c`, não uma porcentagem certificada de compatibilidade. Uma instrução presente no decodificador ainda precisa de testes de casos-limite e não implica que jogos reais funcionem.

## Instruções implementadas no interpretador parcial

- **Imediatas e comparação:** `mulli`, `addic`, `addic.`, `addi`, `addis`, `ori`, `oris`, `xori`, `xoris`, `andi.`, `andis.`, `cmpwi`, `cmplwi`.
- **Load/store inteiro:** `lwz`, `lbz`, `lhz`, `lha`, `stw`, `stb`, `sth`.
- **Rotação e máscara:** `rlwimi`, `rlwinm`, `rlwnm`.
- **Controle de fluxo:** `b`, `bc`, `bclr`, `bcctr`, incluindo parte das condições BO/BI e atualização de LR/CTR.
- **CR:** `cmpw`, `cmplw`, operações lógicas CR (`crnor`, `crandc`, `crxor`, `crnand`, `crand`, `creqv`, `crorc`, `cror`), `mfcr`, `mtcrf`, `mcrxr`.
- **Lógicas e bits:** `and`, `andc`, `or`, `orc`, `xor`, `nor`, `eqv`, `nand`, `cntlzw`, `extsb`, `extsh`.
- **Deslocamentos:** `slw`, `srw`, `srawi`.
- **SPR suportados:** leitura/escrita de `LR`, `CTR` e `XER` por `mfspr`/`mtspr`.
- **Aritmética e carry:** `add`, `subf`, `addc`, `subfc`, `adde`, `subfe`, `addme`, `addze`, `subfme`, `subfze`.
- **Overflow assinado recém-adicionado:** `addo`, `subfo` — a validação de CI da v1.61b ainda está pendente; não tratar como validado até os jobs terminarem.

## Limitações de arquitetura conhecidas

- **FPU Gekko e paired-single:** não implementadas.
- **MMU, TLB e exceções completas:** não implementados; faults de memória são retornados como resultado do interpretador, não como exceções arquiteturais completas.
- **Interrupções, timers e estados privilegiados:** incompletos.
- **Instruções inteiras:** a lista acima não é exaustiva da ISA PowerPC/Gekko; muitas instruções continuam retornando `CUBEX_CPU_UNSUPPORTED`.
- **GameCube:** o CPU parcial não substitui o restante da plataforma (Flipper/VI, DSP, drive, IPL/BIOS, controladores e sincronização).

## Validação e build

- A suíte host e ASan/UBSan são executadas pelo GitHub Actions.
- O alvo Free60/libXenon é um smoke build ELF32 PowerPC big-endian; não é uma aplicação XEX completa.
- `default.xex` funcional, boot em Xbox 360 RGH e compatibilidade com jogos continuam não comprovados.

## Próximas etapas recomendadas

1. Fechar a validação v1.61b, corrigindo falhas antes de adicionar novos opcodes.
2. Adicionar testes com e sem Rc e com operandos positivos/negativos para instruções aritméticas.
3. Auditar `OE`, `Rc`, SO/OV/CA e o comportamento de XER em todas as instruções aritméticas.
4. Implementar exceções arquiteturais e alinhamento/mapeamento com resultados definidos.
5. Integrar um ponto de entrada de aplicação ao build de destino e só então retomar a trilha de XEX.
6. Tratar FPU, MMU, Flipper/VI, DSP, drive e IPL como subsistemas próprios, com testes e milestones separados.


### v1.62b — em validação
- Adicionada `MULLW` (XO 235): produto de palavras com sinal, armazenando os 32 bits inferiores do produto; forma `Rc` atualiza CR0.
- Testes host cobrem `-2 × 3` e um produto que retorna zero nos 32 bits inferiores.
- CI para a implementação #93 está em andamento; não marcar `MULLW` como validada até a suíte host e sanitizers passarem.

- **Multiplicação:** `mullw` (XO 235) foi adicionada na v1.62b; CI host e sanitizers passaram, mas o smoke build ainda estava em andamento na última consulta. `sraw` (XO 792) foi adicionada na v1.63b; os testes host/sanitizers passaram na última consulta, enquanto os builds de destino seguem em execução. Não declarar essas revisões totalmente validadas até todos os jobs terminarem.

- **Multiplicação:** `mullw` (v1.62b) passou em host, sanitizers e smoke build no run #94. `mulhwu` (XO 11, v1.64b) foi adicionada para retornar os 32 bits superiores do produto unsigned; os workflows #97–#98 ainda estão em execução.

- **Nota de regressão:** os testes de `SRAW` na execução #96 falharam por codificação invertida dos registradores fonte/destino no teste; correção enviada no commit `63f634aa805505185ef4faf72db953ed21abd384`, aguardando CI #99. A falha da execução #97 de `MULHWU` ocorreu na suíte compartilhada com essa regressão; não marcar v1.64b como validada até #98/#99 concluírem.

- **Multiplicação:** `mulhw` (XO 75, v1.65b) foi adicionada para obter a palavra superior do produto signed de 32 bits; testes de produto negativo e produto com palavra superior zero foram adicionados. CI #100–#101 pendente.

- **Validação atualizada:** o run #99 passou em host, sanitizers, probe e smoke build após a correção do teste `SRAW`; a revisão testada contém `MULHWU`. `MULHW` v1.65b permanece pendente nos runs #100–#101.

- **Validação atualizada:** `MULHW` v1.65b passou host, sanitizers, probe e smoke build nos runs #100–#101; `SRAW`/`MULHWU` passou após a correção no run #99. A cobertura continua parcial e não implica compatibilidade com jogos.

- **Multiplicação com overflow:** `mullwo` (XO 747, v1.66b) foi adicionada para armazenar a palavra inferior do produto signed e atualizar `XER.OV`/`XER.SO`; CI #102–#103 pendente.

- **Divisão unsigned:** `divwu` (XO 459) e `divwuo` (XO 971) adicionadas na v1.67b. A forma com overflow marca `XER.OV` e `XER.SO` quando o divisor é zero; por segurança do executor, o resultado é definido como zero nesse caso. CI #104–#105 pendente.


### v1.68b — NEG / NEGO (validação pendente)
- NEG XO 104: negação em complemento de dois.
- NEGO XO 616: negação com overflow signed; atualiza XER.OV e mantém XER.SO sticky; Rc atualiza CR0.
- Testes cobrem operandos positivos, INT_MIN e preservação de SO após OV limpar.


### v1.69b — DIVW / DIVWO (validação pendente)
- DIVW XO 491 e DIVWO XO 1003 implementadas com divisão signed, atualização opcional de CR0 e detecção de casos de overflow na forma OE.
- Testes host incluem quociente negativo e INT_MIN / -1.


## v1.70b — ADDZEO

- Added `ADDZEO` (extended opcode XO 714): adds XER.CA to the RA source, writes RT, updates carry-out, signed OV and sticky SO, and honors Rc/CR0.
- Host coverage exercises positive-to-negative signed overflow, CA clearing, OV clearing on a subsequent non-overflowing operation, sticky SO, and CR0.
- This extends the integer interpreter only; it does not imply FPU, MMU, Flipper, DSP, disc/BIOS, functional XEX, or game compatibility.


## v1.71b — SUBFIC

- Added `SUBFIC` (primary opcode 8): computes the sign-extended immediate minus the RA source and writes RT.
- XER.CA is set when the unsigned subtraction has no borrow and cleared when a borrow occurs; unrelated XER bits, including sticky SO, are preserved.
- Host regression coverage checks both borrow outcomes and result values.
- This is an incremental integer-core improvement, not evidence of a complete emulator, functional XEX, RGH boot, or game compatibility.


## v1.72b — SUBFZE

- Added `SUBFZE` (extended opcode XO 200), computing `0 - RA - !XER.CA` and writing RT.
- Updates XER.CA for the no-borrow case and honors Rc/CR0.
- Host tests cover CA=1 and CA=0, result values, and CR0.
- The instruction-level tests do not establish complete GameCube emulation or functional Xbox 360 output.


### Additional SUBFZE boundary case

Added a regression case for `RA = 0xffffffff` with `CA = 0`: the low-word result wraps to zero while the operation still represents a borrow, so CA remains clear and CR0 reports EQ when SO is clear. This supplements the ordinary CA-set and CA-clear cases.


## v1.73b — SUBFME

- Added `SUBFME` (extended opcode XO 232): computes `~RA + XER.CA`, writes RT, and updates XER.CA from the 33-bit sum.
- Honors Rc/CR0 and preserves unrelated XER flags.
- Host tests cover CA=1 wrap/carry and a CA=0 negative result.
- The instruction remains part of a partial experimental interpreter; XEX/boot/game compatibility are not established.


### SUBFME test semantic correction

Corrected the SUBFME expected values to match the operation `~RA + 0xffffffff + XER.CA` (equivalent to subtract-from-minus-one with carry-in). For RA=0, CA=1 the low word is `0xffffffff`; for RA=1, CA=0 it is `0xfffffffd`. Both cases produce carry-out and a negative CR0 result. The host test, ASan/UBSan, diagnostic ELF32 build, core smoke build, and XEX preflight jobs all passed in CI run #124.

### Test additions after v1.73b (CI pending)
- Added host regression coverage for `SLW` and `SRW` with register shift counts of 1 and 32, checking result values and `CR0` updates.
- Added a host regression test for `MCRXR` moving `XER.SO/OV/CA` into CR field 2 and clearing those XER bits.
- Commits: `bc1d571` (shift test encoding correction) and `9ae6f29` (MCRXR test).
- These are test-only changes. They do not expand the emulator's implemented instruction set, and the new tests must not be considered validated until CI completes successfully.

### Additional CPU regression tests (CI pending)
- Commit `7e19765`: added `SRAW` boundary cases for shift counts 32 and 33, checking sign-fill, `XER.CA`, and `CR0`.
- Commit `617164a`: added `ADDO` tests for signed overflow setting `OV` and sticky `SO`, then clearing `OV` while preserving `SO`.
- These changes extend host-test coverage only. They do not add new CPU opcodes and are not considered validated until the corresponding CI checks complete.
