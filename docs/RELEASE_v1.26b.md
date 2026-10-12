# CubEX360 v1.26b — instruções MSR

## Mudanças
- Suporte parcial a `mfmsr rD` para copiar MSR para um registrador geral.
- Suporte parcial a `mtmsr rS` para gravar um registrador geral no MSR.
- Novo teste para os dois caminhos e rejeição de campos reservados.

## Validação local
- 20 executáveis de teste nativos passaram com C99, `-Wall -Wextra -Werror -O2`.
- Integridade do ZIP verificada.

## Limitações
O tratamento de privilégios, efeitos colaterais do MSR e exceções ainda não foi implementado. libXenon/`xenon-gcc` não foi validado; `xenon.elf` e `default.xex` não foram gerados/confirmados; teste em hardware pendente. O ZIP completo foi criado localmente, mas ainda não foi anexado ao repositório como artefato e o código integral não foi publicado nesta branch.
