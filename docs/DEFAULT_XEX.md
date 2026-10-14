# Gate de compilação para `default.xex`

## O que já foi demonstrado
- O GitHub Actions compila um programa mínimo com Free60/libXenon.
- O artefato produzido é ELF32 PowerPC big-endian, não XEX.
- Execução: https://github.com/MeiKinosaki/CubEX360/actions/runs/38217103703

## Critérios obrigatórios para aceitar um XEX
1. Compilador/linker e runtime identificados, com versões registradas.
2. O build deve linkar o núcleo CubEX360, não o programa de diagnóstico.
3. O artefato deve ser gerado pelo fluxo XEX suportado; não basta mudar a extensão do ELF.
4. Validar magic/estrutura XEX com ferramenta de inspeção compatível, registrar SHA-256 e conservar logs.
5. Fazer teste de inicialização em Xbox 360 RGH compatível antes de marcar como funcional.

## Caminhos a avaliar
- Fluxo de desenvolvimento Xbox 360 que gere executável PE/XEX compatível e permita linkar o programa com os imports/runtime necessários.
- Ferramentas de empacotamento XEX documentadas e reproduzíveis, desde que a imagem de entrada tenha sido gerada com ABI/sections/imports corretos.

## Estado atualizado em v1.41b\n- O CI tem job host com ASan/UBSan; a última execução aprovada antes dos testes de load/store está em https://github.com/MeiKinosaki/CubEX360/actions/runs/38332557518. A revisão com load/store aguarda seu próprio resultado.\n- O núcleo agora tem testes adicionais de LBZ/LHZ/LHA/STB/STH, acessos desalinhados e opcode não suportado, mas continua um interpretador parcial.\n\n## Bloqueios atuais
- Não há toolchain XEX configurada no repositório ou runner.
- O ELF libXenon não é diretamente um executável XEX.
- O núcleo integral do emulador ainda não está integrado ao probe na árvore GitHub atual.
- Não há `default.xex` gerado nem teste no console.

## Próxima ação
Manter a etapa libXenon como validação de compilação de baixo nível e, em paralelo, selecionar uma toolchain XEX real antes de criar um job de empacotamento. O workflow não deve emitir arquivo com extensão .xex se não houver ferramenta validada.


## Atualização v1.48b — gate automatizado
O workflow passou a executar o preflight XEX como gate de CI e exige que ele retorne o código de bloqueio esperado quando `XEX_CC`/`XEX_PACKAGER` não estão configurados. Isso valida que o projeto falha de forma segura; **não** representa 60% de geração de um XEX real. Ainda não foi selecionada uma toolchain/runtime XEX comprovada, nem compilado/validado `default.xex`. Próximo marco: identificar ferramentas reais e compatíveis, compilar um alvo integrado com imports/runtime corretos e validar a estrutura XEX gerada.


## v1.51b — plano de toolchain e executável

### Estado verificado
- O workflow usa Free60/libXenon e valida um ELF32 PowerPC big-endian para smoke test.
- Esse artefato ainda não é um XEX e o probe não representa o emulador completo.
- O script `scripts/xex-preflight.sh` bloqueia a etapa de empacotamento quando não há ferramentas configuradas.
- Não há `default.xex` gerado nem execução comprovada no console.

### Opções em avaliação (não aprovadas para produção)
- **Free60/libXenon:** compilador e bibliotecas úteis para a camada bare-metal; o pipeline atual produz ELF32.
- **RXDK360 `elf2xex.py`:** projeto público de pesquisa que descreve empacotamento ELF→XEX. Antes de adoção, precisamos revisar o código, licenciamento, ABI esperada, stubs/imports, entry point, seções e formato de saída, e reproduzir testes de validação. A documentação do projeto não substitui teste no hardware alvo.
- **XDK/XEX pipeline:** outra possibilidade, mas requer ambiente/toolchain compatível e integração real com o código C atual.

### Critérios para avançar
1. Fixar repositório, commit e licença da ferramenta selecionada.
2. Construir um executável mínimo de teste e validar a estrutura XEX com um verificador independente.
3. Integrar `memory.c`, `bus.c`, `cpu.c` e o runtime necessário num alvo de aplicação, em vez de apenas compilar o smoke ELF.
4. Guardar logs e SHA-256 do artefato, e nunca renomear ELF para `.xex`.
5. Só declarar funcional após teste de inicialização em Xbox 360 RGH.

O avanço da CPU segue em paralelo; passar testes de instruções não significa compatibilidade completa com o Gekko. A versão v1.51b amplia um subconjunto de operações inteiras, enquanto FPU, exceções, MMU, timers/IRQ e integração de sistema ainda estão incompletos.


## Plano de toolchain na nuvem — atualização 2026-10-14

### Pipeline no GitHub-hosted runner
- O workflow `.github/workflows/xenon-build.yml` executa testes nativos, sanitizers e compila o smoke target com Free60/libXenon em container.
- O resultado ELF32 PowerPC big-endian pode ser publicado como artefato do Actions; ele **não** é `default.xex`.

### Caminho para produzir o `default.xex` real
- **RXDK-360 / XDK:** a integração com o Xbox 360 requer uma instalação válida do XDK/licenças aplicáveis e ferramentas compatíveis. Não é seguro presumir que um runner GitHub limpo já tenha esses componentes.
- **ELF→XEX experimental:** um empacotador de pesquisa pode montar um contêiner XEX2, mas empacotar o ELF do Free60/libXenon não prova compatibilidade de ABI, runtime, imports ou inicialização. Não usar isso para rotular o artefato como executável funcional do CubEX360.

Referências técnicas para revisão:
- RXDK-360: https://github.com/Team-Resurgent/RXDK360
- Documentação ELF→XEX: https://github.com/Team-Resurgent/RXDK360/blob/main/docs/xex-packing.md

### Gate de liberação
Não publicar nem renomear um ELF como `default.xex`. O job XEX só deve ser habilitado quando um ambiente de build autorizado tiver compiler/linker/packer compatíveis configurados e o build ligar o núcleo real. A validação deve inspecionar a estrutura XEX, guardar SHA-256 e logs e testar a inicialização no Xbox 360 RGH. Até essas condições serem satisfeitas, o artefato XEX permanece **bloqueado**.
