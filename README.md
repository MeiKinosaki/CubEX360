# CubEX360 v1.30b — integração do núcleo na toolchain

Projeto experimental de interpretador parcial PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Marco v1.30b (pacote local)

O pacote de desenvolvimento v1.30b acrescenta um smoke test que liga os fontes atuais de memória, barramento, estado da CPU e interpretador ao programa libXenon. O teste verifica:
- alocação de 24 MiB de RAM;
- alias cached/uncached e leitura big-endian do barramento;
- execução de `addi r3,r0,42` no interpretador.

A suíte host tem 21 executáveis de teste; todos passaram com C99, `-Wall -Wextra -Werror -O2`. O ZIP foi verificado localmente. O pacote v1.30b ainda não foi publicado integralmente nesta árvore do GitHub.

## CI e limites

O workflow já publicado validou um ELF32 de diagnóstico, mas ainda não compilou o novo alvo de núcleo integrado. É necessário publicar os fontes v1.30b e então rodar o workflow atualizado para confirmar o ELF do núcleo.

- [Execução anterior do probe libXenon](https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703)
- [Status da toolchain](docs/TOOLCHAIN_STATUS.md)
- [Plano de geração de default.xex](docs/DEFAULT_XEX.md)

## `default.xex`

Não foi gerado. O fluxo libXenon validado até agora gera ELF32; ele não converte automaticamente ELF em XEX. Não renomeie o ELF. O projeto também ainda não é um emulador completo de GameCube, e o teste no Xbox 360 RGH permanece pendente.
