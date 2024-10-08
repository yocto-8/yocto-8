#include <cstdio>
#include <string>
#include <unistd.h>

#include <emu/emulator.hpp>
#include <hal/hal.hpp>
#include <p8/parser.hpp>

static std::array<std::byte, 1024 * 1024 * 8> yolo_heap;

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Syntax: y8vm [game.p8]\n");
		return 1;
	}

	emu::emulator.init(yolo_heap);

	// for now chdir straight up here. later on we need proper vfs stuff
	// this also only handles linux paths
	std::string cart_path = std::string(argv[1]);
	const std::string cart_dir = cart_path.substr(0, cart_path.rfind('/'));
	cart_path = cart_path.substr(cart_dir.size() + 1);

	printf("Setting root directory to '%s'\n", cart_dir.c_str());
	chdir(cart_dir.c_str());

	const auto success = emu::emulator.load_from_path(cart_path);
	printf("Loaded with status: %d\n", success);

	if (!success) {
		return 1;
	}

	printf("RUNNING CARTRIDGE\n");

	emu::emulator.run_until_shutdown();

	return 0;
}