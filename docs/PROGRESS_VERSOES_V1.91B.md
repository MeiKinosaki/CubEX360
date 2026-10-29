# CubEX360 — checkpoint incremental v1.91b

Data: 2026-10-29

## v1.90b — DMA RAM e regressão

**Concluído com CI aprovado** no commit `f8a118da2a8c4603637cc56589c57c96bff0b791`.

- Protótipo de cópia RAM–RAM com limites, aliases cached/uncached e comportamento de sobreposição via `memmove`.
- Testes host e sanitizers aprovados: [GitHub Actions 40833019129](https://github.com/MeiKinosaki/CubEX360/actions/runs/40833019129).
- Limite: isto ainda não modela canais DMA reais do GameCube.

## v1.91b — temporizador determinístico e sinal de interrupção

**Em validação CI; não marcar como concluído até os jobs terminarem.**

- Adicionado `cubex_timer_t` com modo one-shot/periódico, contagem determinística e flag `interrupt_pending`.
- Testes para expiração, pausa, limpeza da flag e avanço atravessando múltiplos períodos.
- Corrigido comportamento de chamada após expiração de one-shot desabilitado.
- CI: [histórico de Actions](https://github.com/MeiKinosaki/CubEX360/actions).

## Limites que permanecem

- O timer é uma primitiva genérica, não a implementação de um temporizador específico do GameCube.
- O MMIO é um banco genérico de registradores; ainda não há mapa real completo do Flipper/DSP.
- ELF32 de smoke test não equivale a `default.xex`; geração e boot no Xbox 360 RGH ainda não estão comprovados.
- Não há alegação de compatibilidade de jogos comerciais.
