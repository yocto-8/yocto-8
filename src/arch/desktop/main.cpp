#include <cstdio>

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

	printf("Loading game from '%s'\n", argv[1]);

	hal::FileReaderContext cart;
	if (hal::fs_create_open_context(argv[1], cart) !=
	    hal::FileOpenStatus::SUCCESS) {
		printf("Failed to load cartridge from %s\n", argv[1]);
		return 1;
	}

	const auto parse_err = p8::parse(hal::fs_read_buffer, &cart);

	switch (parse_err) {
	case p8::ParserStatus::OK:
		break;

	default:
		printf("Parsing error %d!\n", int(parse_err));
		break;
	}

	printf("RUNNING CARTRIDGE\n");

	emu::emulator.run();
}