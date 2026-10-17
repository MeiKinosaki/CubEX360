# CubEX360 — histórico consolidado de desenvolvimento

Data de consolidação: 2026-10-14  
Projeto: emulador experimental de Nintendo GameCube para Xbox 360 RGH  
Repositório: https://github.com/MeiKinosaki/CubEX360

> Este documento consolida os marcos recuperados das conversas de desenvolvimento e do histórico Git. A numeração identifica marcos de desenvolvimento; não significa que o emulador seja funcional ou compatível com jogos. Percentuais antigos são estimativas informais, não métricas automatizadas.

## Identidade e objetivo

- Nome do projeto: **CubEX360**.
- Plataforma-alvo: Xbox 360 com RGH.
- Sistema emulado: Nintendo GameCube.
- CPU principal a emular: IBM/Motorola PowerPC Gekko.
- Estratégia de plataforma: C portátil para o núcleo e pipeline Free60/libXenon para produzir artefatos PowerPC/Xbox 360.
- Meta executável: gerar um XEX válido, normalmente `default.xex`, a partir de uma aplicação integrada real. Um ELF não pode ser apenas renomeado para XEX.

## Histórico recuperado por versões

### Fase inicial — v0.3b, v0.8b e v1.18b
- O projeto já tinha versões de protótipo registradas v0.3b e v0.8b.
- Em v1.18b, o núcleo PowerPC parcial já tratava comparações e campos do registrador de condição (CR), com testes para `cmp`/`cmpl`; formatos de 64 bits eram rejeitados em vez de aceitos incorretamente.
- O histórico conversacional anterior menciona 14 testes nesta fase. Isso não deve ser confundido com o número atual de testes.

### v1.19b — aritmética e deslocamentos
Registrado em conversas anteriores:
- Instruções: `adde`, `subfe`, `addme`, `subfze`, `mulli`, `subfic`, `addic`, `addic.`, `slw`, `srw`, `sraw`, `srawi`.
- 15 conjuntos/casos de testes nativos foram reportados como aprovados naquele estágio.
- A disponibilidade de `xenon-gcc`, ELF de aplicação e XEX ainda era bloqueio.

### v1.20b — operações de bits
- Adicionadas `cntlzw`, `extsb` e `extsh`.
- Testes locais e pacote ZIP foram reportados como aprovados.
- `xenon.elf`, `default.xex` e teste físico no Xbox 360 RGH continuavam pendentes.
- Uma toolchain planejada foi citada como “Shining Saturn”; a linha de trabalho posterior do repositório passou a usar Free60/libXenon para o probe e o alvo de smoke test.

### v1.22b–v1.31b — documentação e primeira prova de toolchain
- Foram criadas notas de release e documentação de build.
- Um probe compilado com Free60/libXenon foi verificado como ELF32 PowerPC big-endian e publicado como artefato de CI.
- Hash registrado para o probe: `96d490d9c467eca735f571cfd8327d1838fbb23a791bbf9e64bac7d5c2648854`.
- Esse artefato é uma prova de toolchain, **não** o emulador completo.
- A suíte host foi reportada com 21 testes no pacote v1.31b; o gate XEX corretamente bloqueava a geração sem ferramentas validadas.
- O ZIP v1.31b foi descrito como criado/testado localmente, mas não publicado integralmente como release/anexo do GitHub.

### v1.32b — estrutura do núcleo
- Criado `core-src/` com memória principal de 24 MiB, barramento cached/uncached, interpretador limitado e alvo de smoke test libXenon.
- Adicionado build do núcleo em CI, separado do probe da toolchain.
- O ELF do smoke test não representava ainda uma aplicação de emulação completa.

