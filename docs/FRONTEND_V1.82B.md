# CubEX360 v1.82b — configurações e biblioteca

## Arquivos adicionados

- `frontend/settings.json`: perfis globais Quality, Balanced e Performance; parâmetros futuros de vídeo/áudio; espaço para overrides por jogo.
- `frontend/filters.json`: filtros planejados por favoritos, recentes, capa, compatibilidade e gênero, além de pesquisa e ordenação.
- `frontend/per_game_profile.schema.json`: formato-base para personalização por Game ID.
- `assets/boot/boot_sequence.json`: sequência de abertura original com fade para a biblioteca e áudio opcional escolhido pelo usuário.

## Estado de implementação

Os arquivos definem a configuração, não um menu gráfico funcional. O frontend ainda precisa ler/escrever essas opções, conectar filtros à biblioteca e aplicar somente as opções que o renderer/audio backend realmente suportar. Resolução interna, antialiasing, VSync, frameskip e filtragem não devem ser apresentados como funcionais sem implementação e medição.

A abertura ainda precisa de renderização no runtime e integração de áudio. Não há tema oficial do GameCube distribuído no projeto; use áudio original ou fornecido pelo usuário com os direitos apropriados.

## Outros subsistemas

O save/load atual cobre CPU + RAM principal, não todo o hardware. A API de capas é uma estrutura com callback de download: a pilha de rede real do Xbox 360 e o servidor de capas precisam ser integrados. ELF32 de teste não equivale a `default.xex`; nenhum boot funcional no RGH foi comprovado.
