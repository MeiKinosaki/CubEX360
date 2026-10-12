# Status da toolchain — CubEX360

## Estado confirmado
- O workflow anterior compilou um programa mínimo com Free60/libXenon e produziu ELF32 PowerPC big-endian: https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703
- O pacote local v1.30b foi extraído e `make clean test` executou com sucesso: 21 testes nativos, incluindo `test_core_smoke`.
- O workflow foi corrigido para verificar o ELF e publicar relatório e SHA-256.
- Execuções do workflow após as correções ainda estão em andamento: https://github.com/MeiKinosaki/CubEX360/actions

## Limitações
- O ELF confirmado é somente um probe; não contém o núcleo completo do emulador.
- Os fontes completos do pacote v1.30b ainda não estão todos commitados nesta árvore.
- `default.xex` não foi gerado.
- A toolchain/runtime XEX não foi identificada e validada neste projeto.
- Teste em Xbox 360 RGH pendente.

## Próximas etapas
1. Confirmar que o workflow revisado passa e verificar o artefato ELF.
2. Publicar integralmente os fontes do núcleo.
3. Compilar um ELF que ligue o núcleo CubEX360.
4. Selecionar uma toolchain XEX compatível, validar formato/imports/ABI e testar em hardware.
