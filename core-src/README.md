# CubEX360 core smoke target

Este alvo valida a cadeia libXenon com uma máquina integrada mínima: memória principal simulada, barramento, interpretador PowerPC parcial e coordenador de execução. O smoke program executa ADDI e verifica estado de CPU e contador de instruções. A CPU contém outras instruções em desenvolvimento, mas isso não significa cobertura completa do Gekko.

## Integração host

A API `cubex360/machine.h` coordena:
- memória principal e CPU;
- avanço determinístico do timer em um tick por instrução aposentada;
- cópia DMA RAM–RAM com limites;
- banco MMIO genérico de 32 bits.

O teste `test_machine_integration.c` executa um pequeno programa, verifica a expiração do timer, faz leitura/escrita MMIO e copia dados entre aliases cached/uncached via DMA.

**Limites importantes:** o timer ainda não representa um dispositivo GameCube real; o DMA não implementa canais do controlador real; o MMIO não é o mapa de registradores do Flipper; a memória não implementa MMU/TLB nem coerência de cache. O avanço de um tick por instrução é uma política de integração provisória, não temporização ciclo-exata.

## Build no container Free60/libXenon

```sh
export DEVKITXENON=/usr/local/xenon
export PATH="$DEVKITXENON/bin:$DEVKITXENON/usr/bin:$PATH"
make clean && make
```

O resultado esperado é `cubex360_core_smoke.elf32`. Isso ainda não é `default.xex`, não prova boot no Xbox 360 RGH e não demonstra compatibilidade com jogos comerciais. A CI precisa validar separadamente o teste host integrado, sanitizers e smoke build.
