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

## Bloqueios atuais
- Não há toolchain XEX configurada no repositório ou runner.
- O ELF libXenon não é diretamente um executável XEX.
- O núcleo integral do emulador ainda não está integrado ao probe na árvore GitHub atual.
- Não há `default.xex` gerado nem teste no console.

## Próxima ação
Manter a etapa libXenon como validação de compilação de baixo nível e, em paralelo, selecionar uma toolchain XEX real antes de criar um job de empacotamento. O workflow não deve emitir arquivo com extensão .xex se não houver ferramenta validada.
