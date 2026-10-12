# Status da toolchain — CubEX360 v1.28b

## Diagnóstico CI (12/10/2026)
O workflow conseguiu baixar a imagem `free60/libxenon:latest` e localizar `/usr/local/xenon/bin/xenon-gcc`. A primeira compilação falhou no link com `undefined reference to main`, porque o Makefile não declarava explicitamente os objetos como dependências do alvo ELF.

## Correção aplicada
O Makefile foi corrigido para declarar `$(OUTPUT).elf: $(OFILES)`. Um novo push deve disparar o workflow; é necessário aguardar e inspecionar essa execução antes de afirmar sucesso.

## Estado atual
- Toolchain: imagem e `xenon-gcc` encontrados no runner.
- Compilação do probe: falhou antes da correção; execução pós-correção ainda precisa ser verificada.
- `cubex360_toolchain_probe.elf32`: não confirmado.
- Núcleo CubEX360 integrado ao alvo: pendente.
- `default.xex`: não gerado. O fluxo libXenon atual gera ELF32 e não fornece uma conversão ELF→XEX validada.
- Teste no Xbox 360 RGH: pendente.
