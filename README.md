# CubEX360 — toolchain-first development (v1.99b)

Projeto experimental de interpretador parcial PowerPC/Gekko para um futuro emulador de GameCube no Xbox 360.

## Build libXenon verificado
O workflow [run 38219582132](https://github.com/MeiKinosaki/CubEX360/actions/runs/38219582132) concluiu com sucesso e publicou o artefato `cubex360_toolchain_probe.elf32`.
- Formato: ELF32 PowerPC big-endian.
- SHA-256: `96d490d9c467eca735f571cfd8327d1838fbb23a791bbf9e64bac7d5c2648854`.
- O artefato é somente um probe de toolchain, não o núcleo do emulador.

## v1.31b — pacote local
O pacote de desenvolvimento v1.31b inclui relatório do ELF verificado, script de validação de formato/hash e gate de pré-verificação XEX. A suíte host passou com 21 testes; o gate XEX bloqueia corretamente porque ainda não há ferramentas XEX validadas configuradas.

**Nota de publicação:** o ZIP v1.31b foi criado e testado no ambiente de desenvolvimento, mas ainda não foi publicado integralmente no GitHub como release/anexo.

## Bloqueios
- O núcleo completo ainda não está integrado ao alvo libXenon do workflow.
- A toolchain/runtime XEX não foi identificada nem validada.
- `default.xex` não foi gerado nem testado em Xbox 360 RGH.

Consulte [status da toolchain](docs/TOOLCHAIN_STATUS.md), [relatório ELF32](docs/CI_ELF32_VERIFICATION.md) e [plano XEX](docs/DEFAULT_XEX.md).


## v1.32b — alvo de integração libXenon
Foi adicionado `core-src/` com um alvo inicial que liga memória principal de 24 MiB, barramento cached/uncached e interpretador limitado. O workflow agora tenta compilar esse alvo separadamente do probe de toolchain e publicar `cubex360_core_smoke.elf32`. O resultado só deve ser considerado confirmado após o job CI terminar e o artefato ser inspecionado.

O alvo suporta apenas um subconjunto de instruções PowerPC e é um smoke test, não um emulador completo. `default.xex` continua pendente.


## v1.35b — operações imediatas PowerPC adicionais

O interpretador parcial agora também implementa `ORIS`, `XORI` e `ANDIS.`, além de `ANDI.`. Os testes host foram expandidos para validar os valores resultantes e CR0 quando a instrução atualiza o campo de condição. Os commits mais recentes acionaram o CI; confira o estado antes de tratar essa revisão como aprovada.

- [CPU](core-src/src/cpu.c)
- [Testes](core-src/tests/test_core_host.c)
- [CI](https://github.com/MeiKinosaki/CubEX360/actions)

O núcleo continua parcial; `default.xex` ainda não foi gerado.


## v1.36b — correção de decodificação lógica

O CI encontrou uma falha nos testes das operações lógicas imediatas: a implementação usava incorretamente os campos de origem/destino nas instruções PPC. A CPU foi corrigida para seguir a codificação PowerPC (`rA` como fonte e `rS/rT` como destino nas instruções lógicas imediatas), e as palavras de máquina nos testes foram corrigidas. As novas execuções do CI estão em andamento; não marcar como aprovado até terminar.

- [Implementação](core-src/src/cpu.c)
- [Testes corrigidos](core-src/tests/test_core_host.c)
- [GitHub Actions](https://github.com/MeiKinosaki/CubEX360/actions)

O núcleo continua parcial; `default.xex` ainda não foi gerado.


## v1.40b — operações aritméticas imediatas e sanitizers

Adicionado suporte experimental para `MULLI`, `ADDIC` e `ADDIC.`, com casos de regressão no teste nativo. O workflow também ganhou um job com AddressSanitizer e UndefinedBehaviorSanitizer para detectar acessos inválidos e comportamento indefinido no núcleo C.

**Importante:** aguardar a conclusão do CI para validar esta revisão. O núcleo ainda é parcial e `default.xex` ainda não foi gerado.


## v1.41b — load/store e segurança de execução

O interpretador parcial recebeu suporte experimental para `LBZ`, `LHZ`, `LHA`, `STB` e `STH`, mantendo `LWZ` e `STW`. Os testes cobrem carregamento sem sinal/com sinal, gravação de byte/meia-palavra, falha de acesso desalinhado e rejeição segura de opcode não suportado. Também foram corrigidos casos de aliasing nos testes host e a semântica de operandos assinados em `MULLI` e do registrador-fonte em `ADDIC`.

- [Código da CPU](core-src/src/cpu.c)
- [Testes de regressão](core-src/tests/test_core_host.c)
- [Execuções de CI](https://github.com/MeiKinosaki/CubEX360/actions)

A revisão está aguardando o resultado do CI. O job libXenon atual gera um ELF32 de smoke test, não um executável integrado do emulador. Não há ferramenta XEX validada configurada, portanto `default.xex` permanece bloqueado e não deve ser simulado renomeando o ELF.


## v1.42b — desvio condicional BC

Adicionado suporte experimental ao opcode PowerPC `BC`, com avaliação de condição CR e decremento/avaliação CTR, além de atualização de LR quando LK=1. O teste de regressão verifica o desvio quando CR0.EQ está ativo e confirma que a instrução no caminho ignorado não altera o estado. Uma falha no sanitizer revelou que a codificação de deslocamento do teste usava BD=8 (32 bytes); corrigida para BD=2 (8 bytes). Aguardando CI da revisão.


### Cobertura adicional de BC
Os testes também verificam o caminho não tomado quando CR0.EQ está falso e a semântica de `BCL` para atualizar LR. O deslocamento BD dos desvios é codificado já alinhado em bits; o teste usa BD=2 para representar um salto de 8 bytes. A revisão aguarda CI.


## v1.44b — retorno por LR e desvios via CTR

Adicionado suporte experimental a `BCLR` (incluindo a forma comum `BLR`) e testes para retorno através de LR. Os testes também cobrem `BC` condicionado por CTR, verificando decremento e salto quando CTR permanece diferente de zero. A revisão aguarda CI. O suporte ainda não equivale a compatibilidade completa com PowerPC/Gekko.


A cobertura de desvios indiretos foi ampliada: `BCTR` agora usa o destino em CTR sem decrementá-lo, e o teste confirma que a instrução no caminho sequencial é ignorada. Esta alteração também aguarda a execução do CI.


## v1.44b — desvios indiretos PowerPC

Além de `BCLR/BLR`, o núcleo agora implementa a forma `BCCTR/BCTR` e preserva CTR durante o salto indireto. Testes verificam o destino em CTR, o salto sobre a instrução sequencial e o valor não decrementado de CTR. CI da revisão pendente.


## v1.45b — comparação imediata e SPR LR/CTR

Adicionado suporte experimental a `CMPWI`/`CMPLWI` (formas imediatas de comparação assinada e não assinada) e a `MFSPR`/`MTSPR` para os SPRs LR e CTR. Foram incluídos testes de atualização do campo CR especificado e de leitura/escrita dos registradores especiais. Uma revisão dos testes corrigiu as máscaras usadas para verificar CR2 e CR3. O CI desta revisão precisa terminar antes de considerá-la aprovada.

- [CPU](core-src/src/cpu.c)
- [Testes host](core-src/tests/test_core_host.c)
- [GitHub Actions](https://github.com/MeiKinosaki/CubEX360/actions)

O suporte continua parcial: instruções SPR não implementadas são rejeitadas, e `default.xex` ainda não foi gerado.


## v1.46b — comparação entre registradores

Ampliado o interpretador com as formas PowerPC `CMPW` e `CMPLW` para comparação assinada e não assinada entre GPRs, com seleção do campo CR de destino. Os testes usam um operando negativo para distinguir a comparação assinada (LT) da não assinada (GT). O CI ainda precisa confirmar a revisão atual.

- [Implementação](core-src/src/cpu.c)
- [Testes](core-src/tests/test_core_host.c)
- [GitHub Actions](https://github.com/MeiKinosaki/CubEX360/actions)


## v1.47b — RLWINM

Adicionado o opcode PowerPC `RLWINM` (rotate left word immediate then AND with mask), com máscara MB/ME que trata corretamente máscaras que atravessam o limite de bits. Inclui testes de rotação de 8 bits com máscara e extração dos 8 bits baixos. CI desta revisão pendente.

- [Implementação da CPU](core-src/src/cpu.c)
- [Regressões](core-src/tests/test_core_host.c)
- [GitHub Actions](https://github.com/MeiKinosaki/CubEX360/actions)


## v1.48b — RLWIMI, acesso ao barramento e execução limitada

- Adicionado `RLWIMI` (rotate left word immediate then mask insert), preservando os bits do destino fora da máscara.
- O barramento de RAM principal agora expõe acessos de 8, 16 e 32 bits para as regiões cached/uncached; loads/stores de byte e meia-palavra da CPU passam por essas funções.
- Adicionado `cubex_cpu_run`, que executa um orçamento limitado de instruções e propaga erros de memória/opcode em vez de continuar silenciosamente.
- O alvo `main.c` agora testa uma sequência de duas instruções usando a rotina de execução; testes host cobrem também os aliases do barramento e `RLWIMI`.
- O workflow inclui um gate que confirma que a geração XEX continua bloqueada sem ferramentas reais configuradas.

**Validação:** aguardar o GitHub Actions do commit atual. O build libXenon segue sendo um ELF32 de smoke test do núcleo, não uma aplicação XEX pronta. Não há evidência de `default.xex` gerado, inicialização em Xbox 360 RGH, GPU/áudio/GameCube IPL ou compatibilidade com jogos.

Consulte [o gate de XEX](docs/DEFAULT_XEX.md) e [a execução de CI](https://github.com/MeiKinosaki/CubEX360/actions).


## v1.49b — deslocamentos PowerPC/Gekko

A CPU parcial ganhou as instruções inteiras `SLW`, `SRW` e `SRAWI`. Os deslocamentos lógicos zeram o resultado quando o bit de faixa do registrador de deslocamento está ativo; `SRAWI` propaga o sinal sem depender de deslocamento à direita de inteiro negativo definido pela implementação, atualiza o carry do XER e, na forma com Rc, atualiza CR0. Foram acrescentadas regressões para deslocamentos, carry e atualização de condição.

**Escopo honesto:** isso amplia o subconjunto interpretado, não representa 60% de compatibilidade total da CPU Gekko. Ainda faltam muitas instruções, exceções, MMU, FPU/paired-single, timers/interrupções e integração completa do sistema. `default.xex` segue não gerado.


## v1.51b — RLWNM e cobertura de rotação

Adicionado `RLWNM` (rotate left word then AND with mask), usando o deslocamento em registrador e a máscara MB/ME. O teste valida rotação de 8 bits e extração do campo mascarado. A suíte de testes agora inclui também os deslocamentos `SLW`, `SRW` e `SRAWI` da versão anterior.

**Validação da versão:** a execução de CI para esta alteração deve ser consultada antes de declarar os testes aprovados. A CPU continua sendo um subconjunto parcial de Gekko; isso não significa 60% de compatibilidade total. O ELF32 de smoke test não é um `default.xex`; a geração e execução em Xbox 360 continuam pendentes.


## v1.51b — expansão da CPU e foco de integração

Expansão do interpretador PowerPC/Gekko com `XORIS`, `CNTLZW`, `ANDC`, `NOR`, `EXTSB` e `EXTSH`, com testes de resultado para cada operação. O CI também deve continuar verificando o alvo integrado de smoke compilado com Free60/libXenon.

**Toolchain/XEX:** a toolchain libXenon disponível no CI gera ELF32 PowerPC big-endian. Isso comprova compilação/linkedição de baixo nível, não geração de XEX. `default.xex` segue bloqueado até selecionar e validar uma toolchain/runtime que produza XEX real com ABI, imports e entry point compatíveis.


## Estado atual — v1.56b
- `MFCR` e `MTCRF` passaram no CI da v1.55b.
- A v1.56b adiciona operações lógicas sobre bits do registrador CR. O workflow final da correção de testes está verde; veja [GitHub Actions](https://github.com/MeiKinosaki/CubEX360/actions).
- A toolchain em nuvem compila o núcleo experimental para ELF32 PowerPC big-endian com Free60/libXenon.
- **`default.xex` ainda não foi gerado.** O ELF atual não deve ser renomeado nem tratado como XEX. A rota de build XEX exige um ambiente Windows com toolchain Xbox 360/XDK autorizado e integração de runtime/ABI/imports ainda pendente.
- Plano de nuvem: [docs/CLOUD_TOOLCHAIN.md](docs/CLOUD_TOOLCHAIN.md); estado XEX: [docs/DEFAULT_XEX.md](docs/DEFAULT_XEX.md).

## Estado de desenvolvimento — v1.58b
- A suíte host agora inclui regressões para limites da RAM, alinhamento e falha de fetch em endereço não mapeado.
- A implementação `MCRXR` foi integrada na v1.57b; os workflows correspondentes passaram.
- O workflow XEX foi endurecido para detectar códigos de saída não zero do script de build.
- **Validação da v1.58b pendente:** conferir os resultados em [GitHub Actions](https://github.com/MeiKinosaki/CubEX360/actions).
- **`default.xex` do emulador completo não gerado.** O ELF de smoke test não é o executável final.
- Roadmap de build: [CLOUD_TOOLCHAIN.md](docs/CLOUD_TOOLCHAIN.md); histórico: [DEVELOPMENT_HISTORY.md](docs/DEVELOPMENT_HISTORY.md).
## Desenvolvimento atual — v1.59b
- A v1.58b passou no CI para `ADDC`/`SUBFC` e testes de memória.
- A v1.59b adiciona `ADDE`, `SUBFE`, `ADDME` e `SUBFZE` com testes de regressão; CI ainda pendente.
- O núcleo continua um interpretador PowerPC parcial. GPU, DSP, drive, IPL, FPU e MMU não estão completos.
- O build ELF32 é um smoke target; `default.xex` funcional ainda não foi gerado.
- Acompanhe o [CI](https://github.com/MeiKinosaki/CubEX360/actions) e leia o [status da toolchain](docs/TOOLCHAIN_STATUS.md).

## Desenvolvimento atual — v1.61b (CI pendente)
- A v1.60b foi corrigida e o workflow #87 terminou com sucesso: [resultado](https://github.com/MeiKinosaki/CubEX360/actions/runs/38720276159).
- A v1.61b adiciona `ADDO` e `SUBFO` para semântica de overflow assinado no XER, com regressões para OV/SO e CR0.
- Os workflows #88 (implementação) e #89 (testes) ainda precisam terminar antes de validar a revisão.
- FPU, MMU, GPU Flipper, DSP, unidade de disco, IPL/BIOS e compatibilidade com jogos seguem incompletos. O ELF de smoke test não é `default.xex`.


## Auditoria da CPU
O inventário de opcodes presentes e limitações arquiteturais está em [docs/CPU_COVERAGE.md](docs/CPU_COVERAGE.md). A validação v1.61b ainda está em curso; não interpretar o inventário como cobertura total de Gekko ou compatibilidade com jogos.


## Desenvolvimento atual — v1.62b
- O run #92 confirmou a validação do teste de `ADDO.`/`CR0` na v1.61b: [resultado](https://github.com/MeiKinosaki/CubEX360/actions/runs/38723342332).
- A v1.62b adiciona `MULLW` e testes de resultado truncado; a execução #93 ainda está em andamento: [CI](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947391104).
- Ainda não há `default.xex` funcional nem compatibilidade de jogos comprovada.


## Desenvolvimento atual — v1.63b
- `MULLW` foi adicionada na v1.62b; run #93 passou e a validação do teste está em andamento: [run #94](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947407729).
- A v1.63b adiciona `SRAW` com teste de `XER.CA` e `CR0`: [implementação #95](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947472203), [testes #96](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947492785).
- Não há `default.xex` funcional nem compatibilidade com jogos comprovada.


## Desenvolvimento atual — v1.64b
- `MULLW` foi validada na execução #94: [CI](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947407729).
- `SRAW` está em validação nas execuções [#95](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947472203) e [#96](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947492785).
- A v1.64b adiciona `MULHWU`, multiplicação unsigned com resultado na palavra superior; CI pendente: [implementação #97](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947550859), [testes #98](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947566701).


**Aviso de validação:** a execução #96 detectou campos de registrador invertidos no teste de `SRAW`; correção enviada em [commit](https://github.com/MeiKinosaki/CubEX360/commit/63f634aa805505185ef4faf72db953ed21abd384), CI #99 pendente. A execução #97 da `MULHWU` falhou pela suíte que continha esse teste; aguardar #98/#99 antes de declarar as versões aprovadas.


## Desenvolvimento atual — v1.65b
- Correção do teste SRAW (campos fonte/destino): [commit](https://github.com/MeiKinosaki/CubEX360/commit/63f634aa805505185ef4faf72db953ed21abd384), [CI #99](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947638330).
- A v1.64b adiciona `MULHWU`; a v1.65b adiciona `MULHW` para palavra superior signed do produto. CI #100–#101 pendente: [implementação](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947712923), [testes](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947726589).

**Atualização CI:** a execução #99 passou em todos os jobs após corrigir a codificação dos registradores de `SRAW`; ela valida a revisão com `SRAW` e `MULHWU`. `MULHW` v1.65b é posterior e segue pendente nas execuções #100–#101.


**Validação concluída:** as execuções [#100](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947712923) e [#101](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947726589) passaram em todos os jobs para `MULHW` v1.65b. A correção do teste SRAW também passou na [execução #99](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947638330). O executável final `default.xex` ainda não está comprovado.


## Desenvolvimento atual — v1.66b
- A v1.65b `MULHW` passou em todos os jobs: [implementação #100](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947712923), [testes #101](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947726589).
- A v1.66b adiciona `MULLWO` com detecção de overflow signed e testes de `XER.OV`/`XER.SO`; CI pendente: [#102](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947885203), [#103](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947902411).


## Atualização v1.67b
- `MULLWO` v1.66b validada pelos [runs #102](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947885203) e [#103](https://github.com/MeiKinosaki/CubEX360/actions/runs/38947902411).
- A v1.67b adiciona `DIVWU` e `DIVWUO`, com testes de quociente e divisor zero. CI pendente: [implementação #104](https://github.com/MeiKinosaki/CubEX360/actions/runs/39009708417), [testes #105](https://github.com/MeiKinosaki/CubEX360/actions/runs/39009715986).
- Divisão por zero tem resultado provisório zero no núcleo experimental; não é prova de compatibilidade com o hardware original. `default.xex` funcional/boot RGH continuam não comprovados.


## Atualização v1.68b — NEG / NEGO
- A v1.67b foi corrigida: o run #107 passou em todos os jobs: https://github.com/MeiKinosaki/CubEX360/actions/runs/39010301838
- A v1.68b adiciona NEG e NEGO com testes para overflow e CR0. A validação CI desta nova revisão está pendente.
- O projeto continua sendo um núcleo experimental parcial; default.xex funcional, boot no RGH e compatibilidade de jogos não foram comprovados.


## Atualização v1.69b — DIVW / DIVWO
- A v1.68b passou em todos os jobs no run #111: https://github.com/MeiKinosaki/CubEX360/actions/runs/39010540651
- A v1.69b adiciona divisão signed DIVW/DIVWO e testes de borda; CI ainda pendente.
- O núcleo segue experimental: `default.xex` funcional, boot RGH e compatibilidade de jogos não comprovados.


### v1.70b development note

The experimental Gekko integer core now includes an `ADDZEO` implementation and host regression coverage for carry, signed overflow, sticky summary overflow, and CR0 updates. Check the latest GitHub Actions run before considering this revision validated. This is not a claim of a working GameCube emulator, `default.xex`, RGH boot, or game compatibility.


### v1.71b development note

`SUBFIC` has been added to the experimental Gekko integer interpreter, with regression tests for borrow/no-borrow carry behavior and preservation of XER state. Check the latest GitHub Actions run for validation. No claim is made that a functional `default.xex`, Xbox 360 RGH boot, or playable GameCube title has been proven.


### v1.72b development note

The experimental Gekko integer interpreter now includes `SUBFZE` and host tests for XER.CA and CR0 behavior. Validation status is available in GitHub Actions. A working `default.xex`, RGH boot, and game compatibility remain unproven.


### v1.73b development note

Added `SUBFME` to the partial Gekko integer interpreter, with host tests for carry and CR0 behavior. Check GitHub Actions for validation status. No working `default.xex`, RGH boot, or game compatibility is claimed.


SUBFME regression expectations were corrected to the full subtract-from-minus-one arithmetic and are awaiting CI revalidation.


## v1.74b — variantes aritméticas do núcleo Gekko

O interpretador recebeu as variantes com overflow `ADDEO`, `SUBFCO` e `SUBFEO`, e testes host para verificar resultado, carry, overflow assinado, SO sticky e CR0. O CI desses commits ainda não foi confirmado; não considerar os testes aprovados até a execução terminar.

- [Implementação da CPU](core-src/src/cpu.c)
- [Testes host](core-src/tests/test_core_host.c)
- [Inventário de cobertura](docs/CPU_COVERAGE.md)

**Estado real:** avanço de código experimental na versão de desenvolvimento v1.74b. FPU/paired-single, MMU/TLB, exceções completas, DSP, vídeo Flipper, drive/IPL e integração total continuam pendentes. `default.xex` funcional e boot no Xbox 360 RGH não foram comprovados.


## v1.75b — DCBZ e testes de memória

O núcleo recebeu suporte experimental a `DCBZ`, que zera o bloco alinhado de 32 bytes na RAM principal mapeada, além de testes para alinhamento, conteúdo zerado e endereço inválido. A validação do GitHub Actions ainda está pendente.

- [Implementação](core-src/src/cpu.c)
- [Testes host](core-src/tests/test_core_host.c)
- [Cobertura e limitações](docs/CPU_COVERAGE.md)

Esta versão não representa um emulador completo: cache real/coerência, FPU, MMU/TLB, exceções, GPU, DSP, drive/IPL e integração ainda precisam de trabalho. Não há `default.xex` funcional nem boot no RGH comprovados.


## v1.76b — instruções de memória com atualização

Foram adicionadas formas experimentais `LWZU`, `LBZU`, `STWU`, `STBU`, `LHZU`, `LHAU` e `STHU`. Os testes host cobrem cargas/armazenamentos selecionados e atualização do registrador-base. O CI ainda precisa validar esta etapa.

- [CPU](core-src/src/cpu.c)
- [Testes](core-src/tests/test_core_host.c)
- [Cobertura e limitações](docs/CPU_COVERAGE.md)

A memória permanece um modelo simplificado; não equivale ao mapa de hardware completo do GameCube. Não há `default.xex` funcional nem boot no RGH comprovados.


## v1.77b — transferências múltiplas LMW/STMW

A CPU experimental agora implementa `LMW` e `STMW` para transferir vários GPRs entre a RAM mapeada e o banco de registradores. Os testes cobrem r28–r31, ordem big-endian e endereço desalinhado. A validação CI ainda está pendente.

- [Implementação da CPU](core-src/src/cpu.c)
- [Testes host](core-src/tests/test_core_host.c)
- [Inventário de cobertura](docs/CPU_COVERAGE.md)

O núcleo e os periféricos permanecem incompletos; não há `default.xex` funcional nem boot no RGH comprovados.


## v1.78b — acessos indexados com atualização

Adicionadas as instruções experimentais `LWZUX`, `LBZUX`, `STWUX` e `STBUX`, com cálculo de endereço RA+RB e atualização da base após sucesso. Os testes host cobrem os acessos por palavra/byte e a atualização do registrador-base. CI pendente.

- [CPU](core-src/src/cpu.c)
- [Testes host](core-src/tests/test_core_host.c)
- [Cobertura](docs/CPU_COVERAGE.md)

O núcleo continua parcial e o boot de um `default.xex` no Xbox 360 RGH não foi comprovado.


## v1.79b — instruções indexadas de memória

Foram adicionadas as instruções experimentais `LWZX`, `LBZX`, `STWX`, `STBX`, `LHZX`, `LHAX` e `STHX`, com testes host para acesso por palavra/byte/halfword, extensão de sinal e preservação de RA. O CI ainda precisa validar esta etapa.

- [CPU](core-src/src/cpu.c)
- [Testes host](core-src/tests/test_core_host.c)
- [Cobertura](docs/CPU_COVERAGE.md)

O núcleo e os periféricos seguem parciais; `default.xex` funcional e boot no RGH não foram comprovados.


## v1.80b — save/load state e estrutura da biblioteca

- API experimental de save/load para estado da CPU + 24 MiB de RAM, com versão de formato e CRC32.
- Teste host de round-trip adicionado; workflow host/sanitizer agora inclui `core-src/src/state.c`.
- Estrutura inicial de biblioteca/favoritos em `frontend/library.json`; diretórios documentados para ROMs, capas e save states.
- Especificação da futura interface estilo Aurora e tela de abertura: [documentação](docs/FRONTEND_STATE_V1.80B.md).

**Limites:** os save states não incluem GPU/DSP/disco/controladores; não há UI funcional, carregador de ROM, música/tela de abertura no runtime nem `default.xex` funcional comprovado. O ELF32 atual é apenas um smoke test da toolchain. A música original oficial do GameCube não é distribuída; poderá ser usado um arquivo fornecido legalmente pelo usuário.


## v1.81b — download automático de capas (estrutura)

A biblioteca agora tem configuração de download automático de capas PNG por ID do jogo e uma API de cache que evita baixar novamente arquivos existentes. O teste host usa transporte simulado; não confirma acesso à Internet. Para funcionar no Xbox 360, o frontend precisa implementar o callback HTTP usando a rede Ethernet/Wi-Fi disponível e configurar um servidor de capas legítimo.

- [API de capas](core-src/include/cubex360/covers.h)
- [Cache/download callback](core-src/src/covers.c)
- [Configuração da biblioteca](frontend/library.json)
- [Notas de frontend](docs/FRONTEND_STATE_V1.80B.md)

O URL de servidor é um placeholder. Downloads reais, renderização das capas e suporte de rede no console ainda não estão validados.


## v1.82b — perfis gráficos, filtros e sequência de abertura (configuração)

- `frontend/settings.json`: perfis configuráveis Quality/Balanced/Performance, opções de abertura e espaço para substituições por jogo.
- `frontend/filters.json`: filtros planejados para favoritos, recentes, capas ausentes/presentes, estado de compatibilidade e gêneros, além de pesquisa/ordenação.
- `frontend/per_game_profile.schema.json`: modelo de ajustes individuais por Game ID.
- `assets/boot/boot_sequence.json`: sequência de abertura com transição para a biblioteca e áudio opcional fornecido pelo usuário.

**Importante:** são configurações e esquemas; os menus gráficos, persistência de ajustes, filtros reais e controles de renderização ainda precisam ser implementados no frontend/runtime. As opções de qualidade/desempenho só terão efeito quando o renderer suportar cada opção. A tela de abertura e o áudio também não estão implementados no runtime. Nenhuma música oficial da Nintendo é incluída.


## v1.83b — CPU: indexed halfword update operations

Added experimental Gekko interpreter cases for `LHZUX`, `LHAUX` and `STHUX`, with host regression tests for memory values, signed halfword extension and RA update. The implementation and tests are committed, but CI has no published status for these commits yet. See [CPU coverage notes](docs/CPU_COVERAGE.md).

This does not change the status of the frontend: settings, filters, cover-download callback and boot sequence are still scaffolding until connected to the real Xbox 360 runtime. The final `default.xex` and boot on RGH remain unverified.


## v1.84b — desenvolvimento modular e CI unificada

- Documentada a divisão em cinco frentes independentes: núcleo, hardware, Xbox 360, interface/persistência e integração/validação.
- Adicionado `scripts/run-host-tests.sh` com modos `strict` e `sanitize` para tornar a execução local e a do CI reproduzíveis.
- O workflow `xenon-build.yml` agora chama o script comum nos jobs de testes host, reduzindo duplicação de comandos.
- Critérios objetivos definidos: cada mudança deve ser validada pelo SHA exato; teste host, ELF32 e XEX são marcos diferentes; nenhuma etapa é considerada funcional sem evidência apropriada.

O script foi registrado, mas não foi executado neste ambiente. O CI precisa confirmar a execução. A arquitetura modular documenta frentes de trabalho; não significa que todos os subsistemas de hardware ou frontend já existam.


## v1.85b — validação de configuração da interface

- Adicionado `scripts/validate-config.sh`, que analisa os JSONs da biblioteca, perfis, filtros e sequência de abertura e verifica os perfis obrigatórios.
- GitHub Actions ganha um job separado para validar esses arquivos e passa a disparar com alterações em `frontend/**` e `assets/boot/**`.
- Isso valida a integridade estrutural das configurações, não implementa por si só a interface gráfica nem prova download de capas, áudio ou execução no console.


## v1.86b — API de transferências em bloco para memória

- Adicionadas `cubex_memory_copy_to` e `cubex_memory_copy_from` para transferências de bytes limitadas à RAM principal.
- A implementação verifica limites antes de copiar e usa `memmove`, permitindo sobreposição segura dos intervalos.
- Testes host cobrem ida/volta, limite da RAM, ponteiros nulos e transferências vazias.
- Uso futuro previsto em DMA, leitura/escrita de estados e inicialização de buffers; estes subsistemas ainda precisam integrar a API.

Os commits de código e teste não têm status de CI publicado verificado nesta etapa. Não declarar os testes aprovados até a execução do workflow para o SHA correspondente.


## v1.87b — regressões de cópia sobreposta

- Ampliados os testes da API de memória para verificar cópias com regiões de origem e destino sobrepostas, tanto no sentido de deslocamento para a frente como para trás.
- Os casos exercitam a semântica esperada de `memmove`; não adicionam DMA nem integração de save/load state.
- Commit dos testes: `b954c77e4b4e0ca9d31e12a40f988dd022fe1ce4`.

**Validação:** o resultado de GitHub Actions para este SHA ainda não foi confirmado. Não declarar os testes aprovados até consultar o CI. `default.xex` funcional, boot em Xbox 360 RGH e compatibilidade com jogos continuam não comprovados.


## v1.88b — transferências em bloco pelo barramento

- Adicionadas `cubex_bus_copy_to` e `cubex_bus_copy_from` para transferências limitadas através dos endereços físicos, cached e uncached da RAM principal.
- Testes acrescentados para leitura cruzada entre aliases, validação de limites e rejeição de regiões não mapeadas.
- Commits principais: `bbab540eb01cd57ed074b40fddf6bd8b175b9190`, `b6375b6fa6634fc3567149ffdfb23f70300962b6` e `50196f5507e8b10977991c6766a76a3f4fb8910b`.

**Validação:** o estado do GitHub Actions ainda precisa ser confirmado para os commits atuais. Esta API prepara uma interface para futuros consumidores de DMA, mas não implementa um controlador DMA de GameCube. `default.xex` funcional, boot no Xbox 360 RGH e compatibilidade com jogos continuam não comprovados.


## v1.89b — integridade e falha segura de save states

- Expandida a regressão host de save/load state: o teste corrompe um byte do bloco de RAM salvo e exige rejeição pelo checksum.
- O teste também verifica que uma carga inválida não modifica parcialmente os registradores da CPU nem a RAM que já estava ativa.
- Commit do teste: `8e3bbe609debf7414eb9e77f263c972075e80bbc`.

**Validação:** o CI para o SHA desta alteração ainda precisa ser confirmado. Os estados continuam abrangendo somente CPU e RAM principal, não GPU/DSP/timers/disco. `default.xex` funcional e boot no RGH permanecem não comprovados.


## Desenvolvimento atual — v1.94b (RFI e Program Exception)

- O caminho de exceção de programa foi organizado para registrar a causa em SRR1: instrução ilegal e instrução privilegiada.
- `RFI` agora verifica o modo privilegiado antes de restaurar SRR0/SRR1; em modo usuário, gera Program Exception em vez de restaurar contexto.
- Testes host cobrem RFI em supervisor (alinhamento do PC e restauração do MSR) e tentativa em modo usuário (vetor `0x700` e causa em SRR1).
- Commits: `89ad39d` (implementação) e `6ccd973` (testes).
- **Validação concluída:** todos os seis jobs passaram nos workflows [#309](https://github.com/MeiKinosaki/CubEX360/actions/runs/43627224216) e [#310](https://github.com/MeiKinosaki/CubEX360/actions/runs/43627227898), incluindo host-core-test, ASan/UBSan, build-core-smoke e build-probe. Isso valida a revisão de código testada; não significa que um `default.xex` final foi produzido.
- Permanecem pendentes o tratamento completo de todas as exceções, a semântica detalhada de MSR/SRR1 por tipo de exceção, MMU/TLB, FPU, Flipper/GPU, DSP, drive/IPL e integração de jogo. Não há `default.xex` funcional, boot RGH ou compatibilidade de jogos comprovados.


## Desenvolvimento — v1.95b (registradores de segmento Gekko; CI pendente)

- Estado da CPU ampliado com os 16 registradores de segmento (SR0–SR15).
- Interpretador recebeu instruções privilegiadas `MTSR`, `MFSR`, `MTSRIN` e `MFSRIN`; acessos em modo usuário geram Program Exception.
- Formato de save state avançou para versão 5, preservando leitura dos formatos v1–v4 e incluindo SR0–SR15 no novo formato.
- Testes host adicionados para acesso direto/indireto, privilégio e round-trip dos registradores SR.
- Commits: CPU state `95d5a3b`, execução `7121247`, estado `889d0ab`, testes `2f7a0ad` e ajuste de offset `2f7a0ad`.

**Validação pendente:** os novos testes e sanitizers ainda precisam passar no GitHub Actions para esta revisão. Isto ainda não implementa tradução de endereços por segmentos, BAT, SDR1/PTE ou MMU completa. `default.xex`, boot RGH e compatibilidade com jogos seguem não comprovados.


## Desenvolvimento — v1.96b (registradores BAT do Gekko; tradução ainda pendente)

- Adicionados IBAT0U–IBAT3L e DBAT0U–DBAT3L ao estado da CPU.
- MFSpr/MTSpr leem e escrevem SPRs BAT 528–543 em modo supervisor; modo usuário gera Program Exception.
- Save state avança para v6, persistindo os 16 registradores BAT e mantendo leitura de v1–v5.
- Testes host cobrem escrita/leitura de IBAT0U, bloqueio em modo usuário e round-trip dos BATs.
- Limite: ainda não há comparação BAT, tradução EA→PA, permissões WIMG/proteção ou MMU integrada ao fetch/load/store.
- default.xex, boot RGH e execução de jogos continuam não comprovados.


## v1.97b — lookup experimental de BAT

- Adicionada função isolada de lookup BAT para IBAT/DBAT: compara BEPI/BL, valida Vs/Vu e forma o endereço físico a partir de BRPN + deslocamento.
- Testes host cobrem bloco de 128 KiB, seleção instruction/data, privilégio supervisor/usuário e miss.
- Não integrado ao fetch/load/store: o barramento ainda recebe endereços efetivos diretamente; page table via SDR1, proteção PP, WIMG, exceções de tradução e aliases/cache seguem pendentes. Não contar MMU como funcional.
- default.xex, boot RGH e jogos continuam não comprovados.


## v1.98b — BAT ligado ao caminho de memória

- Fetch de instrução e acessos escalares load/store agora passam por wrappers de endereço efetivo→físico quando MSR[IR]/MSR[DR] estão ativos.
- Com relocação desativada, preserva o caminho físico atual; com relocação ativa, exige hit em BAT nesta etapa.
- Testes host cobrem fetch por IBAT, load por DBAT e miss de DBAT gerando Data Storage.
- Limitações: sem page-table/SDR1 fallback, proteção PP, atributos WIMG, exceção Instruction Storage específica para miss IBAT e tradução de operações especiais/bulk; MMU ainda parcial.
- XEX/boot/jogos não comprovados.


## v1.99b — pipeline Linux para candidato default.xex

- O workflow #333 passou nos seis jobs: host-core-test, host-sanitizers, xex-preflight-gate, frontend-config-validation, build-core-smoke e build-probe. Isso valida a revisão v1.98b no SHA testado.
- Adicionado .github/workflows/default-xex-cloud.yml: runner GitHub-hosted Ubuntu, checkout do xsdk aberto e fixado no commit ade2c81d914634834d77394c5d9f8c4575f395bc, construção da toolchain powerpc-xsdk-elf, link do runtime xsdk, compilação do alvo CMake CubEX360 e empacotamento ELF→XEX2.
- Adicionado xex-target/ com um título mínimo que liga memory.c, bus.c e cpu.c, executa duas instruções e registra o resultado. É um candidato inicial de bring-up do núcleo, não o emulador GameCube completo.
- O workflow valida o contêiner XEX2, calcula SHA-256 e publica default.xex como artefato de Actions.
- Status: pipeline criado, ainda sem execução validada. Não há arquivo default.xex publicado, nem boot RGH ou compatibilidade de jogos comprovados até o workflow terminar e o artefato ser inspecionado no console.
- Referência da toolchain: https://github.com/xenosworks/xsdk (MIT; commit fixado no workflow).

## v2.0b — regressões de exceções com vetor alto

- Ampliados os testes host para verificar seleção dos vetores de Data Storage, Alignment e Program Exception com MSR[IP] ativo.
- Adicionados casos para interrupção externa: bloqueio quando MSR[EE] está desligado, entrada pelo vetor alto quando habilitado, salvamento de SRR0/SRR1 e limpeza de EE.
- Adicionados casos para decrementer: vetor alto, salvamento de contexto e limpeza de DEC pending.
- Commit dos testes: `382b9cce871180c9a15ee0d0babb49dd6b18bdd8`.

**Estado de validação:** os workflows host, sanitizers e builds foram disparados para este SHA e ainda estão na fila. Portanto, os testes estão escritos, mas ainda não estão aprovados. A porcentagem de 60% representa estimativa de implementação do projeto, não compatibilidade de jogos nem conclusão do núcleo de exceções.

**Pendências:** confirmar CI; corrigir qualquer falha; completar auditoria de SRR1/RFI e permissões BAT; implementar SDR1/page tables/TLB; desenvolver GX FIFO/command processor; concluir o alvo `default.xex` e verificar boot no RGH. Nenhum jogo GameCube está comprovadamente executável nesta revisão.


## v2.1b — primeiro módulo de fila GX FIFO limitada

- Criada uma fila circular de palavras de 32 bits para preparar o caminho de escrita do GX FIFO: capacidade fixa de 256 palavras, índices circulares, contagem, detecção de cheia/vazia e sinalização de overflow.
- Adicionados testes host para FIFO vazia, ordem FIFO, capacidade/overflow, esvaziamento e wrap-around do buffer.
- Os testes foram ligados aos modos strict e ASan/UBSan de `scripts/run-host-tests.sh`.
- Commits: módulo `c5f17e0` / `0c6f2b6`, testes `7b0ad18`, integração CI `91e15ea`.

**Limite importante:** este é somente o contêiner de fila bounded. Ainda não está conectado ao endereço físico de escrita GX FIFO `0xCC008000`, ao CP/GP command processor, à rasterização, ao EFB/XFB ou ao sincronismo da GPU. Não representa GPU GameCube funcional nem execução de comandos GX. MMU/TLB, FPU e exceções permanecem parciais.

**Validação:** os novos jobs strict e ASan/UBSan foram disparados pelo commit de integração; aguardam resultado. Os workflows de XEX também estão em andamento, sem artefato `default.xex` confirmado nesta revisão. Sem boot RGH ou jogos comprovados.

**Progresso estimado:** 61% de implementação geral. O aumento de 60% para 61% corresponde exclusivamente ao módulo de fila GX FIFO e sua cobertura de teste adicionada; não significa avanço equivalente na GPU completa ou na compatibilidade de jogos.

## v2.2b — integração GX FIFO validada por CI

- A FIFO GX foi conectada ao barramento Flipper no endereço de escrita `0xCC008000`; palavras de 32 bits são enfileiradas e podem ser consumidas pela API interna.
- O teste de integração verifica duas escritas consecutivas, ordem de leitura, porta não legível e comportamento da fila vazia.
- Correção dos scripts strict e ASan/UBSan para ligar `gx_fifo.c` aos alvos de integração.
- **Validação concluída no commit `b6c28291659f91301a637533d6df6d86a35d9cc9`:** os seis jobs do workflow [#43646955709](https://github.com/MeiKinosaki/CubEX360/actions/runs/43646955709) passaram: host-core-test, host-sanitizers, xex-preflight-gate, frontend-config-validation, build-probe e build-core-smoke.
- Artefatos publicados: `cubex360-libxenon-elf32` (SHA-256 `9b70f082b36207e20d387e8ba42a3500b3419d431e2287154a4a958fa7863c2e`) e `cubex360-core-smoke-elf32` (SHA-256 `84aa846b8f1f09a81fde48c95618b75a88faa3d8b893b5003b883387cd7e4298`). São ELF32 de probe/smoke test, não o executável final.

**Limites que continuam abertos:** a fila ainda não decodifica os comandos GX nem executa listas de vértices/primitivas; SDR1, page-table walk, TLB e permissões PP ainda não estão implementados; FPU Gekko/paired-single e suas exceções ainda faltam; DSP e vários dispositivos Flipper continuam parciais. O build XEX por XSDK permanece em execução em revisões anteriores e não foi confirmado por artefato nesta validação. Não há boot em RGH nem compatibilidade de jogos comprovados.

**Progresso estimado:** 62% de implementação geral. O avanço reflete a integração do endereço GX FIFO com o barramento e a aprovação dos testes strict e sanitizers; não significa GPU completa, MMU/FPU completas ou 62% de compatibilidade de jogos.

## v2.3b — SDR1 SPR e correção do build XEX

- Adicionado o registrador `SDR1` ao estado do Gekko.
- Implementados `MFSPR` e `MTSPR` para SPR 25, com bloqueio em modo usuário e exceção de instrução privilegiada.
- Testes de leitura, escrita e proteção de privilégio incluídos em `test_core_host.c`.
- **Validação:** workflow libXenon no commit `56a4a797eed402c32abfeec10b235ff269dadf96`, execução #43647427810: seis jobs aprovados, incluindo host-core-test, host-sanitizers e build-core-smoke.
- Corrigido o workflow `xex-cloud.yml` para invocar o script de toolchain com `bash`, evitando erro de permissão 126. Correção no commit `6f54436cc6c45ada11a4faf8f810bd577792f128`; nova execução aguardando resultado.

**Ainda não implementado:** page-table walk PPC 6xx/Gekko, PTE0/PTE1, TLB, permissões PP/R/C e exceções completas de tradução/proteção. SDR1 está apenas armazenado e acessível como SPR; não há tradução por páginas ainda. FPU Gekko, paired-single e decodificador GX continuam pendentes/parciais. Nenhum `default.xex` final foi confirmado.

**Progresso estimado:** 63% de implementação geral. O avanço cobre o estado e acesso privilegiado de SDR1 com testes aprovados; não representa MMU completa nem compatibilidade de jogos.
