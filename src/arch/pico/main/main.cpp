#include <emu/emulator.hpp>
#include <p8/parser.hpp>

#include <platform/platform.hpp>

namespace pico = arch::pico;

#include "cartridges/closed/fluid.hpp"
// #include "cartridges/rgbcal.hpp"

int main() {
	pico::platform::init_hardware();

	emu::StringReader reader{cartridge};
	// TODO: err handling
	p8::parse(emu::StringReader::reader_callback, &reader);

	emu::emulator.run();
}