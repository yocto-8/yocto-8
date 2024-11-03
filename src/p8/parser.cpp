#include "parser.hpp"
#include "coredefs.hpp"
#include "devices/image.hpp"
#include "hal/hal.hpp"
#include "llex.hpp"
#include "p8/lookahead.hpp"
#include <emu/bufferio.hpp>
#include <emu/emulator.hpp>
#include <emu/mmio.hpp>

#include "lstate.hpp"
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

static constexpr auto state_matchers_leading_newline =
	std::to_array<std::pair<std::string_view, ParserState>>({
		{"\n__lua__", ParserState::PARSING_LUA},
		{"\n__gfx__", ParserState::PARSING_GFX},
		{"\n__label__", ParserState::PARSING_LABEL},
		{"\n__gff__", ParserState::PARSING_GFF},
		{"\n__map__", ParserState::PARSING_MAP},
		{"\n__sfx__", ParserState::PARSING_SFX},
		{"\n__music__", ParserState::PARSING_MUSIC},
	});

RemappedDevices RemappedDevices::from_config(ParserMapConfig config) {
	auto memory = emu::emulator.memory();
	return {.gfx = config.remap_device<devices::Spritesheet>(memory),
	        .gfx_addr = config.remap(devices::Spritesheet::default_map_address),
	        .map = config.remap_device<devices::Map>(memory),
	        .map_addr = config.remap(devices::Map::default_map_address),
	        .gff = config.remap_device<devices::SpriteFlags>(memory),
	        .gff_addr = config.remap(devices::SpriteFlags::default_map_address),
	        .music = config.remap_device<devices::Music>(memory),
	        .music_addr = config.remap(devices::Music::default_map_address)};
}

Parser::Parser(ParserMapConfig map_config, global_State *lua_global_state)
	: _mapping(map_config), _dev{RemappedDevices::from_config(map_config)},
	  _current_state(ParserState::EXPECT_HEADER), _off_gfx(0),
	  _off_map(128 * 32), // we start on the bottom 32 rows
	  _off_gff(0), _off_music(0), _lua_global_state(lua_global_state) {}

// NOLINTNEXTLINE
#define PARSER_CHECK(cond, err_code)                                           \
	if (!(cond)) {                                                             \
		return (err_code);                                                     \
	}

ParserStatus Parser::consume(hal::ReaderCallback &fs_reader, void *fs_ud) {
	using namespace y8;

	// Clear the target memory region
	if (_mapping.clear_memory) {
		emu::emulator.memory().fill(0, _mapping.target_region_start,
		                            _mapping.region_size);
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

	/// Simple line parser for GFX/MAP/GFF blocks with identical nibble logic
	const auto parse_nibble_line =
		[&r, this](auto &device, PicoAddr device_addr, std::size_t &offset,
	               emu::NibbleOrder order) {
			while (hex_digit(r.peek_char()) != -1) {
				int nibble1 = hex_digit(r.consume_char());
				int nibble2 = hex_digit(r.consume_char());

				if (_mapping.is_target_mapped(device_addr + offset)) {
					device.set_nibble(offset * 2, nibble1, order);
					device.set_nibble(offset * 2 + 1, nibble2, order);
				}

				++offset;
			}
			// allow garbage/spaces after non-hex
			r.consume_until_next_line();
		};

	// Parsing loop, per-line.
	while (!r.is_eof()) {
		if (try_read_block_header()) {
			continue;
		}

		// Caller doesn't want to parse this section?
		// Skip to the next line (faster than handling it later).
		if ((_mapping.state_mask & u32(_current_state)) == 0) {
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
			parse_nibble_line(_dev.gfx, _dev.gfx_addr, _off_gfx,
			                  emu::NibbleOrder::LSB_FIRST);
			break;
		}

		case ParserState::PARSING_MAP: {
			parse_nibble_line(_dev.map, _dev.map_addr, _off_map,
			                  emu::NibbleOrder::MSB_FIRST);
			break;
		}

		case ParserState::PARSING_GFF: {
			parse_nibble_line(_dev.gff, _dev.gff_addr, _off_gff,
			                  emu::NibbleOrder::MSB_FIRST);
			break;
		}

		case ParserState::PARSING_SFX: {
			// FIXME: implement SFX block parsing
			r.consume_until_next_line();
			break;
		}

		case ParserState::PARSING_MUSIC: {
			if (r.consume_empty_line()) {
				break;
			}

			// consider `0c 727c7f7f`

			// in the example, parse `0c` into `flag`
			int flag_high = hex_digit(r.consume_char());
			PARSER_CHECK(flag_high != -1, ParserStatus::BAD_MUSIC_FLAG);
			int flag_low = hex_digit(r.consume_char());
			PARSER_CHECK(flag_low != -1, ParserStatus::BAD_MUSIC_FLAG);
			u8 flag = (flag_high << 4) | flag_low;

			// in the example, consume the space
			PARSER_CHECK(r.consume_char() == ' ', ParserStatus::BAD_MUSIC_FLAG);

			// in the example, consume all four bytes as a pattern description
			for (int i = 0; i < 4; ++i) {
				int pattern_high = hex_digit(r.consume_char());
				PARSER_CHECK(pattern_high != -1,
				             ParserStatus::BAD_MUSIC_PATTERN);
				int pattern_low = hex_digit(r.consume_char());
				PARSER_CHECK(pattern_low != -1,
				             ParserStatus::BAD_MUSIC_PATTERN);

				u8 pattern = (pattern_high << 4) | pattern_low;

				// `flag` contains four flags at the least significant bits
				// each flag is mapped to one pattern, at the MSB.
				// i.e. flag 0 goes to patterns[0], etc.
				pattern |= ((flag >> i) & 0b1) << 7;

				if (_mapping.is_target_mapped(_dev.music_addr + _off_music * 4 +
				                              i)) {
					_dev.music.get_byte(_off_music * 4 + i) = pattern;
				}
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

			// Consume spaces after #include
			while (r.consume_if([](char c) { return c == ' ' || c == '\t'; }))
				;

			// Parse filename into a buffer
			std::array<char, 128> include_fname; // NOLINT
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