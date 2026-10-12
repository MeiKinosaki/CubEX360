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
