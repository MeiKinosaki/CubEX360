# CubEX360 — arquitetura modular e integração contínua (v1.84b)

## Contratos de módulo

O projeto deve manter interfaces pequenas e dependências direcionadas. Um módulo não deve acessar diretamente a implementação interna de outro quando uma API pública pode ser usada.

| Frente | Responsabilidade | Contrato/artefato atual | Critério para fechar a etapa |
|---|---|---|---|
| A — Núcleo | Gekko, instruções, CR/XER, exceções, MMU e interrupções | `cpu.h`, `cpu.c`, `memory.h`, `memory.c`, `bus.h`, `bus.c` | Testes por instrução, memória e estados de erro; cobertura documentada |
| B — Hardware | Flipper/GPU, DSP/áudio, DMA, timers e periféricos | Ainda não há implementação completa dos dispositivos | Testes de registros MMIO, interrupções e eventos por dispositivo |
| C — Xbox 360 | libXenon, entrada, runtime, ELF e XEX | `core-src/Makefile`, `.github/workflows/xenon-build.yml`, scripts de preflight | ELF verificado; XEX só fechado após ferramenta real e execução/validação |
| D — Interface/persistência | Biblioteca, favoritos, capas, perfis, save/load | `frontend/*.json`, `covers.h/c`, `state.h/c` | Frontend consome configurações; testes de persistência e rede; estados completos por subsistema |
| E — Integração/validação | Testes host, sanitizers, CI e artefatos | `scripts/run-host-tests.sh`, workflow de GitHub Actions | Testes verdes para o commit exato; artefatos com hash e relatório |

## Regra de integração

1. Cada frente altera seu módulo e seus testes; mudanças em contratos compartilhados devem ser pequenas e revisadas.
2. Toda mudança C no núcleo ou nos contratos deve executar o teste host estrito.
3. Mudanças em memória, decodificação ou persistência também executam sanitizers.
4. O workflow de toolchain é separado: testes host verdes não provam que ELF/XEX funciona no console.
5. Não marcar uma etapa concluída sem resultado ligado ao SHA exato do commit.
6. Reaproveitar código externo apenas após conferir licença, arquitetura, dependências e compatibilidade com o Xbox 360; registrar origem e modificações.
7. Perfilamento será feito com cargas reproduzíveis e contadores medidos. Não atribuir gargalos por estimativa.

## Estado real nesta etapa

Foi criado um script comum para executar os testes host em modo estrito ou com ASan/UBSan. O workflow chama o mesmo script para evitar divergência entre CI e execução reproduzível. As frentes B e D continuam majoritariamente em estrutura/configuração, e o XEX final permanece não comprovado.


## v1.85b — validação automática de configuração

O workflow agora tem um job separado para verificar os arquivos JSON de biblioteca, perfis, filtros e sequência de abertura. O script também confere a presença dos perfis Quality, Balanced e Performance e exige filtros não vazios. A execução CI deste commit ainda precisa ser confirmada; esta verificação não demonstra que os menus já estão conectados ao runtime.


## v1.86b — transferências de memória em bloco

A Frente A expõe agora operações de cópia de bytes com validação de limites para facilitar futuros consumidores como DMA e save/load state. A API usa `memmove` e tem testes host de limites, ponteiros e round-trip. A execução CI ainda precisa ser verificada no commit correspondente. Esta API não equivale a DMA ou estado completo de hardware implementados.


## v1.87b — cópias sobrepostas na memória

Os testes host cobrem agora sobreposição para ambos os sentidos de cópia usando a API de transferência em bloco. O objetivo é proteger a semântica `memmove` que a implementação utiliza. Esta alteração é somente uma regressão de teste; não implementa DMA, estados completos de hardware ou runtime do frontend. A execução CI para o SHA `b954c77e4b4e0ca9d31e12a40f988dd022fe1ce4` ainda precisa ser confirmada.
