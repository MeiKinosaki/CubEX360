# v1.30b — teste de integração do núcleo

O pacote de desenvolvimento v1.30b inclui um alvo de diagnóstico libXenon que vincula o código atual de memória, barramento, estado de CPU e interpretador. O teste escreve a instrução PowerPC `addi r3,r0,42` em RAM GameCube simulada e executa uma etapa do interpretador.

## Validação host
A versão inclui `tests/test_core_smoke.c`, que verifica RAM de 24 MiB, alias cached, leitura big-endian, execução da instrução e avanço do PC. A suíte nativa passou com C99, `-Wall -Wextra -Werror -O2`.

## Estado de publicação
O teste está no pacote local v1.30b; o código-fonte completo dessa versão ainda não foi commitado nesta árvore. A próxima tarefa é publicar todos os fontes e adaptar o workflow remoto para compilar `cubex360_core_smoke.elf32`.

Esse smoke test não é um emulador completo, não carrega DOL/ISO e não produz `default.xex`.
