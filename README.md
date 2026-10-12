# CubEX360 v1.27b — SPR de salvamento de exceção

Projeto experimental de interpretador parcial de instruções PowerPC/Gekko para servir de base a um emulador de GameCube no Xbox 360.

## Mudanças da v1.27b
- Amplia `mfspr` para ler SRR0 (SPR 26) e SRR1 (SPR 27).
- Amplia `mtspr` para gravar SRR0 (SPR 26) e SRR1 (SPR 27).
- Testes de regressão verificam a gravação e leitura dos dois registradores.

## Validação local
- `make clean test`: 20 executáveis de teste nativos passaram.
- Compilação C99 com `-Wall -Wextra -Werror -O2`.
- Integridade do ZIP verificada.

## Estado da publicação
O pacote ZIP foi criado e validado localmente. Esta documentação está publicada no GitHub, mas o código-fonte integral da v1.27b e o ZIP ainda não foram anexados ao repositório.

## Limitações
O mapeamento de SPRs é parcial e ainda não modela o fluxo completo de exceções nem as regras de privilégio. A toolchain libXenon/`xenon-gcc` não foi validada; `xenon.elf` e `default.xex` não foram gerados/confirmados. O teste em Xbox 360 RGH está pendente. Este projeto ainda não é um emulador completo de GameCube.
