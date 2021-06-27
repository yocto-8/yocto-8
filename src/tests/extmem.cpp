#include "pico/stdlib.h"
#include "pico/time.h"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstdio>

int main() {
    set_sys_clock_khz(250000, true);

    stdio_init_all();

    printf("Now trying cortex-M Crimes\n");
    
    /*const auto time_start = get_absolute_time();
    for (int i = 0; i < 10'000'000; ++i)
    {
        *(volatile int*)(0x2F000000+i*4);
    }
    const auto time_end = get_absolute_time();
    printf("10e6/%lldµs\n", absolute_time_diff_us(time_start, time_end));*/

    // TODO: test all (register) variants

    // STR(imm)
    asm(
        "str %[value], [%[addr], #8]"
        :: [value]"r"(123456), [addr]"r"(0x2F000100-8)
    );

    // LDR(imm)
    {
        std::uint32_t out;
        asm(
            "ldr %[out], [%[addr], #16]"
            : [out]"=r"(out)
            : [addr]"r"(0x2F000100-16)
        );
        printf("LDR(imm): %d (expected 123456)\n", out);
    }

    // STRB(imm)
    asm(
        "strb %[value], [%[addr], #3]"
        :: [value]"r"(128), [addr]"r"(0x2F000200-3)
    );

    // LDRB(imm)
    {
        std::uint8_t out;
        asm(
            "ldrb %[out], [%[addr], #1]"
            : [out]"=r"(out)
            : [addr]"r"(0x2F000200-1)
        );
        printf("LDRB(imm): %d (expected 128)\n", out);
    }

    // STRH(imm)
    asm(
        "strh %[value], [%[addr], #2]"
        :: [value]"r"(5000), [addr]"r"(0x2F000300-2)
    );

    // LDRH(imm)
    {
        std::uint16_t out;
        asm(
            "ldrh %[out], [%[addr], #4]"
            : [out]"=r"(out)
            : [addr]"r"(0x2F000300-4)
        );
        printf("LDRH(imm): %d (expected 5000)\n", out);
    }

    // STM
    {
        register uint32_t a asm("r4"), b asm("r5"), c asm("r6"), addr asm("r7");
        a = 123456;
        b = 654321;
        c = 696969;
        addr = 0x2F000400;
        asm(
            "stm %[addr]!, {%[a], %[b], %[c]}"
            :: [addr]"r"(addr), [a]"r"(a), [b]"r"(b), [c]"r"(c)
        );
    }

    // LDM(imm)
    {
        register uint32_t a asm("r0"), b asm("r1"), c asm("r2"), addr asm("r3");

        addr = 0x2F000400;

        asm(
            "ldm %[addr]!, {%[a], %[b], %[c]}"
            : [a]"=r"(a), [b]"=r"(b), [c]"=r"(c)
            : [addr]"r"(addr)
        );
        printf("LDM(imm): %d %d %d (expect 123456, 654321, 696969)\n", a, b, c);
    }

    malloc(128);
}