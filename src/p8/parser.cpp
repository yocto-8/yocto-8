#include "parser.hpp"
#include "p8/lookahead.hpp"

#include <devices/image.hpp>
#include <devices/map.hpp>
#include <devices/spriteflags.hpp>
#include <emu/bufferio.hpp>
#include <emu/emulator.hpp>

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

const std::array<std::pair<std::string_view, Parser::State>, 7>
	state_matchers_leading_newline{{
		{"\n__lua__", Parser::State::PARSING_LUA},
		{"\n__gfx__", Parser::State::PARSING_GFX},
		{"\n__label__", Parser::State::PARSING_LABEL},
		{"\n__gff__", Parser::State::PARSING_GFF},
		{"\n__map__", Parser::State::PARSING_MAP},
		{"\n__sfx__", Parser::State::PARSING_SFX},
		{"\n__music__", Parser::State::PARSING_MUSIC},
	}};

Parser::Parser()
	: _current_state(State::EXPECT_HEADER), _current_gfx_nibble(0),
	  _current_tile_nibble(2 * 128 * 32), // we start on the bottom 32 rows
	  _current_gff_nibble(0) {}

#define PARSER_CHECK(cond, err_code)                                           \
	if (!(cond)) {                                                             \
		return (err_code);                                                     \
	}

ParserStatus Parser::consume(hal::ReaderCallback &fs_reader, void *fs_ud) {
	LookaheadReader r(fs_reader, fs_ud);

	const auto try_read_block_header = [&]() -> bool {
		if (_current_state != State::EXPECT_HEADER &&
		    _current_state != State::EXPECT_VERSION) {

			for (const auto &[to_match, matched_state] :
			     state_matchers_leading_newline) {
				// matches without the leading newline?
				if (r.peek_matches(to_match.substr(1))) {
					// this string_view is known to be nul-terminated
					printf("Parsing %s block\n", to_match.substr(1).data());
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

		if (_current_state == State::EXPECT_BLOCK) {
			// failed to match any block, fail
			return ParserStatus::BAD_INITIAL_BLOCK;
		}

		switch (_current_state) {
		case State::EXPECT_HEADER: {
			// allow trash or space after this
			PARSER_CHECK(
				r.consume_matches("pico-8 cartridge // http://www.pico-8.com"),
				ParserStatus::BAD_CARTRIDGE_HEADER);
			r.consume_until_next_line();
			_current_state = State::EXPECT_VERSION;
			break;
		}

		case State::EXPECT_VERSION: {
			// allow any version string after this
			PARSER_CHECK(r.consume_matches("version "),
			             ParserStatus::BAD_VERSION_LINE);
			r.consume_until_next_line();
			_current_state = State::EXPECT_BLOCK;
			break;
		}

		case State::PARSING_LUA: {
			LuaBlockReaderState lua_block_state{
				.parser = this, .reader = &r, .eof = false};
			emu::emulator.load_and_inject_header(lua_block_reader_callback,
			                                     &lua_block_state);
			_current_state = State::EXPECT_BLOCK;
			break;
		}

		case State::PARSING_GFX: {
			auto sprite_sheet = emu::device<devices::Spritesheet>;
			char c;
			while (r.consume_if(c, [](char c) { return hex_digit(c) != -1; })) {
				sprite_sheet.set_nibble(_current_gfx_nibble, hex_digit(c));
				++_current_gfx_nibble;
			}
			// allow garbage/spaces after non-hex
			r.consume_until_next_line();
			break;
		}

		case State::PARSING_MAP: {
			auto map = emu::device<devices::Map>;
			char c;
			while (r.consume_if(c, [](char c) { return hex_digit(c) != -1; })) {
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

		case State::PARSING_GFF: {
			auto sprite_flags = emu::device<devices::SpriteFlags>;
			char c;
			while (r.consume_if(c, [](char c) { return hex_digit(c) != -1; })) {
				putchar(c);

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
			putchar('\n');
			break;
		}

		default:
			r.consume_until_next_line();
			break;
		}
	}

	return ParserStatus::OK;
}

const char *lua_block_reader_callback(void *ud, std::size_t *size) {
	LuaBlockReaderState &state = *static_cast<LuaBlockReaderState *>(ud);
	LookaheadReader &r = *state.reader;

	// Last buffer read stopped at a newline?
	if (r.peek_char() == '\n') {
		// Can we match a cartridge block?
		for (const auto &[to_match, matched_state] :
		     state_matchers_leading_newline) {
			if (r.peek_matches(to_match)) {
				// New block? return to parent
				r.consume_char(); // Eat the newline
				*size = 0;
				state.eof = true;
				return nullptr;
			}
		}
	}

	int chars_read = 0;
	const char *buf = r.consume_rest_of_current_buffer_while(
		[&chars_read](char c) {
			// don't return if the first char is '\n'
			return (chars_read++ == 0 || c != '\n') && c != '\0';
		},
		size);

	return buf;
}
} // namespace p8::detail