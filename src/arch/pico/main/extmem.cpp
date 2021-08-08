#include "pico/stdlib.h"
#include "pico/time.h"
#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include "tinyalloc.h"
#include <extmem/cachedinterface.hpp>
#include <extmem/spiram.hpp>
#include <emu/emulator.hpp>

using namespace arch::pico;

int main() {
    set_sys_clock_khz(250000, true);

    stdio_init_all();

    extmem::spiram::setup();

    std::array<std::uint8_t, 1024> test;

    int successes = 0, total = 0;

    printf("Performing PSRAM test\n");

    for (int i = 0; i < 100; ++i)
    {
        std::size_t bank_addr = (rand() % 8192) * 0x400;

        for (std::size_t i = 0; i < test.size(); ++i)
        {
            test[i] = rand() % 256;
        }

        ++total;
        //printf("Writing page.\n");
        extmem::spiram::write_page(bank_addr, test);

        //printf("Reading page back.\n");
        std::array<std::uint8_t, 1024> test_target;
        extmem::spiram::read_page((rand() % 8192) * 0x400, test_target); // read from random page
        extmem::spiram::read_page(bank_addr, test_target); // and read back from page

        if (test != test_target)
        {
            printf("\n\nFAIL\n");
            for (std::size_t i = 0; i < test_target.size(); ++i)
            {

                if (i%(extmem::spiram::burst_size*2) == 0)
                    printf("\n");
                
                if (test[i] == test_target[i]) printf("-- ");
                else printf("%02x ", test_target[i]);
            }
        }
        else {
            ++successes;
        }
        if (total % 20 == 0) printf("Success rate %d/%d\n", successes, total);
    }

    if (successes != total)
    {
        printf("FAILED PSRAM test: %d/%d\n", successes, total);
        return 1;
    }

    printf("Now trying cortex-M Crimes\n");

    // TODO: test all (register) variants
/*
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
    }*/

    emu::emulator.init(std::span<std::byte, extmem::bank_size>(
        reinterpret_cast<std::byte*>(extmem::bank_base),
        reinterpret_cast<std::byte*>(extmem::bank_base + extmem::bank_size)
    ));

    emu::emulator.load(R"(
for key, value in pairs(_G) do
    print('\t', key, value)
end
)");

    {
        const auto time_start = get_absolute_time();
        for (int i = 0; i < 10'000'000; ++i)
        {
            *(volatile int*)(0x20000000);
        }
        const auto time_end = get_absolute_time();
        printf("native 10e6/%lldµs\n", absolute_time_diff_us(time_start, time_end));
    }

    {
        const auto time_start = get_absolute_time();
        for (int i = 0; i < 10'000'000; ++i)
        {
            *(volatile int*)(0x2F000000);
        }
        const auto time_end = get_absolute_time();
        printf("emulated 10e6/%lldµs\n", absolute_time_diff_us(time_start, time_end));
    }
}