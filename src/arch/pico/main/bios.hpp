#include <string_view>

std::string_view bios_cartridge =
	R"(pico-8 cartridge // http://www.pico-8.com
version 32
__lua__
print("Y8 bios", 8, 8); flip(); function _update60() end --load("/flash/y8/shell.p8"))";