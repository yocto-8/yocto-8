#include <emu/emulator.hpp>
#include <p8/parser.hpp>

#include <platform/platform.hpp>

namespace pico = arch::pico;

int main() {
	pico::platform::init_hardware();

	// hardcoded to `bios_cartridge`
	emu::emulator.load_from_path("/y8/bios.p8");
	emu::emulator.run();
}