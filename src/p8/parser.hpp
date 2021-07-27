#pragma once

#include <string_view>

namespace p8
{

class Parser
{
    public:
    Parser(std::string_view source);

    enum class State
    {
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

    State get_current_state() const
    {
        return _current_state;
    }

    private:
    void finalize();

    std::string_view _source;
    std::size_t _current_block_offset, _current_offset;
    State _current_state;

    std::string_view _lua_block;
    std::size_t _current_gfx_nibble;
};

}