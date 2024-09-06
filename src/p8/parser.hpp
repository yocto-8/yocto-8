#pragma once

#include <string_view>

namespace p8 {

class Parser {
	public:
	// TODO: The parser should instead be able to read from something that can
	// read chunks (probably of a size it cannot control). Cartridges cannot be
	// assumed to be all stored in memory at once. The Lua parser should be able
	// to read using the same chunk reader. The parser here should be able to
	// automatically inject code when `#include` is detected. The Lua parser
	// doesn't really need to be edited to be able to early quit: This could
	// reasonably be handled inside of the chunk reader mechanism.
	Parser(std::string_view source);

	enum class State {
		EXPECT_HEADER,
		EXPECT_VERSION,
		EXPECT_BLOCK,
		PARSING_LUA,
		PARSING_GFX,
		PARSING_LABEL,
		PARSING_GFF,
		PARSING_MAP,
		PARSING_SFX,
		PARSING_MUSIC,
		DONE,

		ERRORS_BEGIN,
		ERROR_UNKNOWN_HEADER = ERRORS_BEGIN
	};

	bool parse_line();

	State get_current_state() const { return _current_state; }

	private:
	void finalize();

	std::string_view _source;
	std::size_t _current_block_offset, _current_offset;
	State _current_state;

	std::string_view _lua_block;
	std::size_t _current_gfx_nibble, _current_tile_nibble, _current_gff_nibble;
};

} // namespace p8