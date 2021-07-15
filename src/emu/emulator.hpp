#pragma once

extern "C"
{
#include <lua.h>
}

#include <array>
#include <gsl/gsl>
#include <tinyalloc.h>

namespace emu
{

class Emulator
{
public:
    // TODO: should be some singleton instead..
    Emulator();
    ~Emulator();

    void load(gsl::span<const char> buf);

    void hook_update();

    Emulator(const Emulator&) = delete;
    Emulator& operator=(const Emulator&) = delete;

private:
    std::array<char, 1024 * 1024 * 8> _memory_buffer;
    lua_State* _lua;
};

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize);

extern Emulator emulator;

}