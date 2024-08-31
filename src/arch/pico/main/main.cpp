#include <emu/emulator.hpp>
#include <p8/parser.hpp>

#include <platform/platform.hpp>

namespace pico = arch::pico;

#include "cartridges/closed/fluid.hpp"

int main() {
	pico::platform::init_hardware();

	p8::Parser parser(cartridge);

	while (parser.parse_line())
		;

	emu::emulator.run();
}