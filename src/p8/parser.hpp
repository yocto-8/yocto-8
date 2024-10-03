#pragma once

#include "hal/types.hpp"
#include <emu/bufferio.hpp>
#include <hal/hal.hpp>
#include <p8/lookahead.hpp>

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

namespace detail {

class Parser {
	public:
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

	Parser();

	ParserStatus consume(hal::ReaderCallback &fs_reader, void *fs_ud);
	State get_current_state() const { return _current_state; }

	private:
	State _current_state;

	std::size_t _current_gfx_nibble, _current_tile_nibble, _current_gff_nibble;
};

class LuaBlockReaderState {
	public:
	LuaBlockReaderState(LookaheadReader &reader)
		: _reader(&reader), _is_main_eof(false), _is_including(false) {}

	static const char *reader_callback(void *ud, std::size_t *size);
	emu::Reader get_reader() { return {reader_callback, this}; }

	private:
	LookaheadReader *_reader;
	bool _is_main_eof;

	hal::FileReaderContext _include_reader;
	bool _is_including;
};

} // namespace detail

inline ParserStatus parse(hal::ReaderCallback &reader, void *ud) {
	return detail::Parser{}.consume(reader, ud);
}

} // namespace p8