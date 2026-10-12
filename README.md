# CubEX360 v1.25b — lógica do CR

Projeto experimental de interpretador parcial de instruções PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Mudanças da v1.25b

A versão de desenvolvimento local adiciona suporte parcial a oito operações lógicas do registrador de condição (CR):
- `crand`
- `crandc`
- `creqv`
- `crnand`
- `crnor`
- `cror`
- `crorc`
- `crxor`

A suíte nativa contém 19 executáveis de teste e passou com C99, `-Wall -Wextra -Werror -O2`. O pacote ZIP da v1.25b foi criado e sua integridade verificada.

## Publicação do código

Esta documentação da v1.25b está publicada. O ZIP completo ainda não foi anexado como release, e os arquivos-fonte integrais da versão ainda não foram commitados nesta branch.

## Xbox 360 e limitações

A toolchain libXenon/`xenon-gcc` ainda não foi validada neste ambiente. Não há `xenon.elf` nem `default.xex` gerados ou confirmados, e o teste em Xbox 360 RGH está pendente.

O projeto permanece um interpretador parcial: faltam componentes importantes da CPU Gekko, FPU, MMU/exceções, GPU, DSP, leitura de discos e integração de hardware para compatibilidade ampla com jogos.
