# CubEX360 — plano de toolchain em nuvem

Este documento separa a compilação remota já existente da futura geração de um executável XEX real.

## Pipeline existente
Workflow: .github/workflows/xenon-build.yml

- Testes nativos do núcleo e sanitizers no GitHub-hosted runner.
- Build do alvo smoke usando Free60/libXenon em container.
- Verificação de ELF32 PowerPC big-endian e upload dos artefatos ELF/relatórios.

Esse alvo é um ELF de smoke test. Não é `default.xex` e não deve ser renomeado para `.xex`.

## Caminho para XEX
RXDK-360 documenta uma toolchain de desenvolvimento para Xbox 360 baseada em Windows e Visual Studio, que reutiliza um XDK fornecido pelo usuário. Por isso, não presumimos que um runner Ubuntu hospedado pelo GitHub tenha as ferramentas XEX necessárias.

Referências públicas para avaliação:
- RXDK-360: https://github.com/Team-Resurgent/RXDK-360
- XexTool: https://github.com/Team-Resurgent/XexTool
- Documentação de empacotamento: https://github.com/Team-Resurgent/RXDK-360/blob/main/docs/xex-packing.md

## Etapas antes de habilitar o build XEX
1. Disponibilizar um runner Windows dedicado e autorizado com as ferramentas e SDKs apropriados.
2. Fixar versões/commits das ferramentas e documentar a configuração do runner sem armazenar licenças ou segredos no repositório.
3. Criar um alvo de aplicação que ligue o núcleo CubEX360 real e use ABI, entry point, runtime e imports compatíveis.
4. Gerar um XEX de teste e inspecionar estrutura e imports com ferramentas independentes.
5. Guardar logs e SHA-256 como artefatos do workflow.
6. Validar inicialização no hardware Xbox 360 RGH antes de chamar o artefato de funcional.

## Gate de segurança
O job de XEX deve falhar de forma explícita se as ferramentas não estiverem configuradas ou se o artefato não passar nas verificações. Não publicar um arquivo vazio, não renomear ELF e não apresentar um probe como emulador completo.

## Estado atual
- Toolchain remota Free60/libXenon: configurada para smoke build.
- Integração de runtime/ABI XEX: pendente.
- `default.xex` do CubEX360 completo: não gerado.
- Teste no Xbox 360 RGH: não confirmado.
## Workflow XEX dedicado (v1.57b — infraestrutura)
Foi adicionado `.github/workflows/xex-build.yml`, acionado manualmente, para um runner Windows self-hosted rotulado `XBOX360-XDK`. O runner precisa ser provisionado e autorizado pelo mantenedor; não há XDK ou segredo/licença incluído no repositório.

Configuração exigida no runner:
- `CUBEX_XEX_BUILD_SCRIPT`: caminho para o script local que executa a compilação real com a toolchain instalada.
- `CUBEX_XEX_OUTPUT`: caminho para o XEX candidato produzido por esse script.

O workflow falha se a configuração não existir, se o build falhar, ou se o arquivo não tiver pelo menos 4096 bytes e magic `XEX2`. O validador calcula SHA-256 e o artefato é publicado com retenção curta para inspeção. Esse gate confirma somente estrutura mínima do contêiner; não confirma imports, ABI, compatibilidade de runtime ou boot no RGH.

**Estado:** workflow e validação mínima versionados; runner Windows e script de build real ainda precisam ser configurados. Nenhum `default.xex` funcional foi produzido por esta alteração.