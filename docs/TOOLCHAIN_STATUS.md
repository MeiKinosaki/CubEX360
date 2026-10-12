# Status da toolchain — CubEX360 v1.30b

## Novo teste de integração no pacote local
O pacote v1.30b contém um alvo libXenon que compila os fontes atuais de memória, barramento, estado da CPU e interpretador. O smoke test tenta validar RAM de 24 MiB, o alias de memória, leitura big-endian e a execução de `addi r3,r0,42`.

## Validação local
- `make clean test`: 21 executáveis nativos passaram, incluindo `test_core_smoke`.
- Compilação nativa: C99 com `-Wall -Wextra -Werror -O2`.
- ZIP v1.30b: integridade confirmada.
- Hash SHA-256 do ZIP: `c193b4695adf09c951279beb1a898660d9a35f10d11166e3e25dc09d8bf75da0`.

## CI e publicação
O workflow remoto existente validou somente o programa de diagnóstico no run https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703. A nova compilação do núcleo integrado ainda não foi executada: o pacote v1.30b não foi publicado integralmente na árvore do repositório, então não declarar o build integrado como aprovado.

## Bloqueios
- Publicar o código-fonte completo v1.30b e atualizar o workflow remoto para compilar o alvo integrado.
- Confirmar um ELF32 gerado pela nova execução CI.
- Identificar e validar a cadeia de ferramentas XEX compatível.
- `default.xex` não foi gerado nem testado no Xbox 360 RGH.
