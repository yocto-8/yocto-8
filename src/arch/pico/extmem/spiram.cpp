#include "spiram.hpp"

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <array>
#include <cstdio>

namespace arch::pico::extmem::spiram
{

static spi_inst_t *const psram_spi = spi1;

void setup()
{
    // FIXME: higher freqs seem to be borked
    // possibilities:
    // - dupont cables over 50MHz is not a good idea (emhargged)
    // - fast reads are not actually used and we're running out of spec at high freqs
    // never noticed this before because the freq calculation made it so it ran at 25MHz when running at 250MHz...
    // so it was broken at the default 125MHz and 350MHz somehow
    spi_init(psram_spi, 25 * 1000 * 1000);
    printf("SPI baudrate: %d\n", spi_get_baudrate(psram_spi));
    gpio_set_function(pin_rx, GPIO_FUNC_SPI);
    gpio_set_function(pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(pin_tx, GPIO_FUNC_SPI);

    gpio_init(pin_cs);
    gpio_set_dir(pin_cs, GPIO_OUT);
    select(false);
    sleep_us(150);

    // reset enable
    select(true);
    std::array<std::uint8_t, 1> reset_en_cmd_buf = {
        0x66
    };
    spi_write_blocking(psram_spi, reset_en_cmd_buf.data(), reset_en_cmd_buf.size());
    select(false);

    // reset and wait
    select(true);
    std::array<std::uint8_t, 1> reset_cmd_buf = {
        0x99
    };
    spi_write_blocking(psram_spi, reset_cmd_buf.data(), reset_cmd_buf.size());
    sleep_us(200);
    select(false);
    sleep_us(200);
}


void select(bool chip_selected)
{
    gpio_put(pin_cs, !chip_selected);
}

void validate_address(std::uint32_t page_aligned_address)
{
    assert(page_aligned_address % page_size == 0);
    assert(page_aligned_address < ram_size);
}

void read_page(std::uint32_t page_address, std::span<std::uint8_t, page_size> buf)
{
    validate_address(page_address);

    // 1K page burst read: no need for 32-byte burst reads

    const std::array<std::uint8_t, 4> read_header {
        0x0B, // fast read
        std::uint8_t((page_address >> 16) & 0xFF),
        std::uint8_t((page_address >> 8) & 0xFF),
        std::uint8_t((page_address >> 0) & 0xFF),
    };

    select(true);
    spi_write_blocking(psram_spi, read_header.data(), read_header.size());
        
    std::uint8_t wtf;
    spi_read_blocking(psram_spi, 0, &wtf, 1);

    spi_read_blocking(psram_spi, 0, buf.data(), buf.size());
    select(false);
    //sleep_ms(1);
}

void write_page(std::uint32_t page_address, std::span<const std::uint8_t, page_size> buf)
{
    validate_address(page_address);

    for (std::size_t i = 0; i < page_size / burst_size; ++i)
    {
        select(true);
        std::size_t burst_address = page_address + i * burst_size;

        const std::array<std::uint8_t, 4> write_header {
            0x02,
            std::uint8_t((burst_address >> 16) & 0xFF),
            std::uint8_t((burst_address >> 8) & 0xFF),
            std::uint8_t((burst_address >> 0) & 0xFF),
        };

        spi_write_blocking(spi1, write_header.data(), write_header.size());
        spi_write_blocking(spi1, buf.data() + i * burst_size, burst_size);
        select(false);
        //sleep_ms(1);
    }
}

}