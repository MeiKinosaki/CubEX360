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


### Integração CP ao barramento — extensão v1.93b

- Adicionados callbacks de MMIO de 16 bits em `cubex_memory_t`; inicialização e destruição limpam os quatro callbacks de 16/32 bits.
- O barramento consulta callbacks nativos para meia-palavra alinhada. Retorno `-2` significa que o dispositivo não reivindica o endereço e permite fallback ao banco genérico; erros reais do CP não fazem fallback.
- O CP foi incluído no estado da máquina e conectado ao caminho de acesso de 16 bits.
- Acesso de 32 bits à janela CP é rejeitado, evitando read-modify-write contra registradores com side effects.
- O banco genérico de teste foi deslocado para `0xCC000100`, fora da abertura CP.
- Teste integrado acrescentado: leitura de SR, escrita/leitura de CR, clear de overflow, rejeição de larguras/alinhamentos incorretos e execução de `STH`/`LHZ` da CPU contra o CP.
- Os alvos de integração strict/sanitizer agora linkam explicitamente `cp.c` e `flipper_map.c`.

**Validação:** GitHub Actions run #43623131846 foi criado para o commit `c4ff3fa0839bb696fcd79c8a8305c4f23d51d60a`. No momento do registro, o run estava queued/in progress; não marcar como aprovado até conclusão. Se falhar, corrigir com base nos logs e repetir.

**Limite:** o CP ainda não gera nem consome FIFO GX, não processa display lists, não sincroniza PE e não dispara interrupções reais via PI. O mapa e o CP permanecem em desenvolvimento.


### Reteste após correções — 2026-11-14

No run #43623206618, commit `c674c9a42dc93a972e3e208188d655f6dec70f33`:
- `host-core-test`: **success**.
- `host-sanitizers` (ASan/UBSan): **success**.
- `frontend-config-validation`: **success**.
- `xex-preflight-gate`: **success** (gate bloqueou corretamente a geração sem ferramentas XEX reais).
- `build-core-smoke` e `build-probe`: ainda em execução no momento desta atualização.

As falhas anteriores foram corrigidas: os testes de CPU para o banco genérico agora usam a janela deslocada para `0xCC000100`, a instrução `STH` grava o valor de controle esperado e o alvo sanitizer de integração vincula CP e decoder Flipper. Não marcar a etapa completa até o restante do run terminar.


**Resultado final do run #43623206618:** todos os seis jobs terminaram com sucesso (host-core-test, host-sanitizers, frontend-config-validation, xex-preflight-gate, build-core-smoke e build-probe). O artefato publicado é `cubex360-core-smoke-elf32.zip`; não é XEX. A execução completa não comprova boot em hardware Xbox 360.


## v1.93b — Pixel Engine (PE) + Processor Interface (PI), em validação

### Modelos criados

- `core-src/include/cubex360/pe.h`, `core-src/src/pe.c`: registradores PE de 16 bits para Z configuration, Alpha configuration, destination alpha, Alpha Mode, Alpha Read, Interrupt Status e PE Token.
- PE Interrupt Status no offset `0x0A`: flags de FINISH/TOKEN e bits enable correspondentes; acknowledge por write-one-to-clear nas flags de evento.
- `core-src/include/cubex360/pi.h`, `core-src/src/pi.c`: PI Cause e PI Mask em registradores de 32 bits nos offsets `0x00` e `0x04`; cause com W1C e máscara para as 14 fontes documentadas.
- Integração PE e PI no estado de `cubex_machine_t`; callbacks nativos de 16 bits para CP/PE e 32 bits para PI.
- Eventos explícitos `cubex_machine_pe_raise_finish()` e `cubex_machine_pe_raise_token()` sincronizam flags PE com os bits PI 10 (FINISH) e 9 (TOKEN).
- A causa PI permanece latched até software escrever os bits correspondentes no registrador PI Cause. O acknowledge PE e o acknowledge PI são operações distintas.
- Testes integrados verificam largura/alinhamento, enable, eventos, token, PI cause/mask, IRQ pendente e acknowledge individual/total.
- Acesso de 16 bits a regiões Flipper sem callback nativo é rejeitado, para evitar read-modify-write sobre registradores com side effects.
- Banco MMIO genérico continua acessível fora das janelas Flipper documentadas.

### Auditoria e correção baseada em documentação

- CP/PE/PI endereços e bits foram comparados com YAGCD §§5.1, 5.2 e 6.2.
- PI causa usa os bits 0–13 (ERROR, RSW, DI, SI, EXI, AI, DSP, MEM, VI, PE TOKEN, PE FINISH, CP, DEBUG, HSP).
- Referências: https://hitmen.c02.at/files/yagcd/yagcd/chap5.html e https://hitmen.c02.at/files/yagcd/yagcd/chap6.html
- Referência cruzada para os campos de controle CP: Dolphin CommandProcessor.cpp e WiiBrew Hardware/GX/Command Processor.

### Validação

- CI para commit `f3924f9151066f5bde42d7491a644064317b3afc`: `host-core-test` passou; `frontend-config-validation` passou; `xex-preflight-gate` passou. `host-sanitizers`, `build-core-smoke` e `build-probe` estavam em execução no momento deste registro.
- Run: https://github.com/MeiKinosaki/CubEX360/actions/runs/43623584025
- Corrigidos erros anteriores de símbolos de callbacks MMIO 32-bit duplicados/não utilizados e de despacho da máquina.
- Não promover a versão até ASan/UBSan e demais jobs terminarem.

### Limites

- PE ainda não rasteriza pixels, não escreve no Embedded Framebuffer (EFB), não executa blending/depth tests reais e não recebe comandos GP.
- PI ainda não injeta exceção External Interrupt no estado CPU/MSR/vetor 0x500; `cubex_pi_irq_pending()` só expõe a condição lógica pendente.
- Os eventos PE são acionados por API de teste/integração; ainda não são produzidos por um pipeline GX funcional.
- CP ainda não consome FIFO, não executa display lists e ainda não está conectado ao PI via seu evento de watermark.
- Não há execução de jogos, renderização gráfica, boot em Xbox 360 ou `default.xex` comprovados.


### CP → PI event routing — atualização incremental

- `cubex_cp_irq_pending()` calcula a saída de interrupção CP usando flags de overflow/underflow/breakpoint e seus bits de habilitação.
- A máquina sincroniza CP para PI Cause bit 11; PE Finish e PE Token seguem nos bits 10 e 9.
- `cubex_machine_cp_set_status()` permite gerar eventos de hardware de CP através da máquina e propaga a causa latched para PI.
- Escritas de controle/clear no CP recalculam a saída; a causa PI permanece latched até o W1C no PI Cause.
- Teste integrado cobre habilitar overflow IRQ, gerar overflow, observar PI bit 11, limpar status CP e reconhecer separadamente a causa PI.
- CI revelou que a primeira versão do teste tinha variável local ausente e, depois, que a sincronização CP→PI não estava chamada no caminho da máquina. Ambos os pontos foram corrigidos nos commits seguintes; aguardar o CI do commit `9c068c380fbce0b3caae6e7fd7b3db37847bdde4` ou posterior.
