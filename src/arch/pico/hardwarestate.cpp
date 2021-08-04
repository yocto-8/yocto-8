#include "hardwarestate.hpp"

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/vreg.h>

#include <emu/emulator.hpp>
#include <extmem/cachedinterface.hpp>
#include <cmdthread.hpp>

namespace arch::pico
{

void initialize_default_frequency()
{
    // NOTE: Not all following settings may work on any board.
    // In any case, those settings run the RP2040 out-of-spec and increases power draw.
    // Most settings beyond 250MHz will fail if you do not edit the flash SPI frequency multiplier to 4 (default 2).

    // The bootup settings are 120MHz@1.10V.

    /*vreg_set_voltage(VREG_VOLTAGE_0_90);
    set_sys_clock_khz(120000, false);*/

    vreg_set_voltage(VREG_VOLTAGE_1_10);
    set_sys_clock_khz(250000, false);
    
    /*vreg_set_voltage(VREG_VOLTAGE_1_15);
    set_sys_clock_khz(300000, false);*/

    /*vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(400000, false);*/
}

void initialize_stdio()
{
    stdio_init_all();
}

void initialize_buttons()
{
    hw.buttons[0].init(16);
    hw.buttons[1].init(18);
    hw.buttons[2].init(17);
    hw.buttons[3].init(19);
    hw.buttons[4].init(20);
    hw.buttons[5].init(21);
}

#include <extmem/spiram.hpp>

void test_ram()
{
    using namespace arch::pico;
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

void initialize_spi_ram()
{
    pico::extmem::spiram::setup();
    test_ram();
}

void initialize_emulator()
{
    emu::emulator.init(gsl::span<char, pico::extmem::bank_size>(
        reinterpret_cast<char*>(pico::extmem::bank_base),
        reinterpret_cast<char*>(pico::extmem::bank_base + pico::extmem::bank_size)
    ));
}

void initialize_ssd1351(spi_inst_t* spi_instance)
{
    hw.ssd1351.init({
        .spi = spi_instance,
        .pinout = {.sclk = 2, .tx = 3, .rst = 4, .cs = 5, .dc = 6 }
    });
}

void initialize_cmd_thread()
{
    multicore_launch_core1(core1_entry);
}

void initialize_hardware()
{
    initialize_default_frequency();

    initialize_stdio();

    initialize_buttons();

    initialize_spi_ram();

    // The emulator depends on stdio and external RAM support (as it may require initializing the heap)
    // Some other things may depend on the emulator later on, so perform it as early as possible
    initialize_emulator();

    spi_inst_t& video_spi = *spi0;
    spi_init(&video_spi, 25 * 1000 * 1000);
    initialize_ssd1351(&video_spi);

    initialize_cmd_thread();
}

HardwareState hw;

}