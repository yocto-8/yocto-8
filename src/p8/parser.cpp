#include "parser.hpp"
#include "coredefs.hpp"
#include "hal/hal.hpp"
#include "llex.h"
#include "p8/lookahead.hpp"
#include <devices/image.hpp>
#include <devices/map.hpp>
#include <devices/music.hpp>
#include <devices/spriteflags.hpp>
#include <emu/bufferio.hpp>
#include <emu/emulator.hpp>

#include "lstate.h"
#include <array>

namespace p8::detail {

int hex_digit(char c) {
	switch (c) {
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'a':
	case 'A':
		return 0xA;
	case 'b':
	case 'B':
		return 0xB;
	case 'c':
	case 'C':
		return 0xC;
	case 'd':
	case 'D':
		return 0xD;
	case 'e':
	case 'E':
		return 0xE;
	case 'f':
	case 'F':
		return 0xF;
	}

	return -1;
}

static constexpr std::array<std::pair<std::string_view, ParserState>, 7>
	state_matchers_leading_newline{{
		{"\n__lua__", ParserState::PARSING_LUA},
		{"\n__gfx__", ParserState::PARSING_GFX},
		{"\n__label__", ParserState::PARSING_LABEL},
		{"\n__gff__", ParserState::PARSING_GFF},
		{"\n__map__", ParserState::PARSING_MAP},
		{"\n__sfx__", ParserState::PARSING_SFX},
		{"\n__music__", ParserState::PARSING_MUSIC},
	}};

Parser::Parser(ParserMapConfig map_config, global_State *lua_global_state)
	: _map_config(map_config), _current_state(ParserState::EXPECT_HEADER),
	  _current_gfx_nibble(0),
	  _current_tile_nibble(2 * 128 * 32), // we start on the bottom 32 rows
	  _current_gff_nibble(0), _current_music_pattern(0),
	  _lua_global_state(lua_global_state) {}

#define PARSER_CHECK(cond, err_code)                                           \
	if (!(cond)) {                                                             \
		return (err_code);                                                     \
	}

ParserStatus Parser::consume(hal::ReaderCallback &fs_reader, void *fs_ud) {
	using namespace y8;

	// Clear the target memory region
	if (_map_config.clear_memory) {
		emu::emulator.memory().memset(_map_config.target_region_start, 0,
		                              _map_config.region_size);
	}

	LookaheadReader r(fs_reader, fs_ud);

	const auto try_read_block_header = [&]() -> bool {
		if (_current_state != ParserState::EXPECT_HEADER &&
		    _current_state != ParserState::EXPECT_VERSION) {

			for (const auto &[to_match, matched_state] :
			     state_matchers_leading_newline) {
				// matches without the leading newline?
				if (r.peek_matches(to_match.substr(1))) {
					_current_state = matched_state;
					r.consume_until_next_line();
					return true;
				}
			}
		}

		return false;
	};

	// Parsing loop, per-line.
	while (!r.is_eof()) {
		if (try_read_block_header()) {
			continue;
		}

		// Caller doesn't want to parse this section?
		// Skip to the next line (faster than handling it later).
		if ((_map_config.state_mask & u32(_current_state)) == 0) {
			r.consume_until_next_line();
			continue;
		}

		if (_current_state == ParserState::EXPECT_BLOCK) {
			// failed to match any block, fail
			return ParserStatus::BAD_BLOCK_HEADER;
		}

		switch (_current_state) {
		case ParserState::EXPECT_HEADER: {
			// allow trash or space after this
			PARSER_CHECK(
				r.consume_matches("pico-8 cartridge // http://www.pico-8.com"),
				ParserStatus::BAD_CARTRIDGE_HEADER);
			r.consume_until_next_line();
			_current_state = ParserState::EXPECT_VERSION;
			break;
		}

		case ParserState::EXPECT_VERSION: {
			// allow any version string after this
			PARSER_CHECK(r.consume_matches("version "),
			             ParserStatus::BAD_VERSION_LINE);
			r.consume_until_next_line();
			_current_state = ParserState::EXPECT_BLOCK;
			break;
		}

		case ParserState::PARSING_LUA: {
			LuaBlockReaderState lua_block_state{r, _lua_global_state};
			emu::emulator.load_and_inject_header(lua_block_state.get_reader());
			_current_state = ParserState::EXPECT_BLOCK;
			break;
		}

		case ParserState::PARSING_GFX: {
			auto sprite_sheet = emu::device<devices::Spritesheet>;
			char c;
			while (
				r.consume_if([](char c) { return hex_digit(c) != -1; }, &c)) {
				sprite_sheet.set_nibble(_current_gfx_nibble, hex_digit(c));
				++_current_gfx_nibble;
			}
			// allow garbage/spaces after non-hex
			r.consume_until_next_line();
			break;
		}

		case ParserState::PARSING_MAP: {
			// TODO: deduplicate a bunch of this by reading pairs of nibbles
			// instead, add some abstraction for it that can read malformed
			// files
			// should apply to the gfx, map, gff, sfx, music blocks
			auto map = emu::device<devices::Map>;
			char c;
			while (
				r.consume_if([](char c) { return hex_digit(c) != -1; }, &c)) {
				if (_current_tile_nibble % 2 == 0) {
					map.data[_current_tile_nibble / 2] |= hex_digit(c) << 4;
				} else {
					map.data[_current_tile_nibble / 2] |= hex_digit(c);
				}
				++_current_tile_nibble;
			}
			// allow garbage/spaces after non-hex
			r.consume_until_next_line();
			break;
		}

		case ParserState::PARSING_GFF: {
			auto sprite_flags = emu::device<devices::SpriteFlags>;
			char c;
			while (
				r.consume_if([](char c) { return hex_digit(c) != -1; }, &c)) {
				if (_current_gff_nibble % 2 == 0) {
					sprite_flags.flags_for(_current_gff_nibble / 2) |=
						hex_digit(c) << 4;
				} else {
					sprite_flags.flags_for(_current_gff_nibble / 2) |=
						hex_digit(c);
				}
				++_current_gff_nibble;
			}
			r.consume_until_next_line();
			break;
		}

		case ParserState::PARSING_SFX: {
			r.consume_until_next_line();
			break;
		}

		case ParserState::PARSING_MUSIC: {
			auto music = emu::device<devices::Music>;

			if (r.consume_empty_line()) {
				break;
			}

			// consider `0c 727c7f7f`

			// in the example, parse `0c` into `flag`
			int flag_high = hex_digit(r.consume_char());
			int flag_low = hex_digit(r.consume_char());
			if (flag_low == -1 || flag_high == -1) {
				return ParserStatus::BAD_MUSIC_FLAG;
			}
			u8 flag = (flag_high << 4) | flag_low;

			// in the example, consume the space
			if (r.consume_char() != ' ') {
				return ParserStatus::BAD_MUSIC_FLAG;
			}

			// in the example, consume all four bytes as a pattern description
			for (int i = 0; i < 4; ++i) {
				int pattern_high = hex_digit(r.consume_char());
				int pattern_low = hex_digit(r.consume_char());
				if (pattern_low == -1 || pattern_high == -1) {
					return ParserStatus::BAD_MUSIC_PATTERN;
				}

				u8 pattern = (pattern_high << 4) | pattern_low;

				// `flag` contains four flags at the least significant bits
				// each flag is mapped to one pattern, at the MSB.
				// i.e. flag 0 goes to patterns[0], etc.
				pattern |= ((flag >> i) & 0b1) << 7;

				// store in memory, with the proper memory format
				music.get(_current_music_pattern * 4 + i) = pattern;
			}

			r.consume_until_next_line();
			break;
		}

		default:
			r.consume_until_next_line();
			break;
		}
	}

	return ParserStatus::OK;
}

const char *LuaBlockReaderState::reader_callback(void *ud, std::size_t *size) {
	LuaBlockReaderState &state = *static_cast<LuaBlockReaderState *>(ud);
	LookaheadReader &r = *state._reader;

	release_assert(state._lua_global_state != nullptr);

	LexState *lex_state = state._lua_global_state->y8_active_lexer;

	// Are we parsing an include?
	if (state._is_including) {
		// Try to continue reading from it
		const char *buf = hal::fs_read_buffer(&state._include_reader, size);

		if (buf != nullptr) {
			return buf;
		} else {
			// Stop trying to read from include if it finished reading
			hal::fs_destroy_open_context(state._include_reader);
			state._is_including = false;
			*size = 0;

			// Restore old source name (+ linenumber just after)
			lex_state->source = state._main_file_name;

			// Fallthrough to the usual handling
		}
	}

	lex_state->linenumber = state._main_line_number;
	state._main_file_name = lex_state->source;

	// Last buffer read stopped at a newline?
	if (r.peek_char() == '\n') {
		// Can we match a new cartridge block?
		for (const auto &[to_match, matched_state] :
		     state_matchers_leading_newline) {
			if (r.peek_matches(to_match)) {
				// New block? return to parent
				r.consume_char(); // Eat the newline
				*size = 0;
				state._is_main_eof = true;
				return nullptr;
			}
		}

		// Can we match a `#include` block?
		if (r.peek_matches("\n#include")) {
			r.consume_matches("\n#include");

			++state._main_line_number;

			// Consume spaces
			while (r.consume_if([](char c) { return c == ' ' || c == '\t'; }))
				;

			// Parse filename into a buffer
			std::array<char, 128> include_fname;
			const auto out_buf = r.consume_into(
				[](char c) { return c != ' ' && c != '\n' && c != '\r'; },
				include_fname);
			const std::string_view out{out_buf.begin(), out_buf.end()};

			// Consume the `#include` line, but **keep** the newline.
			//
			// This is a dumb trick to ensure we are jumping a line after the
			// include, even if the included file doesn't have any final
			// newline character.
			while (r.consume_if([](char c) { return c == ' '; }))
				;

			printf("Including file \"%.*s\"\n", int(out.size()), out.data());

			lex_state->source =
				luaX_newstring(lex_state, out.data(), out.size());
			lex_state->linenumber = 0; // +1 from the newline after

			if (hal::fs_create_open_context(out, state._include_reader) ==
			    hal::FileOpenStatus::SUCCESS) {
				// Start reading from the included file.
				state._is_including = true;

				// Inject a newline at the start of the include, otherwise it
				// may get merged with the previous line.
				// We will actually be reading from the buffer on the next call
				// to this callback.
				static constexpr std::array<char, 1> _newline_buf = {'\n'};
				*size = 1;
				return _newline_buf.data();
			} else {
				// TODO: error handling
				printf("Include failed!\n");
				release_abort("Failed to open include");
			}
		}
	}

	// Read this line for Lua
	int chars_read = 0;
	++state._main_line_number;
	const char *buf = r.consume_rest_of_current_buffer_while(
		[&chars_read](char c) {
			// don't return if the first char is '\n'
			return (chars_read++ == 0 || c != '\n') && c != '\0';
		},
		size);

	return buf;
}

void Parser::optimize_map_config() {
	// TODO: if some regions are known to be OOB then skip them
}
} // namespace p8::detail