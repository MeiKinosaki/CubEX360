# CubEX360 — toolchain gate v1.29b

Projeto experimental de interpretador parcial PowerPC/Gekko para um emulador de GameCube no Xbox 360.

## Marco anterior: ELF32 libXenon

O workflow de diagnóstico terminou com sucesso: https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703

Ele produziu um ELF32 PowerPC big-endian para testar a toolchain. Esse programa é somente um probe e não contém o núcleo completo do emulador.

## v1.29b — verificações reproduzíveis
- Adiciona `make verify-xenon-elf ELF32=/caminho/arquivo.elf32` para verificar o tipo ELF32 PowerPC e imprimir SHA-256.
- Adiciona `make xex-preflight`, que falha de forma explícita enquanto não houver compilador e empacotador XEX compatíveis configurados.
- A suíte host foi executada localmente: 20 executáveis de teste passaram.
- Integridade do ZIP da v1.29b verificada.

Pacote local: `CubEX360_v1.29b_toolchain_gate.zip`. O pacote ainda não foi anexado como artefato de release e seu código-fonte integral ainda não está commitado nesta branch.

## Próximas etapas
1. Publicar o núcleo completo e integrar seu alvo de compilação.
2. Produzir um ELF do emulador, não apenas do probe.
3. Validar uma toolchain/runtime XEX real, verificar o executável e testar no Xbox 360 RGH.

**`default.xex` ainda não foi gerado.** Não renomeie ELF para XEX. Consulte [status da toolchain](docs/TOOLCHAIN_STATUS.md) e [plano XEX](docs/DEFAULT_XEX.md).
