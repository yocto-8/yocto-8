#include "parser.hpp"

#include <emu/emulator.hpp>

namespace p8
{

Parser::Parser(std::string_view source) :
    _source(source),
    _current_block_offset(0),
    _current_offset(0),
    _current_state(State::EXPECT_HEADER)
{}

bool Parser::parse_line()
{
    std::string_view current_line;

    std::string_view left_to_parse = _source.substr(_current_offset);

    const auto line_end = left_to_parse.find('\n');

    std::size_t last_line_offset = _current_offset - 1;
    if (line_end != std::string_view::npos)
    {
        current_line = std::string_view(left_to_parse.data(), line_end);
        _current_offset += line_end + 1;
    }
    else
    {
        _current_state = State::DONE;
        return false;
    }

    const auto end_block = [&] {
        switch (_current_state)
        {
        case State::PARSING_LUA:
        {
            emu::emulator.load({&_source[_current_block_offset], &_source[last_line_offset]});
            break;
        }

        default: break;
        }
    };

    const auto match_block = [&](const auto magic, State associated_state) {
        if (current_line == magic)
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

    default: break;
    }

    return true;
}

}