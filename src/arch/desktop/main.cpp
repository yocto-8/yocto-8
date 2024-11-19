#include <cstdio>

#include <emu/emulator.hpp>
#include <hal/hal.hpp>
#include <p8/parser.hpp>

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Syntax: y8 [game.p8]\n");
		return 1;
	}

	emu::emulator.init();

	const auto success = emu::emulator.load_from_path(argv[1]);
	printf("Loaded with status: %d\n", success);

	if (!success) {
		return 1;
	}

	printf("RUNNING CARTRIDGE\n");

	emu::emulator.run_until_shutdown();

	return 0;
}