# CubEX360

Projeto experimental de interpretador parcial de instruções PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Estado atual

**Versão de desenvolvimento: v1.23b — transferência entre campos do CR.**

A versão local v1.23b adiciona:
- `mcrf`: copia um campo de 4 bits do registrador de condição (CR) para outro campo.
- Testes de regressão que verificam a cópia do campo e a preservação dos demais campos.
- Regressão da suíte nativa completa: 18 executáveis de teste passaram com C99, `-Wall -Wextra -Werror -O2`.

Veja [as notas da v1.23b](docs/RELEASE_v1.23b.md).

## Estado de publicação

O README e as notas de versão estão sendo atualizados no GitHub. O ZIP completo do código-fonte v1.23b foi gerado e validado localmente, mas ainda não foi anexado como artefato binário nem todos os arquivos da versão foram commitados nesta branch.

## Compilação e compatibilidade

Os testes nativos são executados com `make clean test` e não comprovam uma compilação para Xbox 360. A toolchain libXenon ainda precisa ser validada; não há `xenon.elf` ou `default.xex` confirmados.

O projeto permanece parcial: ainda faltam partes substanciais da CPU Gekko, FPU, MMU/exceções, GPU, DSP, leitura de discos e integração do sistema para compatibilidade ampla com jogos.
