#pragma once

#include <array>
#include <span>
#include <string_view>
#include <lua.h>

#include <emu/mmio.hpp>
#include <video/palette.hpp>

namespace emu
{

class Emulator
{
public:
    constexpr Emulator() = default;
    ~Emulator();

    void init(std::span<std::byte> memory_buffer);

    void load(std::string_view buf);

    void run();

    enum class HookResult
    {
        SUCCESS,
        UNDEFINED,
        LUA_ERROR
    };
    
    HookResult run_hook(const char* name);

    constexpr Memory memory()
    {
        return Memory{std::span(_memory)};
    }

    auto get_memory_alloc_buffer() const
    {
        return _memory_buffer;
    }

    auto& palette()
    {
        return _palette;
    }

    Emulator(const Emulator&) = delete;
    Emulator& operator=(const Emulator&) = delete;

private:
    std::span<std::byte> _memory_buffer;
    std::array<std::uint8_t, 65536> _memory = {};
    std::array<std::uint32_t, 32> _palette = video::default_palette_rgb8;
    lua_State* _lua = nullptr;
};

void* lua_alloc(void* ud, void* ptr, size_t osize, size_t nsize);

extern constinit Emulator emulator;

template<class Device, std::uint16_t map_address = Device::default_map_address>
inline constexpr auto device = Device(emulator.memory().data.subspan<map_address, Device::map_length>());

}