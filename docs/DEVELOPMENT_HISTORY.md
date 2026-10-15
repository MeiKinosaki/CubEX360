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