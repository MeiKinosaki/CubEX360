# CubEX360 — toolchain-first development

Projeto experimental de interpretador parcial PowerPC/Gekko para um futuro emulador de GameCube no Xbox 360.

## Toolchain
O workflow [CubEX360 libXenon toolchain probe](.github/workflows/xenon-build.yml) compila o programa mínimo `xenon-probe` com Free60/libXenon, verifica o formato ELF32 PowerPC big-endian e publica relatório e SHA-256 como artefatos do CI.

Último probe confirmado: https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703

As execuções recentes que testam o workflow revisado estão em andamento:
https://github.com/MeiKinosaki/CubEX360/actions

## Limite atual
O probe valida a disponibilidade básica da toolchain, não o núcleo completo do CubEX360. O pacote de desenvolvimento v1.30b possui fontes de memória, barramento, estado da CPU e um interpretador parcial; os 21 testes host passaram localmente. Esses fontes ainda não foram todos publicados na árvore principal do repositório nem ligados no ELF de CI.

## default.xex
Ainda não foi gerado. libXenon fornece o caminho ELF/ELF32 e não converte automaticamente para XEX. Os scripts `scripts/xex-preflight.sh` e `scripts/verify-xenon-elf.sh` explicitam esses requisitos e bloqueios; não renomeie ELF para XEX.

Próximos passos: confirmar a execução do probe revisado, publicar o núcleo completo, compilar o alvo integrado e, separadamente, validar uma toolchain/runtime XEX e testar o artefato em hardware.
