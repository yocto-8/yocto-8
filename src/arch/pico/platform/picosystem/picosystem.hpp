#include <io/pushbutton.hpp>
#include <video/st7789.hpp>

#include <array>

namespace arch::pico::platform::picosystem {

struct HardwareState {
	video::ST7789 st7789;
	std::array<io::PushButton, 6> buttons;

	devices::Framebuffer::ClonedArray fb_copy;
};

extern HardwareState hw;

void init_default_frequency();
void init_stdio();
void init_buttons();
void init_emulator();
void init_video_st7789();

} // namespace arch::pico::platform::picosystem