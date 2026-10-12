# Status da toolchain — CubEX360 v1.28b

## ELF32 de diagnóstico — aprovado

Em 12/10/2026, o workflow [run 38217103703](https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703) terminou com sucesso.
- Imagem `free60/libxenon:latest` baixada e `xenon-gcc` encontrado.
- O Makefile foi corrigido para vincular `$(OFILES)` no alvo ELF.
- O artefato `cubex360_toolchain_probe.elf32` foi publicado no CI e baixado para inspeção.
- `file` identificou `ELF 32-bit MSB executable, PowerPC, statically linked, stripped`.
- SHA-256 verificado: `96d490d9c467eca735f571cfd8327d1838fbb23a791bbf9e64bac7d5c2648854`.

## Limite do marco

Esse resultado comprova que a toolchain compila um programa mínimo libXenon; não comprova que o núcleo CubEX360 foi compilado, pois os fontes completos do núcleo não estão na árvore atual do repositório. O ELF é apenas um probe de console.

## Próximas tarefas

1. Publicar os fontes completos do núcleo e seu Makefile de destino.
2. Integrar o núcleo ao alvo libXenon e compilar um ELF do CubEX360, não o probe.
3. Validar a compatibilidade do ABI e o fluxo de criação de XEX em um projeto separado.
4. Verificar o XEX e testar no hardware alvo.

## Estado do XEX

- `default.xex`: não gerado.
- Não renomear ELF32 para XEX; são formatos diferentes.
- O caminho XEX requer toolchain/empacotamento compatível com o loader Xbox 360. Nenhum artefato XEX válido foi produzido por este projeto até agora.
