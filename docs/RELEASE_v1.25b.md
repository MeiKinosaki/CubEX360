# CubEX360 v1.25b — operações lógicas do CR

## Implementação
Adiciona suporte parcial às operações XL-form `crand`, `crandc`, `creqv`, `crnand`, `crnor`, `cror`, `crorc` e `crxor`.

O novo teste `test_cpu_cr_logic` verifica as oito operações. A suíte completa executa 19 binários de teste nativos.

## Validação local
- `make clean test`: 19 executáveis de teste aprovados.
- Compilação C99 com `-Wall -Wextra -Werror -O2`.
- Integridade do ZIP verificada.

## Limitações
- Toolchain libXenon/`xenon-gcc` ainda não validada.
- `xenon.elf` e `default.xex` não gerados/confirmados.
- Teste em Xbox 360 RGH pendente.
- O ZIP completo foi criado localmente, mas ainda não foi anexado como release nem o código integral foi publicado na branch.
