# CubEX360 v1.27b — SPR de salvamento de exceção

## Mudanças
- Suporte parcial a `mfspr` para ler SRR0 (SPR 26) e SRR1 (SPR 27).
- Suporte parcial a `mtspr` para gravar SRR0 (SPR 26) e SRR1 (SPR 27).
- Teste de regressão amplia a cobertura de SPR e confirma leitura após gravação dos dois registradores.

## Testes realizados
- `make clean test`: 20 executáveis nativos passaram.
- Compilação C99 com `-Wall -Wextra -Werror -O2`.
- ZIP validado com `unzip -t`.

## Limitações
O suporte a SPR e ao mecanismo de exceções é parcial. A toolchain libXenon/`xenon-gcc` não foi validada; `xenon.elf` e `default.xex` não foram gerados/confirmados; teste em Xbox 360 RGH pendente. O ZIP completo foi criado localmente, mas ainda não foi anexado como release e o código integral não foi publicado na branch.
