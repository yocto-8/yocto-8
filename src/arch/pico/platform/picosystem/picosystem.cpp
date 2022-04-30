#include "class/cdc/cdc_device.h"
#include <platform/picosystem/picosystem.hpp>

#include <cstdio>
#include <pico/stdlib.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>

#include <emu/emulator.hpp>
#include <hardwarestate.hpp>

namespace arch::pico::platform::picosystem
{

HardwareState hw;

void init_default_frequency()
{
    // Stock PicoSystem SDK uses 250MHz@1.20V -- let's comply with that
    // But note that on some RP2040s you can do 300MHz@1.25V.
    // This may, however, be influenced by PCB design, so practical limits may be different
    // between the Pico and the PicoSystem.
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    set_sys_clock_khz(250000, false);

    // SPI reference clock configuration
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    250 * MHZ,
                    250 * MHZ);
}

void init_stdio()
{
    stdio_init_all();

    while (!tud_cdc_connected()) {
      printf(".");
      sleep_ms(500);
    }

    printf("yop ready\n");
    sleep_ms(1000);
    printf("yop ready2\n");
}

void init_buttons()
{
    hw.buttons[0].init(22);
    hw.buttons[1].init(21);
    hw.buttons[2].init(23);
    hw.buttons[3].init(20);
    hw.buttons[4].init(19);
    hw.buttons[5].init(18);
}

void init_emulator()
{
    emu::emulator.init(std::span<std::byte, 0>());
}

void init_video_st7789()
{
    spi_inst_t* video_spi = spi0;
    spi_init(video_spi, 62'500'000);

    printf("ST7789 baudrate: %d\n", spi_get_baudrate(video_spi));

    picosystem::hw.st7789.init({
        .spi = video_spi,
        .pinout = { .sclk = 6, .tx = 7, .cs = 5, .vsync = 8, .rst = 4, .dc = 9, .backlight = 12 }
    });
}

}