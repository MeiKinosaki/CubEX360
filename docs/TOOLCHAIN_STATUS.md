# Status da toolchain — CubEX360

## Último marco confirmado: ELF32 de diagnóstico

A execução [38219582132](https://github.com/MeiKinosaki/CubEX360/actions/runs/38219582132) terminou com sucesso em 12/10/2026. Os passos de build libXenon e upload do artefato foram aprovados.

Artefato: `cubex360_toolchain_probe.elf32`
- Formato detectado: `ELF 32-bit MSB executable, PowerPC, statically linked, stripped`.
- SHA-256 calculado novamente após baixar o artefato: `96d490d9c467eca735f571cfd8327d1838fbb23a791bbf9e64bac7d5c2648854`.
- ZIP de CI disponível na execução do workflow (artefato temporário com expiração em 90 dias).

## O que isso comprova
- O runner consegue obter o contêiner Free60/libXenon.
- `xenon-gcc` compila o programa mínimo de diagnóstico.
- O pipeline publica um ELF32 e inclui verificação de formato e hash.

## O que NÃO comprova
- O ELF não é o núcleo completo do CubEX360.
- O núcleo do pacote local v1.30b ainda não foi integrado ao alvo do workflow.
- `default.xex` não foi gerado.
- Não há validação de inicialização em Xbox 360 RGH.

## Próximas etapas
1. Publicar integralmente os fontes do núcleo na árvore GitHub.
2. Atualizar o workflow para compilar e linkar memória, barramento e CPU do núcleo.
3. Obter e inspecionar um ELF produzido pelo alvo integrado.
4. Selecionar toolchain/runtime XEX compatível, verificar imports/ABI/estrutura e só então produzir `default.xex`.
5. Testar o executável no hardware compatível.
