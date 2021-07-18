#pragma once

extern "C"
{
#include <lua.h>
}

#include <array>
#include <gsl/gsl>
#include <tinyalloc.h>
#include <video/framebuffer.hpp>

namespace emu
{

class Emulator
{
public:
    // TODO: should be some singleton instead.. and not require init()
    Emulator();
    ~Emulator();

    void init(gsl::span<char> memory_buffer);

    void load(gsl::span<const char> buf);

    void hook_update();

    Emulator(const Emulator&) = delete;
    Emulator& operator=(const Emulator&) = delete;

    video::Framebuffer frame_buffer;

private:
    static int y8_pset(lua_State* state);

    gsl::span<char> _memory_buffer;
    lua_State* _lua;
};

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize);

extern Emulator emulator;

}