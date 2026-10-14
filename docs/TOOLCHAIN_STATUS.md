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
