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
