#include <stdio.h>
#include <xenos/xenos.h>
#include <console/console.h>

/* Toolchain smoke test only; this is not the CubEX360 emulator. */
int main(void)
{
    xenos_init(VIDEO_MODE_AUTO);
    console_init();
    printf("CubEX360 toolchain probe\n");
    printf("libXenon startup and console output: OK\n");
    printf("This ELF32 validates the build path only; emulation is not integrated.\n");
    for (;;) {
        /* Keep the output visible when launched from XeLL. */
    }
    return 0;
}
