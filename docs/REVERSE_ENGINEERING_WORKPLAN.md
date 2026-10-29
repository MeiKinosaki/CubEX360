# CubEX360 — plano de engenharia reversa orientada por evidências

## Objetivo

Acelerar o desenvolvimento em frentes paralelas sem confundir código experimental com comportamento confirmado do Nintendo GameCube ou do Xbox 360. O trabalho deve usar documentação pública, implementações de referência com licença compatível, testes próprios e dumps/traços obtidos legalmente de hardware ou software que o usuário tem autorização para analisar.

## Regras de evidência

Para cada recurso, manter quatro estados distintos:

1. **Especificado** — comportamento descrito por documentação técnica identificável.
2. **Implementado** — código presente no repositório.
3. **Testado em CI** — testes reproduzíveis passaram no commit citado.
4. **Validado no alvo** — comportamento observado no Xbox 360 RGH ou no hardware/emulador de referência, com procedimento e resultado registrados.

Um teste host não comprova temporização exata, compatibilidade de jogos ou inicialização no console. ELF32 não significa que existe um `default.xex` funcional.

## Frentes paralelas

### A — CPU Gekko / PowerPC
- Construir um corpus de instruções com palavras de instrução conhecidas, estado inicial, resultado esperado e exceções esperadas.
- Priorizar instruções inteiras, CR/XER, desvios, cargas/armazenamentos, alinhamento e atualização de registradores.
- Comparar cada teste com documentação PowerPC/Gekko e, quando permitido pela licença, uma implementação de referência.
- Registrar opcode implementado, opcode parcial e opcode ainda não suportado; nunca deduzir cobertura total a partir de alguns testes.

### B — Memória, barramento e MMIO
- Manter casos de fronteira para RAM física, aliases cached/uncached, alinhamento, overflow e endereços não mapeados.
- Usar mapa de memória documentado antes de associar registradores reais a dispositivos.
- Acrescentar despacho MMIO por regiões só depois de definir semântica e testes; não inventar endereços ou efeitos colaterais.

### C — DMA, temporizadores e interrupções
- Separar a primitiva atual de cópia RAM–RAM de um modelo de canais DMA real.
- Especificar registradores, disparadores, conclusão, cancelamento, prioridade e interrupções a partir de referências técnicas verificáveis.
- Testar eventos em limites de ciclo, avanço por múltiplos períodos, pausa e limpeza de flags.
- Comparar traços de eventos, não apenas o valor final da memória.

### D — Flipper / vídeo / DSP / áudio
- Criar modelos pequenos e isolados com entradas/saídas determinísticas.
- Começar por formatos, comandos e registradores documentados; adicionar testes de regressão para cada comportamento.
- Não declarar renderização, sincronização ou áudio reais antes de haver um caminho integrado e uma amostra reproduzível.

### E — Xbox 360 / toolchain
- Manter o build libXenon na nuvem, com logs e artefatos por commit.
- Separar o ELF32 de smoke test, a conversão ELF→XEX, a validação estrutural e o boot real.
- O portão XEX deve falhar explicitamente se o conversor ou as ferramentas necessárias não estiverem disponíveis; não criar arquivos fictícios para fazer o job passar.

### F — Interface, biblioteca e save states
- Desenvolver UI/configuração separadamente do núcleo de emulação.
- Testar parsing de configuração, caminhos, nomes de jogos, cache de capas e validação de arquivos.
- Para save states, registrar versão do formato, integridade, tamanho, estados de CPU/RAM/dispositivos e testes de round-trip. Não declarar estado completo enquanto faltarem dispositivos.

## Método de comparação

Para cada hipótese de engenharia reversa:

1. Escrever a hipótese e citar a fonte técnica.
2. Criar o menor caso de teste que possa refutá-la.
3. Executar o teste no host e com sanitizers.
4. Comparar com uma referência autorizada ou traço reproduzível.
5. Corrigir a implementação e adicionar o caso como regressão.
6. Registrar commit, workflow, jobs, artefatos e limitações.

Não copiar código de referência sem verificar licença e atribuição. Não contornar DRM, proteções de acesso ou controles de segurança. Analisar interfaces e comportamento técnico é suficiente para este plano.

## Situação conhecida no checkpoint

- MMIO: banco genérico de registradores com testes de limites; ainda não é o mapa real completo do Flipper.
- DMA: cópia síncrona RAM–RAM; não representa canais DMA reais.
- Timer: primitiva determinística one-shot/periódica e flag de interrupção; CI do commit `cdf53032b7c2fd10aaa920b9c895b96bf64d2ddb` passou nos seis jobs: [workflow 40833654991](https://github.com/MeiKinosaki/CubEX360/actions/runs/40833654991).
- CPU: interpretador parcial; cobertura de instruções continua incompleta.
- Xbox 360: artefatos ELF32 de smoke test já foram produzidos em runs anteriores; isso não comprova `default.xex` nem boot em console.

## Próximo lote de trabalho

1. Criar corpus de testes de instruções Gekko com resultados esperados.
2. Expandir DMA para descriptor/canal experimental apenas após especificação verificável.
3. Definir contrato de despacho MMIO e registrar dispositivos ainda não mapeados.
4. Cobrir timer com overflow e fronteiras adicionais.
5. Fazer o portão de XEX relatar exatamente qual ferramenta ou validação falta.
6. Publicar estimativas de progresso somente como estimativas, separadas dos estados de CI e validação em alvo.
