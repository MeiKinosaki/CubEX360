# CubEX360 v1.26b

Projeto experimental de interpretador parcial PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Novidades v1.26b
- `mfmsr`: leitura do registrador de estado da máquina (MSR) para um GPR.
- `mtmsr`: gravação de um GPR no MSR.
- Testes nativos para leitura, gravação e rejeição de campos reservados.

## Validação local
O pacote v1.26b foi compilado com C99, `-Wall -Wextra -Werror -O2`. Os 20 executáveis de teste passaram e a integridade do ZIP foi verificada.

## Estado de publicação
O README e as notas da v1.26b estão documentados no repositório. O ZIP completo foi criado localmente, mas ainda não foi anexado como artefato de release e o código-fonte integral da versão não foi commitado por completo nesta branch.

## Limitações
O suporte a `mfmsr` e `mtmsr` é parcial; privilégios, efeitos colaterais do MSR e exceções ainda não são modelados. A toolchain libXenon/`xenon-gcc` não foi validada; não há `xenon.elf` ou `default.xex` confirmados, e o teste no Xbox 360 RGH permanece pendente. O projeto não é um emulador completo de GameCube.
