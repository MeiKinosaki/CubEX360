# Status da toolchain — v1.31b

## Confirmado
- GitHub Actions concluiu com sucesso a execução [38219582132](https://github.com/MeiKinosaki/CubEX360/actions/runs/38219582132).
- O artefato foi baixado e verificado como ELF32 PowerPC big-endian, estaticamente ligado.
- SHA-256: `96d490d9c467eca735f571cfd8327d1838fbb23a791bbf9e64bac7d5c2648854`.
- A suíte host do pacote local v1.31b passou: 21 executáveis.
- `make xex-preflight` foi testado e retorna código 2 quando as ferramentas XEX não estão configuradas, como esperado.

## Não confirmado
- O ELF de CI é somente o programa `xenon-probe`; não contém o núcleo completo do CubEX360.
- O núcleo ainda não foi ligado pelo workflow libXenon.
- `default.xex` não foi gerado.
- Nenhum teste em Xbox 360 RGH foi realizado.

## Próximas etapas
1. Publicar integralmente os fontes do núcleo na árvore GitHub e integrá-los ao alvo libXenon.
2. Gerar e verificar um ELF do núcleo, distinto do probe.
3. Identificar uma toolchain/runtime que gere XEX válido para o Xbox 360.
4. Validar estrutura/imports/ABI do XEX e testar em hardware.

O pacote v1.31b existe como ZIP local; não afirmar que é uma release do GitHub até anexá-lo/publicá-lo realmente.

## Atualização — toolchain em nuvem e default.xex (2026-10-14)

### Compilação remota já configurada
O workflow .github/workflows/xenon-build.yml pode ser acionado manualmente no GitHub Actions. Ele:
- compila e executa os testes host e sanitizers;
- compila o alvo smoke do núcleo usando Free60/libXenon em container;
- valida o formato ELF32 PowerPC big-endian e publica os artefatos ELF e relatórios.

Isso é a etapa de toolchain em nuvem que já existe. O artefato é um ELF de smoke test, não o executável final.

### Caminho de XEX investigado
A documentação pública de RXDK-360/XexTool descreve um empacotador ELF→XEX2 e um fluxo de toolchain Xbox 360. Referências:
- RXDK-360: https://github.com/Team-Resurgent/RXDK360
- XexTool e comando pack: https://github.com/Team-Resurgent/XexTool
- Detalhes do formato e limites do empacotador experimental: https://github.com/Team-Resurgent/RXDK360/blob/main/docs/xex-packing.md

Não é correto simplesmente empacotar o ELF atual como se já fosse o CubEX360 executável: o alvo usa Free60/libXenon, e ABI, entry point, imports do kernel, runtime, layout de memória e chamadas de sistema precisam ser compatíveis com o carregador XEX. O documento de empacotamento também relata limitações de validação em emulador; isso não prova inicialização em hardware real.

### Bloqueio explícito
- default.xex do CubEX360 completo: NÃO GERADO.
- Build do núcleo ligado a runtime/imports XEX validados: pendente.
- Teste no Xbox 360 RGH: pendente.

Próximo marco real: escolher e fixar uma toolchain XEX compatível, criar um alvo de aplicação que use essa ABI/runtime, produzir um XEX de teste, inspecioná-lo com XexTool e executar teste de boot antes de liberar um artefato chamado default.xex. Não renomear ELF nem usar o probe como se fosse o emulador.

## Atualização de desenvolvimento — v1.57b (2026-10-15)

- `MCRXR` foi adicionado ao interpretador: transfere SO/OV/CA do XER para o campo CR selecionado e limpa esses três bits no XER.
- Teste de regressão adicionado para o campo CR2 e limpeza dos bits XER.
- CI da v1.57b está pendente na última consulta; não marcar como aprovado até os jobs host e ASan/UBSan terminarem com sucesso.
- O workflow `.github/workflows/xex-build.yml` continua condicionado a runner Windows próprio com toolchain compatível. O repositório não fornece um runner remoto já configurado.
- Nenhum `default.xex` funcional foi produzido; a geração continua bloqueada até a integração real do núcleo com ABI/runtime/imports XEX e uma toolchain disponível.

### Próxima sequência técnica
1. Validar `MCRXR` nos testes host e sanitizers.
2. Corrigir qualquer falha antes de adicionar instruções.
3. Auditar `MFSPR`/`MTSPR` e XER para máscara de campos e semântica de overflow/carry.
4. Expandir testes de acesso à memória, alinhamento e endereços fora da RAM.
5. Separar o núcleo de emulação do smoke target libXenon e integrar um ponto de entrada de aplicação.
6. Continuar FPU, exceções, MMU, GPU Flipper, DSP, drive e IPL como subsistemas separados; eles continuam ausentes/parciais.
7. Manter o gate XEX fechado até o build produzir um XEX de verdade e a validação estrutural/imports ser demonstrada.

## Atualização — v1.58b (2026-10-15)
- Adicionados testes de limites da RAM, acessos desalinhados de 16/32 bits e endereço de barramento não mapeado.
- Adicionado teste de falha de busca de instrução: deve retornar `CUBEX_CPU_MEMORY_FAULT` sem avançar PC nem incrementar `retired_instructions`.
- Ajustado o workflow XEX para verificar explicitamente o código de saída do script de build, incluindo falhas de ferramentas nativas.
- A execução CI da v1.58b precisa ser consultada após o commit; não presumir sucesso antes do resultado.
- O workflow XEX permanece dependente de runner Windows próprio e toolchain configurada; `default.xex` funcional continua não gerado.

### Limitações atuais mantidas
- O núcleo continua sendo um interpretador PowerPC parcial, não um emulador de GameCube completo.
- FPU, exceções/interrupts, MMU, GPU Flipper, DSP, unidade de disco e IPL ainda precisam de implementação e integração.
- O build Free60/libXenon gera ELF de smoke target, não um `default.xex` validado.
- Continuação da v1.58b: adicionadas `ADDC` e `SUBFC`, com atualização do indicador XER.CA e testes de carry/no-borrow. Os workflows desta última alteração ainda estão pendentes; aguardar resultado antes de considerar o comportamento validado.

## Atualização — v1.59b (2026-10-15)
- Adicionadas as instruções PowerPC `ADDE`, `SUBFE`, `ADDME` e `SUBFZE` para operações encadeadas pelo XER.CA.
- Testes de regressão publicados para carry/no-borrow e consumo de XER.CA.
- Workflows da v1.59b estão em execução na última consulta; aguardar conclusão antes de declarar a implementação validada.
- A v1.58b passou nos workflows de implementação/testes de `ADDC`/`SUBFC` e nos testes de limites da RAM.
- Continua pendente o runner Windows próprio para o build XEX; não há `default.xex` funcional produzido.

### Próximas áreas
1. Validar a cadeia de carry com cenários de carry=0 e carry=1, inclusive subtração com empréstimo.
2. Implementar/validar operações aritméticas restantes, respeitando SO/OV/CA e semântica de OE/Rc.
3. Expandir load/store e exceções de acesso desalinhado.
4. Criar ponto de entrada de aplicação que integre o núcleo real ao alvo libXenon; o alvo atual é smoke build.
5. Só liberar XEX após toolchain, imports/runtime, verificação de contêiner e teste de boot.

## Atualização — v1.60b (2026-10-15)
- Implementadas `ADDZE` e `SUBFME`, ampliando a família de aritmética encadeada pelo XER.CA.
- Adicionados testes para transporte em `ADDZE` e caso sem transporte em `SUBFME`.
- Workflows #85 e #86 estavam na fila na última consulta; não declarar v1.60b validada até o término.
- v1.59b (`ADDE`, `SUBFE`, `ADDME`, `SUBFZE`) passou nos workflows #83 e #84.
- O avanço segue concentrado na CPU inteira e robustez do núcleo; FPU, MMU, GPU, DSP, disco e IPL continuam pendentes.
- O pipeline XEX ainda depende de runner Windows próprio e toolchain/runtime compatíveis. `default.xex` funcional não foi gerado.


## Atualização — v1.61b (2026-10-15; CI pendente)
- Implementadas `ADDO` e `SUBFO` com atualização de OV e SO sticky; forma Rc atualiza CR0.
- Regressões acrescentadas para overflow assinado, OV limpo no caso seguinte sem overflow e SO preservado.
- Runs #88 e #89 ainda estavam em andamento/fila na última consulta; aguardar ambos antes de declarar a versão aprovada.
- O run #87 da correção de teste v1.60b concluiu com sucesso. O run #86 foi uma tentativa anterior que falhou antes da correção.
- Nenhum `default.xex` funcional foi produzido; XEX depende de runner/toolchain/runtime e integração de ABI ainda não disponíveis/validados.


## Atualização — 2026-10-16
- A validação da correção v1.61b foi concluída: run #92 passou, incluindo o caso `ADDO.` e `CR0/SO`.
- A v1.62b adiciona `MULLW` e seus testes; run #93 ainda está em andamento na última consulta.
- O build ELF32 permanece experimental; `default.xex` funcional e boot em Xbox 360 RGH não foram comprovados.


## Atualização — 2026-10-16 (v1.62b–v1.63b)
- Run #93 da implementação `MULLW` passou; no run #94, testes host, sanitizers e probe passaram, mas o smoke build ainda estava em andamento na última consulta.
- A v1.63b adiciona `SRAW`; host e sanitizers passaram no run #95, com builds de destino ainda em andamento. Run #96 testa a regressão específica.
- `default.xex` funcional e boot em Xbox 360 RGH permanecem não comprovados.


## Atualização — v1.64b
- `MULLW` v1.62b passou na execução #94, incluindo host, sanitizers, probe e smoke build.
- `SRAW` v1.63b passou host/sanitizers, mas os jobs de build ainda estavam em execução na última consulta.
- `MULHWU` v1.64b e os testes correspondentes foram enviados; CI #97–#98 ainda não terminou.
- Nenhum `default.xex` funcional ou boot em Xbox 360 RGH foi comprovado.


### Correção de CI — 2026-10-16
- A falha do run #96 foi localizada em codificação errada dos registradores no teste de `SRAW` (fonte/destino invertidos no teste). Correção enviada; run #99 pendente.
- O run #97 falhou na suíte compartilhada que continha o teste SRAW incorreto; run #98 ainda não tinha conclusão na última consulta. Os testes host/sanitizers da `MULHWU` não devem ser considerados aprovados até nova execução.


## Atualização — v1.65b
- A v1.63b teve falha no teste específico de `SRAW` por campos de registrador invertidos no teste; correção enviada e CI #99 em andamento.
- A v1.64b não pode ser considerada aprovada enquanto a regressão SRAW incluída na suíte não passar; runs #97–#98 falharam na suíte compartilhada.
- A v1.65b adiciona `MULHW` signed e testes; CI #100–#101 pendente.

- **Atualização:** run #99 passou em todos os jobs após corrigir os campos de registrador de SRAW; essa revisão também contém `MULHWU` e sua regressão, então a suíte de v1.63b/v1.64b foi validada. A nova `MULHW` v1.65b ainda aguarda #100–#101.


### Estado verificado — v1.65b
- Runs #100 e #101 concluíram com sucesso em todos os jobs: host, sanitizers, probe e smoke build.
- Runs #97/#98 foram falhas anteriores causadas pela regressão de codificação do teste SRAW; o run #99 passou após a correção, e #100/#101 passaram com `MULHW`.
- Isto valida os testes de núcleo/build experimental, não um `default.xex` funcional nem boot RGH.


## Atualização — v1.66b
- v1.65b `MULHW` validada pelos runs #100–#101, todos os jobs concluídos com sucesso.
- v1.66b adiciona `MULLWO` com flags de overflow e SO sticky; CI #102–#103 pendente.
- `default.xex` funcional, boot RGH e compatibilidade com jogos continuam não comprovados.


## Estado verificado — 2026-10-17
- v1.66b `MULLWO`: workflows #102 e #103 concluídos com sucesso.
- v1.67b adiciona `DIVWU`/`DIVWUO`; workflows #104–#105 ainda em execução/fila. Divisão por zero usa resultado provisório determinístico zero, com OV/SO na variante `O`.
- O pipeline de smoke build não equivale a um `default.xex` funcional nem prova boot em Xbox 360 RGH.
