#include <cstdio>
#include <fstream>
#include <string>

#include <emu/emulator.hpp>
#include <p8/parser.hpp>

std::string string_from_file(std::ifstream &file) {
	file.seekg(0, std::ios_base::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios_base::beg);

	std::string ret(size, '\0');
	file.read(ret.data(), ret.size());

	return ret;
}

static std::array<std::byte, 1024 * 1024 * 8> yolo_heap;

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Syntax: y8vm [game.p8]\n");
		return 1;
	}

	emu::emulator.init(yolo_heap);

	printf("Loading game from '%s'\n", argv[1]);

	std::ifstream cartridge(argv[1]);

	if (!cartridge) {
		printf("Failed to open cartridge\n");
		return 1;
	}

	cartridge.exceptions(std::ios_base::failbit);

	const std::string source = string_from_file(cartridge);

	// FIXME: fs reader outright
	emu::StringReader reader{source};
	const auto parse_err =
		p8::parse(emu::StringReader::reader_callback, &reader);

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