# CubEX360 v1.23b — mover campo do CR

## Mudanças
- Adiciona suporte parcial a `mcrf crfD,crfS`, copiando o campo de quatro bits de origem para o campo de destino.
- Amplia `test_cpu_cr_transfer` para verificar cópia entre campos e preservação dos demais campos CR.

## Validação local
- `make clean test`: os 18 executáveis de teste nativos passaram.
- Compilação C99 com `-Wall -Wextra -Werror -O2`.
- Integridade do ZIP: verificada.

## Limitações
- Toolchain libXenon / `xenon-gcc`: não validada.
- `xenon.elf` e `default.xex`: não gerados ou confirmados.
- Teste em Xbox 360 RGH: pendente.
- O ZIP completo foi gerado localmente, mas não está anexado como artefato de release neste repositório.
