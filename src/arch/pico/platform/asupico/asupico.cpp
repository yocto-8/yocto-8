#include <platform/asupico/asupico.hpp>

#include <pico/stdlib.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>

#include <extmem/cachedinterface.hpp>
#include <hardwarestate.hpp>

namespace arch::pico::platform::asupico
{

HardwareState hw;

void init_default_frequency()
{
    // NOTE: Not all following settings may work on any board.
    // In any case, those settings run the RP2040 out-of-spec and increases power draw.
    // Most settings beyond 250MHz will fail if you do not edit the flash SPI frequency multiplier to 4 (default 2).

    // The bootup settings are 125MHz@1.10V.

    /*vreg_set_voltage(VREG_VOLTAGE_1_10);
    set_sys_clock_khz(250000, false);*/
    
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    set_sys_clock_khz(300000, false);

    /*vreg_set_voltage(VREG_VOLTAGE_1_25);
    set_sys_clock_khz(351000, false);*/

    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    300 * MHZ,
                    300 * MHZ);
}

void init_stdio()
{
    stdio_init_all();
}

void init_buttons()
{
    hw.buttons[0].init(16);
    hw.buttons[1].init(18);
    hw.buttons[2].init(17);
    hw.buttons[3].init(19);
    hw.buttons[4].init(20);
    hw.buttons[5].init(21);
}

/*
#include <extmem/spiram.hpp>

[[gnu::noinline]]
void test_ram()
{
    using namespace arch::pico;
    std::array<std::uint8_t, 1024> test;
    int successes = 0, total = 0;

    printf("Performing PSRAM test\n");

    printf("OP validation\n");

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
        register uint32_t a asm("r0"), b asm("r2"), c asm("r4"), addr asm("r6");
        a = 123456;
        b = 654321;
        c = 696969;
        addr = 0x2F000400;
        asm volatile(
            "stm %[addr], {%[a], %[b], %[c]}"
            : [addr]"+r"(addr)
            : [a]"r"(a), [b]"r"(b), [c]"r"(c)
            : "memory"
        );
    }

    // LDM(imm)
    {
        register uint32_t a asm("r1"), b asm("r3"), c asm("r5"), addr asm("r7");

        addr = 0x2F000400;

        asm(
            "ldm %[addr]!, {%[a], %[b], %[c]}"
            : [a]"=r"(a), [b]"=r"(b), [c]"=r"(c)
            : [addr]"r"(addr)
        );
        printf("LDM(imm): %d %d %d (expect 123456, 654321, 696969)\n", a, b, c);
    }

    printf("Bank swapping test\n");

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
            printf("\n\nFAIL %d\n", int(bank_addr));
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
    }
}
*/

void init_spi_ram()
{
    pico::extmem::spiram::setup();
    //test_ram();
}

void init_emulator()
{
    emu::emulator.init(std::span<std::byte, pico::extmem::bank_size>(
        reinterpret_cast<std::byte*>(pico::extmem::bank_base),
        reinterpret_cast<std::byte*>(pico::extmem::bank_base + pico::extmem::bank_size)
    ));
}

void init_video_ssd1351()
{
    spi_inst_t* video_spi = spi0;
    spi_init(video_spi, 25 * 1000 * 1000);

    printf("SSD1351 baudrate: %d\n", spi_get_baudrate(video_spi));

    asupico::hw.ssd1351.init({
        .spi = video_spi,
        .pinout = { .sclk = 2, .tx = 3, .rst = 4, .cs = 5, .dc = 6 }
    });
}

}