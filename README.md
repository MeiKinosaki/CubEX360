# CubEX360 — toolchain bootstrap (v1.28b)

Projeto experimental de interpretador parcial PowerPC/Gekko para um emulador de GameCube no Xbox 360.

## Marco de toolchain concluído

O workflow [CubEX360 libXenon toolchain probe](https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703) terminou com sucesso em 12/10/2026. O contêiner Free60/libXenon compilou um programa mínimo de diagnóstico e publicou o artefato `cubex360_toolchain_probe.elf32`.

- Formato verificado localmente: ELF 32-bit, big-endian, PowerPC.
- SHA-256: `96d490d9c467eca735f571cfd8327d1838fbb23a791bbf9e64bac7d5c2648854`.
- [Execução CI](https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703).
- O artefato de CI é apenas um probe de toolchain, não contém o núcleo de emulação.

## Próximas etapas

1. Adicionar ao repositório o código-fonte integral do núcleo CubEX360.
2. Criar um alvo libXenon que compile e vincule esse núcleo, em vez do probe isolado.
3. Validar separadamente a cadeia que produz `default.xex`. LibXenon produz ELF/ELF32 e não converte automaticamente para XEX.
4. Verificar o XEX gerado e testar em hardware compatível.

Consulte [status da toolchain](docs/TOOLCHAIN_STATUS.md) e [caminho para default.xex](docs/DEFAULT_XEX.md).

## Limitações atuais

O probe não é um emulador. O núcleo completo ainda não está presente nesta árvore do repositório; a integração da CPU Gekko, memória, GPU, DSP, FPU e sistema permanece pendente. Não há `default.xex` confirmado.