### v1.35b–v1.40b — instruções imediatas, aritmética e testes
- Operações lógicas imediatas: `ANDI.`, `ORIS`, `XORI`, `ANDIS.`.
- Corrigidos erros de decodificação de operandos/destino nas instruções lógicas imediatas, bem como codificações de máquina e expectativas de teste.
- Adicionadas operações aritméticas selecionadas X-form, incluindo suporte parcial para `ADD` e `SUBF`.
- `MULLI`, `ADDIC` e `ADDIC.`.
- Criado job nativo estrito de regressão e job com AddressSanitizer/UndefinedBehaviorSanitizer.
- As correções foram motivadas por falhas reais de testes; os resultados devem ser lidos por commit e não presumidos a partir da documentação.

### v1.41b — loads/stores e segurança
- Adicionados `LBZ`, `LHZ`, `LHA`, `STB` e `STH`, além de `LWZ`/`STW`.
- Testes para carga com/sem sinal, gravação de byte/meia-palavra, acessos desalinhados e rejeição de opcode não suportado.
- Corrigidos casos de aliasing nos testes e semântica de operandos assinados em operações imediatas.

### v1.42b–v1.46b — desvios, comparações e SPRs
- `BC`: condição CR, avaliação/decremento de CTR e atualização de LR quando LK=1.
- `BCLR`/`BLR`, `BCCTR`/`BCTR`, com testes para destino indireto e preservação de CTR em BCTR.
- Comparações imediatas assinadas/não assinadas `CMPWI`/`CMPLWI).
- `MFSPR`/`MTSPR` para LR e CTR.
- `CMPW`/`CMPLW` para comparação entre registradores e seleção do campo CR.
- Vários commits corrigiram codificações de desvios, máscaras de CR e expectativas dos testes.

### v1.47b–v1.50b — rotações, barramento e execução limitada
- v1.47b: `RLWINM), rotação à esquerda com máscara.
- v1.48b: `RLWIMI), acesso ao barramento de RAM de 8/16/32 bits e encaminhamento dos loads/stores de byte/meia-palavra pelo barramento.
- Criado `cubex_cpu_run` para executar um orçamento limitado de instruções e propagar erros.
- Alvo `main.c` executa uma sequência de smoke test.
- Gate de pré-verificação XEX bloqueia com segurança quando ferramentas XEX reais não estão configuradas.
- v1.49b: deslocamentos `SLW`, `SRW`, `SRAWI), com correção de teste para deslocamento zero.
- v1.50b: `RLWNM) — rotação por registrador e máscara.

### v1.51b — expansão do subconjunto inteiro
- Ampliação do subconjunto de instruções inteiras Gekko/PowerPC e regressões adicionais.
- Corrigido um caso de codificação de operando em `XORIS`.
- Documentados o estado da CPU e os bloqueios da toolchain/XEX.

### v1.52b — NEG, EQV e NAND
Commits recuperados:
- Implementação: `075f7ef902c57458be9a93d4956fba57e3c0344c` — `NEG`, `EQV), `NAND`.
- Testes: `48d2eb08d471d4b912338129a26c3b99ac941db9`.
- A execução de CI do commit de testes terminou com sucesso (workflow run 68).

