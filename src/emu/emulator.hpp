#pragma once

#include <lua.h>

#include <array>
#include <gsl/gsl>
#include <tinyalloc.h>
#include <video/framebufferview.hpp>

namespace emu
{

class Emulator
{
public:
    // TODO: should be some singleton instead.. and not require init()
    constexpr Emulator() :
        _memory{},
        _button_state(0),
        _lua(nullptr)
    {}

    ~Emulator();

    void init(gsl::span<char> memory_buffer);

    void load(gsl::span<const char> buf);

    void run();

    void hook_update();

    Emulator(const Emulator&) = delete;
    Emulator& operator=(const Emulator&) = delete;

    video::FramebufferView frame_buffer()
    {
        return {gsl::span(_memory).subspan<0x6000, 8192>()};
    }

private:
    static int y8_pset(lua_State* state);

    static int y8_btn(lua_State* state);

    gsl::span<char> _memory_buffer;
    std::array<std::uint8_t, 65536> _memory;
    std::uint16_t _button_state;
    lua_State* _lua;
};

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize);

extern Emulator emulator;

}