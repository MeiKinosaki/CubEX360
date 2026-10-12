# CubEX360 — toolchain-first development (v1.31b)

Projeto experimental de interpretador parcial PowerPC/Gekko para um futuro emulador de GameCube no Xbox 360.

## Build libXenon verificado
O workflow [run 38219582132](https://github.com/MeiKinosaki/CubEX360/actions/runs/38219582132) concluiu com sucesso e publicou o artefato `cubex360_toolchain_probe.elf32`.
- Formato: ELF32 PowerPC big-endian.
- SHA-256: `96d490d9c467eca735f571cfd8327d1838fbb23a791bbf9e64bac7d5c2648854`.
- O artefato é somente um probe de toolchain, não o núcleo do emulador.

## v1.31b — pacote local
O pacote de desenvolvimento v1.31b inclui relatório do ELF verificado, script de validação de formato/hash e gate de pré-verificação XEX. A suíte host passou com 21 testes; o gate XEX bloqueia corretamente porque ainda não há ferramentas XEX validadas configuradas.

**Nota de publicação:** o ZIP v1.31b foi criado e testado no ambiente de desenvolvimento, mas ainda não foi publicado integralmente no GitHub como release/anexo.

## Bloqueios
- O núcleo completo ainda não está integrado ao alvo libXenon do workflow.
- A toolchain/runtime XEX não foi identificada nem validada.
- `default.xex` não foi gerado nem testado em Xbox 360 RGH.

Consulte [status da toolchain](docs/TOOLCHAIN_STATUS.md), [relatório ELF32](docs/CI_ELF32_VERIFICATION.md) e [plano XEX](docs/DEFAULT_XEX.md).


## v1.32b — alvo de integração libXenon
Foi adicionado `core-src/` com um alvo inicial que liga memória principal de 24 MiB, barramento cached/uncached e interpretador limitado. O workflow agora tenta compilar esse alvo separadamente do probe de toolchain e publicar `cubex360_core_smoke.elf32`. O resultado só deve ser considerado confirmado após o job CI terminar e o artefato ser inspecionado.

O alvo suporta apenas um subconjunto de instruções PowerPC e é um smoke test, não um emulador completo. `default.xex` continua pendente.


## v1.35b — operações imediatas PowerPC adicionais

O interpretador parcial agora também implementa `ORIS`, `XORI` e `ANDIS.`, além de `ANDI.`. Os testes host foram expandidos para validar os valores resultantes e CR0 quando a instrução atualiza o campo de condição. Os commits mais recentes acionaram o CI; confira o estado antes de tratar essa revisão como aprovada.

- [CPU](core-src/src/cpu.c)
- [Testes](core-src/tests/test_core_host.c)
- [CI](https://github.com/MeiKinosaki/CubEX360/actions)

O núcleo continua parcial; `default.xex` ainda não foi gerado.
