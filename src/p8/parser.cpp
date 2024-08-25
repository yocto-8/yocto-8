#include "parser.hpp"

#include <emu/emulator.hpp>
#include <devices/image.hpp>
#include <devices/map.hpp>
#include <devices/spriteflags.hpp>

namespace p8
{

std::uint8_t hex_digit(char c)
{
    switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 0xA;
    case 'b': case 'B': return 0xB;
    case 'c': case 'C': return 0xC;
    case 'd': case 'D': return 0xD;
    case 'e': case 'E': return 0xE;
    case 'f': case 'F': return 0xF;
    }

    return -1;
}

Parser::Parser(std::string_view source) :
    _source(source),
    _current_block_offset(0),
    _current_offset(0),
    _current_state(State::EXPECT_HEADER),
    _current_gfx_nibble(0),
    _current_tile_nibble(2 * 128 * 32), // we start on the bottom 32 rows
    _current_gff_nibble(0)
{}

bool Parser::parse_line()
{
    const std::string_view left_to_parse = _source.substr(_current_offset);
    const auto line_end = left_to_parse.find('\n');

    const std::size_t last_line_offset = _current_offset - 1;

    const auto end_block = [&] {
        switch (_current_state)
        {
        case State::PARSING_LUA:
        {
            _lua_block = _source.substr(_current_block_offset, last_line_offset - _current_block_offset + 1);
            break;
        }

        default: break;
        }
    };

    std::string_view current_line;
    if (line_end != std::string_view::npos)
    {
        current_line = std::string_view(left_to_parse.data(), line_end);
        _current_offset += line_end + 1;
    }
    else
    {
        current_line = left_to_parse;
        _current_offset += current_line.size();
    }

    if (left_to_parse.empty())
    {
        end_block();
        finalize();
        return false;
    }

    const auto match_block = [&](const auto magic, State associated_state) {
        if (current_line.starts_with(magic))
        {
            end_block();
            _current_block_offset = _current_offset;
            _current_state = associated_state;
            return true;
        }

        return false;
    };

    if (match_block("__lua__", State::PARSING_LUA)
        || match_block("__gfx__", State::PARSING_GFX)
        || match_block("__label__", State::PARSING_LABEL)
        || match_block("__gff__", State::PARSING_GFF)
        || match_block("__map__", State::PARSING_MAP)
        || match_block("__sfx__", State::PARSING_SFX)
        || match_block("__music__", State::PARSING_MUSIC))
    {
        return true;
    }

    switch (_current_state)
    {
    case State::EXPECT_HEADER:
    {
        _current_state = State::EXPECT_VERSION;
        break;
    }

    case State::EXPECT_VERSION:
    {
        end_block();
        if (current_line.substr(0, 7) != "version")
        {
            _current_state = State::ERROR_UNKNOWN_HEADER;
            return false;
        }

        _current_state = State::EXPECT_BLOCK;
        break;
    }

    case State::PARSING_GFX:
    {
        auto sprite_sheet = emu::device<devices::Spritesheet>;
        for (const char c: current_line)
        {
            sprite_sheet.set_nibble(_current_gfx_nibble, hex_digit(c));
            ++_current_gfx_nibble;
        }
        break;
    }

    case State::PARSING_MAP:
    {
        auto map = emu::device<devices::Map>;
        // TODO: less stupid nibble logic
        for (const char c: current_line)
        {
            if (_current_tile_nibble % 2 == 0)
            {
                map.data[_current_tile_nibble / 2] |= hex_digit(c) << 4;
            }
            else
            {
                map.data[_current_tile_nibble / 2] |= hex_digit(c);
            }
            ++_current_tile_nibble;
        }
        break;
    }

    case State::PARSING_GFF:
    {
        // TODO: dedup logic with map
        auto sprite_flags = emu::device<devices::SpriteFlags>;
        for (const char c: current_line)
        {
            if (_current_gff_nibble % 2 == 0)
            {
                sprite_flags.flags_for(_current_gff_nibble / 2) |= hex_digit(c) << 4;
            }
            else
            {
                sprite_flags.flags_for(_current_gff_nibble / 2) |= hex_digit(c);
            }
            ++_current_gff_nibble;
        }
        break;
    }

    default: break;
    }

    return true;
}

void Parser::finalize()
{
    emu::emulator.load(_lua_block);
}

}