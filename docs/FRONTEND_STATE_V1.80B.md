# CubEX360 v1.80b — estado rápido e estrutura de frontend

## Implementado nesta etapa

- API `cubex_state_save` / `cubex_state_load` para snapshot versionado de registradores Gekko + RAM principal de 24 MiB.
- Cabeçalho com versão/tamanho, serialização big-endian e CRC32 para rejeitar arquivos truncados ou corrompidos.
- Salvamento em arquivo temporário antes de renomear o resultado, reduzindo o risco de substituir um estado válido por um arquivo incompleto.
- Teste host de ida e volta: salva CPU/RAM, altera os valores, carrega e verifica a restauração.
- Workflow de CI atualizado para compilar `state.c` nos jobs host e sanitizer.
- Configuração inicial da biblioteca em `frontend/library.json`, além dos diretórios documentados para ROMs, capas, save states e assets de abertura.

## Limites do save state

Este é um snapshot parcial de CPU + RAM. Não salva estado de GPU/Flipper, DSP, timers, drive/disco, controladores, interrupções de dispositivos ou outros componentes porque esses subsistemas ainda não existem de forma integrada. A API é experimental e não garante compatibilidade futura entre versões. A implementação precisa passar pelo CI antes de ser considerada validada.

## Frontend estilo Aurora

A estrutura define:
- `games/roms/`: diretório pretendido para jogos do usuário;
- `games/covers/`: capas associadas a IDs de jogo;
- `frontend/library.json`: lista de jogos e favoritos;
- `saves/states/`: arquivos de snapshot;
- `assets/boot/`: futura tela de abertura e configuração de áudio.

Ainda não há navegador de arquivos, parser de imagens de disco, grade gráfica de capas, controles de favoritos nem integração dessas pastas ao runtime do Xbox 360. A interface Aurora é uma especificação de trabalho, não uma UI já funcional.

## Abertura e música

Planeja-se uma tela de abertura original CubEX360 antes do menu. O repositório não inclui a música tema oficial do GameCube nem o vídeo original; o futuro player poderá aceitar um arquivo de áudio fornecido pelo usuário e usado legalmente. O runtime atual ainda não implementa reprodução de áudio nem splash screen no console.

## Toolchain e artefatos

O workflow Free60/libXenon existente produz um ELF32 de smoke test, não o emulador completo. `default.xex` continua bloqueado pela necessidade de runtime/ABI e toolchain XEX apropriados. Não renomear ELF para XEX. O boot no Xbox 360 RGH não foi comprovado.
