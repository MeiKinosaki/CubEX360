# Caminho para `default.xex` — CubEX360

## Estado confirmado

A primeira etapa usa Free60/libXenon. O fluxo oficial gera ELF/ELF32 para execução via XeLL. O workflow `CubEX360 libXenon toolchain probe` tenta compilar um pequeno programa de diagnóstico e publicar o `.elf32` como artefato de CI.

**`default.xex` não é produzido por esse fluxo.** XEX é outro formato de executável do Xbox 360. Não é correto renomear um ELF para `.xex` nem declarar uma conversão sem um linker/runtime e empacotador XEX compatíveis.

## Etapas necessárias

1. Fazer o workflow ELF32 passar no GitHub Actions e guardar o artefato real com seu SHA-256.
2. Integrar o núcleo CubEX360 ao alvo libXenon, substituindo o probe isolado.
3. Escolher e validar uma toolchain/runtime que produza XEX para Xbox 360. libXenon por si só não satisfaz esse requisito; nenhum SDK XEX está configurado neste projeto.
4. Somente depois de compilar com essa toolchain, validar a estrutura XEX e testar `default.xex` no hardware alvo.

## Regra de publicação

Não publicar ELF renomeado, XEX vazio ou artefato falso. Até a validação de uma toolchain XEX, o alvo desta pipeline é o ELF32 de diagnóstico, não `default.xex`.

Referência libXenon: https://github.com/Free60Project/libxenon
