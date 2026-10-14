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
