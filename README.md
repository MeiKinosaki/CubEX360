# CubEX360

Projeto experimental de interpretador parcial de instruções PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Estado atual

**Versão de desenvolvimento: v1.24b — transferência dos flags XER para o CR (`mcrxr`).**

A v1.24b preparada localmente adiciona suporte parcial a `mcrxr crfD`: transfere os flags SO, OV e CA do XER para o campo selecionado do CR e limpa esses três flags no XER. Foram adicionados casos de teste para flags ativos e inativos.

Consulte [as notas da v1.24b](docs/RELEASE_v1.24b.md).

## Validação local

- `make clean test`: 18 executáveis de teste nativos passaram.
- Compilação C99 com `-Wall -Wextra -Werror -O2`.
- Integridade do ZIP da v1.24b verificada.

## Estado de publicação

O README e as notas da versão v1.24b estão publicados nesta branch. O ZIP completo do código-fonte foi criado e testado localmente, mas ainda não foi anexado como artefato binário e o código-fonte integral da versão ainda não foi commitado neste repositório.

## Xbox 360 e limitações

A toolchain libXenon ainda não foi validada neste ambiente. Não há `xenon.elf` nem `default.xex` confirmados. Os testes nativos não demonstram execução no console.

O projeto permanece parcial: faltam partes substanciais da CPU Gekko, FPU, MMU/exceções, GPU, DSP, leitura de discos e integração do sistema para compatibilidade ampla com jogos.
