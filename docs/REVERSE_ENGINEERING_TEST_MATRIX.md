# CubEX360 — matriz de regressão para próxima etapa

Esta matriz define casos que precisam ser exercitados; não declara que já foram executados.

## CPU: desvios
- BC: condição CR verdadeira e falsa; CTR decrementado quando exigido; BO com variantes de ignorar CTR/CR; deslocamento positivo/negativo; AA e LK.
- B: destino relativo e absoluto, sinal do deslocamento e LK.
- BCLR/BLR: destino baseado no LR alinhado, condição tomada/não tomada e atualização de LR.
- BCCTR/BCTR: destino baseado no CTR alinhado, sem decremento indevido de CTR, condição CR e LK.
- Para cada caso: verificar PC final, LR, CTR e contador de instruções aposentadas.

## CR/XER
- Verificar os oito campos CR individualmente, sem alterar campos não selecionados.
- Verificar MFCR/MTCRF e MCRXR, incluindo quais bits de XER são transferidos/limpos conforme especificação.
- Verificar CA em operações com carry/borrow, OV nas variantes com overflow e SO sticky.
- Testar valores zero, positivos, negativos e fronteiras signed/unsigned.

## Load/store
- Byte, halfword e word: dados big-endian, extensão signed/unsigned, alinhamento e limites da RAM.
- Instruções com atualização: RA não pode ser zero nas formas proibidas; testar restrição RA==RT para loads com update.
- Falha de memória deve deixar registradores de destino/base sem atualização parcial.
- LMW/STMW: validar intervalo completo antes de mutar estado e testar último endereço válido.

## DMA e timer
- DMA: tamanho 0, 1, limite exato, excedente por um byte, endereço não mapeado, aliases e sobreposição nos dois sentidos.
- Timer one-shot/periódico: 0 ciclos, período-1, período, período+1, vários períodos, pausa/retomada e limpeza da interrupção.
- Revisar overflow e semântica de interrupção pendente quando múltiplos vencimentos ocorrem em uma única chamada.

## Save states
- Round-trip de todos os campos serializados e RAM.
- Rejeitar magic/versão/tamanho inválidos, truncamento, bytes extras e checksum incorreto.
- Em qualquer falha, CPU e RAM ativas devem permanecer inalteradas.
- Testar falha de abertura/escrita/renomeação quando possível.

## Gates de validação
1. Executar `sh scripts/run-host-tests.sh strict`.
2. Executar `sh scripts/run-host-tests.sh sanitize`.
3. Registrar SHA exato, status e logs do GitHub Actions.
4. Verificar artefato ELF32 e relatório/hash do mesmo SHA.
5. Manter `default.xex` como pendente até ferramenta XEX real, ABI/imports e teste apropriado estarem comprovados.

Nenhuma das tarefas listadas acima é considerada concluída só pela existência deste documento.
