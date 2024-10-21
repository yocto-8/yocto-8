#pragma once

#include "coredefs.hpp"
#include "hal/types.hpp"
#include <emu/bufferio.hpp>
#include <hal/hal.hpp>
#include <p8/lookahead.hpp>

struct global_State; // lua global state forward declaration
union TString;

// The entirety of the parsing and buffered reading logic here is an incredible
// pile of depressing hacks.
//
// The gist of it is that we give Lua a reader it can freely read from, and we
// reuse the same function spec as Lua to allow the hardware abstraction layer
// to give us chunks of files of an arbitrary size.
//
// This ultimately has us make layers of cursed things to do things like limit
// the Lua buffered reader from reading beyond the relevant `__lua__` block in
// the cartridge, or things like inject headers to the cart (for which there's
// maybe a better way??), while requiring no dynamic memory to be allocated
// other than some small buffers.

namespace p8 {

enum class ParserStatus {
	/// No error has occurred
	OK,

	/// Failed to match first line of cartridge
	BAD_CARTRIDGE_HEADER,

	/// Failed to match version line
	BAD_VERSION_LINE,

	/// Failed to match a block when one was expected
	BAD_BLOCK_HEADER,
};

enum class ParserState {
	EXPECT_HEADER = 1 << 0,
	EXPECT_VERSION = 1 << 1,
	EXPECT_BLOCK = 1 << 2,
	PARSING_LUA = 1 << 3,
	PARSING_GFX = 1 << 4,
	PARSING_LABEL = 1 << 5,
	PARSING_GFF = 1 << 6,
	PARSING_MAP = 1 << 7,
	PARSING_SFX = 1 << 8,
	PARSING_MUSIC = 1 << 9,
	DONE = 1 << 10,
};

struct ParserMapConfig {
	y8::PicoAddr target_region_start = 0x0000;
	y8::PicoAddr source_region_start = 0x0000;
	y8::u32 region_size = 0x10000;
	y8::u32 state_mask = 0xFFFFFFFF;
	bool clear_memory = false;

	ParserMapConfig &disable_state(ParserState to_disable) {
		state_mask &= ~y8::u32(to_disable);
		return *this;
	}
};

namespace detail {

class Parser {
	public:
	Parser(ParserMapConfig map_config = {},
	       global_State *lua_global_state = nullptr);

	ParserStatus consume(hal::ReaderCallback &fs_reader, void *fs_ud);
	ParserState get_current_state() const { return _current_state; }

	private:
	void optimize_map_config();

	ParserMapConfig _map_config;

	ParserState _current_state;

	std::size_t _current_gfx_nibble, _current_tile_nibble, _current_gff_nibble;

	global_State *_lua_global_state;
};

class LuaBlockReaderState {
	public:
	LuaBlockReaderState(LookaheadReader &reader, global_State *lua_global_state)
		: _reader(&reader), _is_main_eof(false), _main_line_number(1),
		  _main_file_name(nullptr), _is_including(false),
		  _lua_global_state(lua_global_state) {}

	static const char *reader_callback(void *ud, std::size_t *size);
	emu::Reader get_reader() { return {reader_callback, this}; }

	private:
	LookaheadReader *_reader;
	bool _is_main_eof;

	/// Current line number in the main file (not inside of includes)
	int _main_line_number;
	/// Current main file name
	TString *_main_file_name;

	hal::FileReaderContext _include_reader;
	bool _is_including;

	global_State *_lua_global_state;
};

} // namespace detail

inline ParserStatus parse(hal::ReaderCallback &reader, void *ud,
                          ParserMapConfig map_config = {},
                          global_State *lua_global_state = nullptr) {
	return detail::Parser{map_config, lua_global_state}.consume(reader, ud);
}

} // namespace p8