# CubEX360 v1.22b — transferência do registrador de condição

## Mudanças
- Implementação parcial de `mfcr`, para copiar o registrador CR para um GPR.
- Implementação parcial de `mtcrf`, para atualizar campos selecionados do CR conforme a máscara FXM.
- Testes dedicados para transferência completa, máscara parcial, máscara completa e máscara zero.

## Validação e limites
O pacote preparado localmente inclui a suíte de testes nativa em C. Os testes de host não comprovam a compilação ou execução no Xbox 360.

- Toolchain libXenon / `xenon-gcc`: ainda não validada neste ambiente.
- `xenon.elf`: não gerado/confirmado.
- `default.xex`: não gerado/confirmado.
- Teste em hardware Xbox 360 RGH: pendente.

## Estado do artefato
Esta página documenta a versão v1.22b. O ZIP completo da fonte foi criado na sessão de desenvolvimento, mas ainda não foi enviado como arquivo binário para este repositório. Não trate esta página sozinha como o código-fonte completo ou um lançamento binário.
