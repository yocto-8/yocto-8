#pragma once

#include <lua.h>

#include <array>
#include <gsl/gsl>
#include <tinyalloc.h>
#include <video/framebufferview.hpp>

namespace emu
{

// This is a class, but we really need one instance, and the singleton pattern may not be ideal for perf reasons
// If most things are static and only access the `emulator` instance then stuff *should* be able to be translated
// into an address - e.g. for frame_buffer() - rather than an indirect lookup

class Emulator
{
public:
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

    gsl::span<std::uint8_t, 8> random_state()
    {
        return {gsl::span(_memory).subspan<0x5f44, 8>()};
    }

private:
    // video
    static int y8_pset(lua_State* state);
    static int y8_pget(lua_State* state);
    static int y8_cls(lua_State* state);

    // input
    static int y8_btn(lua_State* state);

    // mmio
    static int y8_memcpy(lua_State* state);

    // math
    static int y8_flr(lua_State* state);

    // rng
    static int y8_rnd(lua_State* state);

    gsl::span<char> _memory_buffer;
    std::array<std::uint8_t, 65536> _memory;
    std::uint16_t _button_state; // FIXME: this should be in _memory
    lua_State* _lua;
};

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize);

extern Emulator emulator;

}