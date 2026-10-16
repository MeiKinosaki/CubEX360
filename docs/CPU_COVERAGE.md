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
