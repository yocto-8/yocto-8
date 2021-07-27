#include "video.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>

namespace emu::bindings
{

int y8_pset(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);
    const auto v = luaL_checkunsigned(state, 3);
    
    if (x >= 128 || y >= 128)
    {
        // Out of bounds
        // TODO: is this the proper behavior?
        return 0;
    }

    emulator.mmio().frame_buffer().set_pixel(x, y, v % 16);

    return 0;
}

int y8_pget(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);

    if (x >= 128 || y >= 128)
    {
        lua_pushunsigned(state, 0);
        return 1;
    }

    lua_pushunsigned(state, emulator.mmio().frame_buffer().get_pixel(x, y));
    return 1;
}

int y8_cls(lua_State* state)
{
    std::uint8_t palette_entry = 0;

    // FIXME: should reset text cursor pos to (0, 0)

    const auto argument_count = lua_gettop(state);

    if (argument_count >= 1)
    {
        palette_entry = luaL_checkunsigned(state, 1);
    }

    emulator.mmio().frame_buffer().clear(palette_entry);

    return 0;
}

int y8_spr(lua_State* state)
{
    // negative width does not flip the sprite, it just draws nothing

    const auto argument_count = lua_gettop(state);

    const auto n = luaL_checkunsigned(state, 1);
    const auto raw_orig_x = luaL_checkinteger(state, 2);
    const auto raw_orig_y = luaL_checkinteger(state, 3);

    std::uint8_t width = 8, height = 8;
    bool x_flip = false, y_flip = false;

    // when providing only 4 arguments, pico-8 does not produce an error, but the sprite is not visible
    // we assume no game rely on this behavior (why would any)

    if (argument_count >= 5)
    {
        width = std::uint8_t(luaL_checknumber(state, 4) * 8);
        height = std::uint8_t(luaL_checknumber(state, 5) * 8);
    }

    if (argument_count >= 6)
    {
        x_flip = lua_toboolean(state, 6);
    }

    if (argument_count >= 7)
    {
        y_flip = lua_toboolean(state, 7);
    }

    // TODO: canvas
    const auto orig_x = std::max(raw_orig_x, 0);
    const auto orig_y = std::max(raw_orig_y, 0);

    const auto target_x = std::min(raw_orig_x + width, 127);
    const auto target_y = std::min(raw_orig_y + height, 127);

    constexpr std::size_t sprites_per_row = 128 / 8;
    const auto sprite_orig_x = (n % sprites_per_row) * 8;
    const auto sprite_orig_y = (n / sprites_per_row) * 8;

    auto sprite = emulator.mmio().sprite_sheet();
    auto fb = emulator.mmio().frame_buffer();
    auto palette = emulator.mmio().draw_palette();

    for (std::uint8_t y = orig_y; y < target_y; ++y)
    {
        for (std::uint8_t x = orig_x; x < target_x; ++x)
        {
            // TODO: resolve palette color
            // TODO: flip_x and flip_y
            const auto sprite_x = (x - orig_x) + sprite_orig_x;
            const auto sprite_y = (y - orig_y) + sprite_orig_y;

            const std::uint8_t palette_entry = palette[sprite.get_pixel(sprite_x, sprite_y)];

            const auto color = palette_entry & 0x0F;
            const bool transparent = (palette_entry >> 4) != 0;

            if (!transparent)
            {
                fb.set_pixel(x, y, color & 0x0F);
            }
        }
    }

    return 0;
}

}