### v1.53b — ORC e acesso ao XER
Commits recuperados:
- Implementação: `8fd0523540390c227165b609d7e7f41982bc26ac` — `ORC) e suporte de leitura/escrita para o SPR XER, além do LR/CTR.
- Testes: `0095ef6bccff98c91792cda027d72ca87b03a1e3`.
- O workflow run 69, referente à implementação, passou. O workflow run 70, referente aos testes, falhou nos jobs host e sanitizers por uma asserção do teste de ORC em `core-src/tests/test_core_host.c:311`.
- Diagnóstico da falha: o teste espera `0xf0f0f0ff`, mas a codificação/valor esperado do caso ORC precisa ser corrigido e validado antes de avançar. Portanto, a revisão de testes v1.53b **não está aprovada** neste momento.
- O job XEX preflight passou (bloqueio correto); os jobs `build-probe` e `build-core-smoke` também passaram, produzindo ELFs de diagnóstico/smoke test. Isso não significa que exista `default.xex`.

## Estado técnico atual (14/10/2026)

### Código presente no repositório
- `core-src/include/cubex360/`: interfaces de CPU, memória e barramento.
- `core-src/src/cpu.c`: interpretador PowerPC/Gekko parcial.
- `core-src/src/memory.c`, `bus.c`: memória e acessos ao barramento.
- `core-src/tests/test_core_host.c`: regressões host.
- `.github/workflows/xenon-build.yml`: jobs de testes host, sanitizers, gate XEX, probe libXenon e core smoke test.
- `scripts/xex-preflight.sh`, `scripts/verify-xenon-elf.sh`: gates/verificações.
- `docs/DEFAULT_XEX.md`, `docs/TOOLCHAIN_STATUS.md`, `docs/CI_ELF32_VERIFICATION.md`, `docs/CORE_SMOKE_BUILD.md`: documentação de bloqueios e build.

### O que foi comprovado
- O CI consegue compilar um probe e um alvo de smoke test em ELF32 PowerPC big-endian.
- O pipeline possui testes nativos estritos e sanitizers.
- O gate XEX falha de forma segura quando não há ferramentas reais configuradas.

### O que NÃO foi comprovado
- Não há evidência confirmada de um `default.xex` válido gerado a partir do núcleo completo.
- Não há evidência de boot do CubEX360 em Xbox 360 RGH.
- Não há compatibilidade demonstrada com jogos de GameCube.
- Não há implementação completa de Gekko/FPU, exceções, MMU, interrupções, GPU Flipper/VI, DSP/áudio, drive de disco, IPL/BIOS ou subsistemas necessários para emulação funcional completa.

## Próximas ações recomendadas (ordem de prioridade)

1. Corrigir o teste de `ORC` em `core-src/tests/test_core_host.c` (assert na linha 311) e validar a codificação dos registradores e o resultado esperado.
2. Executar novamente CI host + ASan/UBSan; só avançar com o resultado observado.
3. Conferir e ampliar a semântica de `MFSPR`/`MTSPR` para XER, incluindo campos/flags corretos, e adicionar testes específicos.
4. Consolidar inventário de instruções implementadas, rejeitadas e ainda ausentes; continuar expansão incremental da CPU Gekko.
5. Integrar CPU, barramento e memória em um alvo de aplicação, sem confundir o smoke test com o emulador completo.
6. Identificar/configurar ferramentas de empacotamento XEX reais e reproduzíveis; manter o gate fechado até o build ser validado.
7. Gerar `default.xex`, inspecionar formato/metadata e somente então planejar teste real em console RGH.

## Estimativas antigas (não são métricas verificadas)

O resumo de progresso anterior estimava, de forma aproximada:
- CPU inteira Gekko: 35%.
- FPU: 5%.
- registradores/flags: 30%.
- exceções/interrupções: 10%.
- memória/barramento: 35%.
- MMU: 5%.
- integração do núcleo: 20%.
- toolchain Free60/libXenon: 60%.
- `default.xex`: 5%.
- boot em RGH: 0%.
- progresso global: cerca de 20%.

Esses percentuais foram estimativas informais anteriores à última falha de CI. Não representam cobertura medida; devem ser recalculados depois da correção dos testes e de uma auditoria de código.

## Referências
- Repositório: https://github.com/MeiKinosaki/CubEX360
- Histórico de commits: https://github.com/MeiKinosaki/CubEX360/commits/main
- GitHub Actions: https://github.com/MeiKinosaki/CubEX360/actions
- Gate e plano XEX: `docs/DEFAULT_XEX.md`
- Estado da toolchain: `docs/TOOLCHAIN_STATUS.md`

## v1.57b–v1.58b — registradores e robustez de memória (2026-10-15)

### v1.57b
- Implementada `MCRXR` para transferir SO/OV/CA do XER ao campo CR selecionado e limpar os bits transferidos.
- Teste de regressão para o campo CR2 e limpeza de SO/OV/CA.
- Workflows #78 e #79 concluídos com sucesso: [implementação](https://github.com/MeiKinosaki/CubEX360/actions/runs/38718716181) e [testes](https://github.com/MeiKinosaki/CubEX360/actions/runs/38718736129).

### v1.58b
- Ampliados testes para limites da RAM, leituras/gravações desalinhadas, endereço de barramento não mapeado e falha de busca de instrução sem aposentadoria de instrução.
- Workflow XEX ajustado para verificar código de saída do script de build antes da validação do candidato.
- CI para a v1.58b ainda precisa ser consultado; os testes não são considerados aprovados até execução bem-sucedida.

### Estado de produto
- O núcleo continua parcial e ainda não oferece emulação completa de GameCube.
- `default.xex` real e funcional continua não gerado; a toolchain XEX depende de runner Windows próprio e ABI/runtime compatíveis.
- Não há boot em Xbox 360 RGH nem compatibilidade de jogos comprovados.
## v1.59b — aritmética encadeada por carry (2026-10-15)
- Implementadas `ADDE`, `SUBFE`, `ADDME` e `SUBFZE` com consumo/atualização de XER.CA.
- Adicionados testes de regressão sequenciais para carry e no-borrow.
- Workflows #83 e #84 foram disparados e estavam em execução na última consulta; não considerar v1.59b validada antes de ambos terminarem.
- v1.58b: workflows #80–#82 concluídos com sucesso para limites/alinhamento de memória e `ADDC`/`SUBFC`.
- `default.xex` do emulador completo segue não gerado; o workflow XEX exige runner Windows e script de toolchain provisionados.

## v1.60b–v1.61b — cadeia de carry e overflow (2026-10-15)

### v1.60b
- `ADDZE` e `SUBFME` foram adicionadas à família de aritmética encadeada por `XER.CA`.
- Run #87 passou após correção de uma expectativa de teste; run #86 é uma execução anterior que falhou e foi corrigida. Isso valida o caso coberto, não todas as combinações de carry.
- [CI de correção v1.60b](https://github.com/MeiKinosaki/CubEX360/actions/runs/38720276159).

### v1.61b — em validação
- Adicionadas `ADDO` (XO 778) e `SUBFO` (XO 552) para atualizar `XER.OV` a partir de overflow assinado; `XER.SO` permanece sticky até ser alterado por instruções apropriadas, e Rc atualiza CR0.
- Testes host cobrem overflow positivo→negativo em soma, overflow em subtração, limpeza de OV no caso sem overflow e persistência de SO.
- Commits: CPU `8cae8b09cd6171360cd47430aa78466ef206989e`; testes `375fbb0b5245dd5741bee56da68ea0e8fc0f5efb`.
- Workflows #88 e #89 estão em andamento/fila na última consulta. A v1.61b **ainda não está validada** até os jobs host, sanitizers e smoke build concluírem.

### Limites que permanecem
- FPU/paired-single, MMU, exceções/interrupções completas, Flipper/VI, DSP, drive, IPL e integração de jogo continuam incompletos.
- ELF32 de smoke test não equivale a `default.xex`; XEX funcional e boot em RGH seguem não comprovados.


### Correção de regressão durante a validação v1.61b
- O run #89 falhou porque o teste esperava OV limpo após `SUBFO`, mas os operandos de teste também causam overflow assinado. O cálculo esperado foi corrigido; não foi removida a asserção de overflow.
- Run #90, com a correção, está em andamento na última consulta: https://github.com/MeiKinosaki/CubEX360/actions/runs/38721264668.
- Inventário auditável dos opcodes presentes e das limitações: `docs/CPU_COVERAGE.md`.

- A execução #90 também falhou no teste de CR0: `SUBFO.` inclui o bit SO sticky no campo de resultado de comparação. A expectativa foi corrigida no commit `2591824ce5`; a execução #91 estava em andamento na última consulta. Foi acrescentado mais um caso para `ADDO.` confirmar CR0 com SO sticky no commit `308272e631`; CI desse último commit ainda não foi consultado como concluído.


## v1.62b — MULLW (2026-10-16; CI pendente)
- Implementada `MULLW` (XO 235), com resultado limitado aos 32 bits inferiores do produto de palavras com sinal.
- Adicionados testes de produto negativo e wrap/truncamento para zero, incluindo a forma com `Rc`.
- Commits: CPU `06b9ec33c7ff084e394a7c274f2ca4b3ac6b1018`; testes `c419f08693a351c7663d48179aa935d7c57363a1`.
- A validação v1.61b foi confirmada pelo run #92: sucesso. A execução #93 da implementação v1.62b está em andamento.


## v1.63b — SRAW (2026-10-16; CI em andamento)
- Implementada a instrução `SRAW` (XO 792): deslocamento aritmético à direita com quantidade em registrador, propagação do sinal, atualização de `XER.CA` pelos bits descartados e atualização de `CR0` na forma `Rc`.
- Testes cobrem fonte negativa com bits descartados, fonte positiva e atualização de CA/CR0.
- Commits: CPU `5dcd05ced2b594d136830c5938aac4d426ed1c95`; testes `c40ee0ec09f255498b2ea717ecc43ccad3f5c801`.
- Run #95: host/sanitizers passaram; build de destino ainda estava em andamento. Run #96 ainda estava em execução na última consulta.
- v1.62b: run #93 passou; run #94 tinha host/sanitizers e probe passados, mas o smoke build seguia em andamento.


## v1.64b — MULHWU (2026-10-16; CI pendente)
- Implementada `MULHWU` (XO 11): multiplica dois operandos unsigned de 32 bits e armazena os 32 bits superiores do produto de 64 bits.
- Testes cobrem `0xffffffff × 2` (palavra superior igual a 1) e um produto cuja palavra superior é zero; ambos usam a forma `Rc` para verificar CR0.
- Commits: CPU `dcddf703cc719216015084016f5a91e43cecfdd7`; testes `5724c514d5c6f667f3897027f57ec19c41832630`.
- CI #97 da implementação está em andamento; #98 dos testes está na fila.
- v1.62b (`MULLW`) foi validada pelo run #94. A validação integral de v1.63b ainda depende de concluir os jobs #95–#96.


### Correção dos testes SRAW e estado de MULHWU
- O run #96 falhou porque os testes codificavam incorretamente os campos de registrador fonte/destino de `SRAW`; o código da instrução não foi apontado como causa. A correção foi enviada no commit `63f634aa805505185ef4faf72db953ed21abd384`; run #99 está pendente.
- O run #97 de `MULHWU` também falhou porque a suíte completa incluía a regressão SRAW incorreta. O run #98 ainda estava em andamento na última consulta. Aguardar os novos workflows antes de concluir a validação v1.63b/v1.64b.


## v1.65b — MULHW (2026-10-16; CI pendente)
- Implementada `MULHW` (XO 75): produto de operandos signed de 32 bits, com armazenamento da palavra superior do produto de 64 bits.
- Testes cobrem `-2 × 3` (palavra superior `0xffffffff`) e `0x7fffffff × 2` (palavra superior zero), incluindo atualização de CR0 na forma `Rc`.
- Commits: CPU `3c6ad2cc67668eab7cdceb79143446f1ea17842a`; testes `88a436e602298d71389a6d9491267e545ee701a4`.
- CI #100–#101 ainda está em execução/fila. Correção do teste SRAW no commit `63f634aa80` está sendo validada pelo run #99.

- **Atualização:** o run #99 terminou com sucesso em todos os jobs (host, sanitizers, probe e smoke build). A correção de codificação dos testes SRAW passou, e o estado da árvore testada inclui também `MULHWU`; isso valida a suíte correspondente à revisão #99. A implementação/testes de `MULHW` entraram depois e aguardam #100–#101.


### Validação concluída — v1.65b
- Runs #100 (implementação) e #101 (testes) terminaram com sucesso em host, sanitizers, probe e smoke build.
- `MULHW` está validada na suíte atual; a revisão também inclui `MULHWU` e a correção dos testes de `SRAW`.
- O run #99 anterior também passou integralmente, confirmando a correção dos campos fonte/destino no teste `SRAW`.


## v1.66b — MULLWO (2026-10-16; CI pendente)
- Implementada `MULLWO` (XO 747): produto signed de 32 bits, armazenamento da palavra inferior e atualização de `XER.OV`; `XER.SO` é sticky em overflow. Forma `Rc` atualiza CR0.
- Testes cobrem `INT_MIN × -1` (overflow), depois `2 × 3` (OV limpo e SO preservado).
- Commits: CPU `cc9d03fbf75b8ea0dd9352fc5b3523e2dccb1491`; testes `c6043ea5687ef742a9fbfbfe13d5074b53d0de5b`.
- CI #102 da implementação está em andamento; #103 dos testes está na fila.
- v1.65b (`MULHW`) passou nos runs #100–#101.


## v1.67b — DIVWU / DIVWUO (2026-10-17; CI pendente)
- Implementadas `DIVWU` e `DIVWUO`, divisão unsigned de 32 bits; `DIVWUO` atualiza `XER.OV` e o sticky `XER.SO` quando o divisor é zero.
- Para evitar comportamento indefinido no núcleo, divisão por zero produz resultado determinístico zero neste emulador experimental; esse comportamento é uma política provisória, não garantia de compatibilidade com hardware GameCube.
- Testes cobrem 100/4 e divisão por zero com verificação de `CR0`, `XER.OV` e `XER.SO`.
- Commits: CPU `05bdf495007a7dac8fe80143e1d7a104a6294637`; testes `0abd59767a2d500fcf1d5dd52fdb388f6cf33e3e`.
- CI #104 (implementação) em andamento; #105 (testes) na fila.
- v1.66b `MULLWO` foi validada: runs #102 e #103 passaram.


## v1.68b — NEG / NEGO
- Adicionadas as instruções PowerPC NEG (XO 104) e NEGO (XO 616), incluindo atualização de CR0 na forma Rc.
- NEGO detecta overflow no caso 0x80000000 e preserva XER.SO como flag sticky; XER.OV é atualizado em cada execução.
- Testes host adicionados para negação de 5, overflow de INT_MIN e limpeza de OV com SO preservado.
- Implementação: 99ba7490cf7a49592558ce525bb61e4cb90986e7; testes: de94e973648eac2d59db951048b4ea3755ba36bc.
- CI desta revisão ainda precisa concluir antes de marcar v1.68b como validada.


## v1.69b — DIVW / DIVWO
- Adicionadas DIVW (XO 491) e DIVWO (XO 1003) para divisão signed de 32 bits.
- Proteção contra divisor zero e INT32_MIN / -1 evita comportamento indefinido do C; nesses casos o núcleo experimental produz zero e DIVWO sinaliza OV/SO.
- Testes cobrem -21 / 4 e overflow por INT_MIN / -1. CI pendente.
- Implementação: 254735071ac1cb8686a9e8f56d1a395071670de8; testes: 944a1ac47a11dd7177f7516cb14d7cd4e99ee7e4.


### v1.70b — ADDZEO and overflow/carry regression

Implemented the PowerPC `ADDZEO` instruction in the Gekko integer interpreter. The operation adds the incoming XER carry bit to the RA register, writes RT, updates carry-out and signed overflow, preserves sticky summary overflow, and updates CR0 when Rc is set. Added host regression cases for `0x7fffffff + CA(1)`, the resulting signed overflow, carry clearing, and a subsequent non-overflowing result while SO remains sticky. CI validation is required before treating the change as verified.
