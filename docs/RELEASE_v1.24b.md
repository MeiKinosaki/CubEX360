# CubEX360 v1.24b — instrução mcrxr

## Mudanças
- Adiciona suporte parcial a `mcrxr crfD`.
- Copia XER[SO], XER[OV] e XER[CA] para os bits SO/OV/CA do campo CR selecionado, deixando o quarto bit em zero.
- Limpa SO, OV e CA no XER após a transferência.
- Adiciona testes de regressão para flags ativos e inativos.

## Testes realizados
- `make clean test`: 18 executáveis nativos passaram.
- Compilação com C99, `-Wall -Wextra -Werror -O2`.
- ZIP verificado com `unzip -t`.

## Limitações conhecidas
- A toolchain libXenon/`xenon-gcc` não foi validada.
- `xenon.elf` e `default.xex` não foram gerados/confirmados.
- Teste no Xbox 360 RGH pendente.
- O ZIP completo foi criado localmente, mas ainda não está anexado como artefato do repositório; o código integral da versão ainda não foi publicado na branch.
