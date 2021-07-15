#include "emulator.hpp"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <cstdio>

namespace emu
{

Emulator::Emulator()
{
    ta_init(
        _memory_buffer.data(),
        _memory_buffer.data() + _memory_buffer.size(),
        8192,
        16,
        sizeof(std::uintptr_t)
    );

    _lua = lua_newstate(lua_alloc, nullptr);
    luaL_openlibs(_lua);
}

Emulator::~Emulator()
{
    lua_close(_lua);
}

void Emulator::load(gsl::span<const char> buf)
{
    const int load_status = luaL_loadbuffer(_lua, buf.data(), buf.size(), "main");
    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("Script load failed: %s\n", lua_tostring(_lua, -1));
    }
    else
    {
        printf("Loaded main segment successfully (%d bytes)\n", int(buf.size()));
    }
}

void Emulator::hook_update()
{
    lua_getglobal(_lua, "_update");
    if (lua_pcall(_lua, 0, 0, 0) != 0)
    {
        printf("_update failed: %s\n", lua_tostring(_lua, -1));
    }
}

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
    (void)ud;

    //printf("blk usage %d used/%d free/%d fresh\n", ta_num_used(), ta_num_free(), ta_num_fresh());

    if (nsize == 0)
    {
        ta_free(ptr);
        return nullptr;
    }
    else if (ptr == nullptr)
    {
        return ta_alloc(nsize);
    }
    else if (nsize <= osize)
    {
        // whatever
        return ptr;
    }
    else
    {
        const auto new_ptr = ta_alloc(nsize);
        std::memcpy(new_ptr, ptr, osize);
        ta_free(ptr);
        return new_ptr;
    }
}

Emulator emulator;

}