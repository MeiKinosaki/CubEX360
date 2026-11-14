# CubEX360 — checkpoint incremental v1.92b (v1.93b em andamento)

Data: 2026-11-14

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


## CPU CR/XER regression suite — adicionada, CI pendente

- Novo `core-src/tests/test_cr_xer_edges.c`: cobre `MFCR`, máscara seletiva `MTCRF`, transferência `MCRXR` para campos CR distintos e limpeza apenas de SO/OV/CA.
- A suíte foi integrada aos modos `strict` e `sanitize` de `scripts/run-host-tests.sh`.
- Esta alteração adiciona cobertura de regressão, não novos opcodes. Não promover versão nem marcar a etapa como validada até o GitHub Actions concluir no commit correspondente.


## v1.92b — barramento unificado CPU/RAM/MMIO (2026-11-14)

**Concluído e validado por CI** no commit `483067feba7d07ac010707afba572b389290aacd`.

- A estrutura de memória passou a aceitar callbacks de MMIO com contexto de barramento; inicialização e destruição limpam esses callbacks para evitar ponteiros indeterminados.
- A máquina conecta seu banco MMIO ao barramento usado pela CPU.
- Acesso de CPU a registradores MMIO agora passa pelo barramento para operações de palavra, byte e meia-palavra; transferências múltiplas de palavras também foram encaminhadas pelo barramento.
- Mantidos os aliases cached/uncached da RAM principal.
- Teste de integração executa instruções Gekko reais do subconjunto atual: `LIS`, `LI`, `STW`, `LWZ`, `STB` e `LBZ` contra o banco MMIO, verificando valor, lane big-endian e leitura de retorno.
- GitHub Actions run [#216 — 43622302875](https://github.com/MeiKinosaki/CubEX360/actions/runs/43622302875) terminou com sucesso: host strict, ASan/UBSan, frontend config, libXenon probe, core smoke ELF32 e gate XEX.

### Limites desta etapa

- O banco MMIO continua genérico (64 registradores), não é ainda o mapa real dos blocos CP, PE, VI, PI, DSP, DI, SI, EXI e AI do Flipper.
- Acesso de byte/meia-palavra usa read-modify-write sobre o banco genérico; semântica de side effects precisa ser implementada por dispositivo antes de usar em registradores reais.
- `DCBZ` ainda opera diretamente sobre RAM; precisa ser tratado separadamente com semântica de cache coerente.
- O ELF32 de smoke test foi compilado e publicado como artefato; não é `default.xex`. O gate XEX passou porque bloqueou corretamente a geração sem ferramentas XEX reais configuradas.
- Nenhum boot no Xbox 360 RGH ou compatibilidade de jogos GameCube foi comprovado.

### Próxima etapa

Substituir o banco genérico por despacho de regiões/dispositivos com mapa documentado do Flipper. Prioridade: separar janelas CP/PE, VI e PI; implementar leitura/escrita de registradores com máscaras, largura, alinhamento e side effects; depois ligar interrupções e DMA aos dispositivos reais.


## v1.93b — Flipper map decoder + primeiro modelo CP (em andamento)

### Mapa de endereços

- Adicionados `cubex_flipper_decode_address()`, enum de regiões e offsets relativos para CP, PE, VI, PI, MI, DSP, DI, SI, EXI, AI e GX FIFO.
- Janelas registradas como aperturas de decodificação; não significam que todos os offsets sejam registradores válidos.
- Corrigido tamanho da janela MI para 0x80 bytes.
- Testes de limites, lacunas e regiões adicionados em `test_flipper_map.c`.
- Documentação técnica em `docs/FLIPPER_MMIO_MAP.md`.

### Command Processor (CP) — primeiro modelo parcial

- Criado `cubex_cp_t` com interface de acesso de 16 bits, alinhamento obrigatório e validação de faixa.
- Implementados estado inicial idle, leitura de SR, máscara de CR, clear register com limpeza write-one-to-clear de overflow/underflow, token e armazenamento dos registradores de FIFO/bounding-box documentados.
- Eventos internos podem atualizar flags de status via `cubex_cp_set_status()`.
- Testes isolados em `test_cp.c`; adicionados aos modos strict e sanitize.

### Limites explícitos

- CP ainda **não está ligado ao barramento da CPU nem ao decodificador da máquina**. Os testes atuais exercitam a API do dispositivo diretamente.
- A semântica exata de alguns bits de controle/IRQ do CP é parcialmente incerta na documentação; as máscaras implementadas são conservadoras e não simulam processamento GX.
- Não há FIFO de comandos GX, GP, PE, interrupção PI ou execução de display lists implementados nesta etapa.
- O modelo ainda não foi validado por CI após as últimas alterações; aguardar a execução no commit mais recente antes de marcar os testes como aprovados.
- Nenhum boot de jogo, compatibilidade comercial ou `default.xex` foi comprovado.

### Referências

- YAGCD §5.1 — CP: https://hitmen.c02.at/files/yagcd/yagcd/chap5.html
- YAGCD §4 — mapa de memória: https://hitmen.c02.at/files/yagcd/yagcd/chap4.html

### Próxima prioridade

1. Confirmar CI strict + ASan/UBSan para mapa e CP.
2. Introduzir callbacks MMIO de 16 bits sem read-modify-write destrutivo.
3. Integrar o CP ao despacho por região, mantendo FIFO como porta especial.
4. Depois modelar PE e PI; somente então ligar as fontes de interrupção reais.
