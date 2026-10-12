# CubEX360 v1.28b — foco na toolchain

Projeto experimental de interpretador parcial PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Prioridade desta versão

A v1.28b concentra-se na infraestrutura de compilação:
- adiciona `xenon-probe/`, um programa mínimo de diagnóstico para verificar o ambiente libXenon;
- adiciona o workflow `.github/workflows/xenon-build.yml`, que usa o contêiner Free60/libXenon para tentar gerar um ELF32 no GitHub Actions;
- documenta separadamente o caminho para `default.xex`.

O probe verifica o toolchain e a saída de console; não é o emulador e ainda não integra o núcleo PowerPC.

## Build do probe

O workflow pode ser executado em GitHub Actions manualmente ou quando os arquivos do probe mudarem. Se a compilação passar, o artefato `cubex360_toolchain_probe.elf32` e seu SHA-256 ficam disponíveis na execução do workflow.

## Sobre default.xex

A toolchain libXenon gera ELF/ELF32 para execução via XeLL; ela não gera `default.xex`. Não é correto renomear um ELF para XEX nem declarar uma conversão sem um runtime e uma toolchain compatíveis. Veja [docs/DEFAULT_XEX.md](docs/DEFAULT_XEX.md).

## Estado
- Testes host do núcleo: executados localmente com `make clean test`.
- ELF32 remoto: pendente de execução do workflow; nenhum artefato de CI foi confirmado ainda.
- Integração do núcleo no alvo Xbox 360: pendente.
- `default.xex`: pendente; exige um fluxo de build XEX validado.
- Teste em Xbox 360 RGH: pendente.
