# CubEX360 core smoke target

Este diretório é um alvo de integração inicial, deliberadamente pequeno, para validar a cadeia libXenon com memória GameCube simulada e um interpretador PowerPC limitado. Ele executa somente ADDI, ADDIS, ORI, LWZ, STW e B incondicional.

Build no container Free60/libXenon:

```sh
export DEVKITXENON=/usr/local/xenon
export PATH="$DEVKITXENON/bin:$DEVKITXENON/usr/bin:$PATH"
make clean && make
```

O resultado esperado é `cubex360_core_smoke.elf32`. Isso ainda não é o emulador completo nem um XEX. O alvo existe para validar a integração da memória, barramento e CPU mínima no compilador PowerPC.
