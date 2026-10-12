# Status da toolchain — v1.29b

## ELF32
O workflow libXenon de diagnóstico terminou com sucesso:
https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703

O artefato é um ELF32 PowerPC big-endian de diagnóstico. Ele não contém o núcleo completo do emulador e não é `default.xex`.

## v1.29b — gates locais
- `make clean test`: 20 executáveis nativos passaram localmente.
- `unzip -t CubEX360_v1.29b_toolchain_gate.zip`: integridade verificada.
- `make verify-xenon-elf ELF32=/caminho/arquivo.elf32`: valida o formato ELF e imprime SHA-256.
- `make xex-preflight`: testado; retorna código 2 e declara o bloqueio se as ferramentas XEX não estiverem configuradas.

## Bloqueios
- O código-fonte completo ainda não está publicado nesta árvore GitHub.
- O ELF atual é apenas um probe, não uma build do núcleo CubEX360.
- Não há compilador/empacotador XEX validado neste projeto.
- `default.xex` não foi gerado nem testado em hardware.

## Próximas tarefas
1. Publicar os fontes completos do núcleo.
2. Integrar e compilar o núcleo com libXenon.
3. Identificar toolchain/runtime XEX e verificar imports/estrutura.
4. Testar o XEX real no Xbox 360 RGH.
