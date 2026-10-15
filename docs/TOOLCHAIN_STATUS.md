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