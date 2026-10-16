# CubEX360 — toolchain-first development (v1.59b)

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
