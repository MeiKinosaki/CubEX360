# CubEX360 — REVERSE ENGINEERING WORKPLAN

## Regras de validação
- Não promover uma versão de desenvolvimento enquanto os testes relacionados à etapa estiverem pendentes ou falhando.
- Registrar o SHA do commit e os resultados dos jobs do GitHub Actions antes de declarar uma etapa aprovada.
- Separar implementação de código, validação host/sanitizers, build ELF32, geração XEX e teste no Xbox 360 RGH.
- Um ELF32 PowerPC big-endian de smoke test não comprova um `default.xex` funcional nem compatibilidade com jogos.
- Tratar documentação técnica como fonte para endereços, campos, efeitos colaterais e semântica; marcar hipóteses explicitamente.

## Frente A — CPU PowerPC/Gekko
1. Expandir regressões de desvios: `BC`, `B`, `BCLR/BLR`, `BCCTR/BCTR`; testar condições tomadas/não tomadas, BO/BI, CTR, LR, AA/LK e alvos alinhados.
2. Ampliar testes de CR/XER: seleção de campos CR, CR0.SO, `MFCR/MTCRF/MCRXR`, carry CA, overflow OV e sticky SO; validar preservação dos bits não afetados.
3. Ampliar acessos de memória: loads/stores por byte, halfword e word, extensão de sinal, alinhamento, limites, falha de acesso, RA=0, restrições de registradores e atualização de RA somente após acesso bem-sucedido.
4. Conferir os casos de borda contra o manual PowerPC/IBM e documentação técnica Gekko disponível; não inferir comportamento de hardware ausente.

## Frente B — barramento, memória e MMIO
1. Preparar uma tabela de regiões/endereço, largura, alinhamento, permissões e efeitos colaterais a partir de documentação verificável do GameCube/Gekko.
2. Manter o banco genérico MMIO como infraestrutura; não mapear registradores específicos nem atribuir efeitos sem fonte e testes.
3. Testar endereço abaixo/acima da região, overflow de cálculo, acesso desalinhado, valores de fronteira e preservação do estado em falha.
4. Documentar explicitamente que os aliases cached/uncached atuais são simplificados e não modelam cache/coerência nem MMU/TLB.

## Frente C — DMA e timers
1. DMA: testar tamanho zero, um byte, último byte válido, limites de origem/destino, aliases cached/uncached, regiões inválidas e cópias sobrepostas.
2. Timer: testar avanço zero, um ciclo antes/no ciclo/depois do vencimento, períodos múltiplos em uma única chamada, pausa/retomada, limpeza de interrupção e overflow aritmético.
3. Manter as primitivas determinísticas isoladas; não alegar equivalência com controladores GameCube sem especificação e integração de eventos/IRQ.

## Frente D — vídeo e DSP
1. Primeiro identificar interfaces, formatos de dados, registradores e fluxo de comandos com referências técnicas rastreáveis.
2. Registrar o que é conhecido, hipótese e desconhecido; adicionar testes de parsing/validação antes de implementar comportamento gráfico ou DSP.
3. Não declarar renderização GameCube, áudio DSP ou compatibilidade de jogos enquanto não houver implementação integrada e evidência de execução.

## Frente E — Xbox 360 e toolchain em nuvem
1. Manter builds host estritos, ASan/UBSan e configuração no GitHub Actions.
2. Validar ELF32 pela classe, endianess, arquitetura, entry point e hash; preservar os relatórios e artefatos do SHA exato.
3. Continuar a pré-verificação XEX em modo fail-closed. Não renomear ELF para `default.xex`; exigir ferramentas autorizadas reais, runtime/ABI/imports e formato XEX validados.
4. Separar explicitamente: testes host aprovados, ELF32 produzido, XEX gerado, boot RGH e jogo executado.

## Frente F — save states e interface
1. Testar round-trip, checksum inválido, truncamento, dados extras, versão/magic inválidos, falha de I/O e garantia de que uma carga inválida não modifica CPU/RAM.
2. Registrar o escopo atual: CPU + RAM principal; periféricos ainda não fazem parte do estado salvo.
3. Tratar JSONs de frontend como configuração/estrutura até existir UI runtime integrada; testar esquema, persistência, callbacks e falhas de arquivo.
4. Capas e rede: usar testes simulados para lógica local, mas não confundi-los com downloads reais no console.

## Sequência de trabalho sugerida
1. Auditar e ampliar testes de desvios, CR/XER e memória sem adicionar novas instruções antes de estabilizar as regressões existentes.
2. Rodar os jobs host estrito e sanitizers no commit exato; corrigir todas as falhas e repetir.
3. Expandir os testes de limites/sequências de DMA e timer; executar novamente host e sanitizers.
4. Documentar a tabela MMIO com referências antes de associar endereços a dispositivos.
5. Validar a integridade de save states e a configuração de frontend.
6. Inspecionar o resultado do build libXenon e os artefatos; manter XEX bloqueado até existir uma cadeia de ferramentas validada.

## Estado inicial deste plano
O README no branch `main` descreve desenvolvimento até v1.89b e registra pendências de CI para etapas recentes. Este documento organiza as próximas tarefas; sua criação não significa que qualquer teste foi executado ou que uma versão nova foi concluída.
