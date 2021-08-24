#include <hal/hal.hpp>

#include <sys/time.h>


#include <unistd.h>
#include <emu/emulator.hpp>
#include "../window.hpp"
#include "devices/image.hpp"

namespace hal
{

std::uint16_t update_button_state()
{
    return 0;
}

void present_frame()
{
    arch::desktop::yolo_window.present_frame(emu::device<devices::Framebuffer>);
}

static std::uint64_t timer_start_micros;

void reset_timer()
{
    timer_start_micros = measure_time_us();
}

std::uint64_t measure_time_us()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (1000000 * std::uint64_t(tv.tv_sec) + std::uint64_t(tv.tv_usec)) - timer_start_micros;
}

void delay_time_us(std::uint64_t time)
{
    usleep(time);
}

void load_rgb_palette(std::span<std::uint32_t, 32> new_palette)
{
    printf("RGB palette update unimplemented on desktop\n");
}

}