# CubEX360

Projeto experimental de interpretador parcial de instruções PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Estado atual

**Versão de desenvolvimento: v1.22b — transferência do registrador de condição (CR).**

A v1.22b preparada nesta sessão contém:
- `mfcr`: leitura do registrador CR completo para um registrador geral.
- `mtcrf`: atualização seletiva dos campos do CR por meio da máscara FXM.
- Testes nativos em C para os casos de transferência do CR.

Consulte [as notas da v1.22b](docs/RELEASE_v1.22b.md) para o escopo e as limitações.

## Importante: estado da publicação

Este repositório está sendo atualizado com os metadados e as notas da versão v1.22b. O arquivo-fonte ZIP completo gerado durante a sessão ainda **não foi anexado ao GitHub**; portanto, a presença desta página não significa que todo o código da v1.22b esteja publicado no repositório.

## Compilação e compatibilidade

O pacote de desenvolvimento usa `make clean test` para testes nativos em computador. Isso não comprova uma compilação para Xbox 360. A toolchain libXenon ainda precisa ser validada, e não há `xenon.elf` ou `default.xex` confirmados nesta publicação.

O projeto continua parcial: não implementa toda a CPU Gekko, FPU, MMU/exceções, GPU, DSP, sistema de discos nem os demais componentes necessários para compatibilidade ampla com jogos.
