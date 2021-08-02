#include "video.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <emu/emulator.hpp>
#include <devices/clippingrectangle.hpp>
#include <devices/drawpalette.hpp>
#include <devices/drawstatemisc.hpp>
#include <devices/screenpalette.hpp>
#include <devices/image.hpp>

namespace emu::bindings
{

namespace detail
{

// TODO: the raw_color naming is confusing:
// - one refers to having the fill pattern color in the upper nibble (set_pixel_with_pattern)
// - one refers to having the transparency info (set_pixel_with_alpha)
// it is currently past 1am and my brain infodumps better than it chooses names

void set_pixel_with_pattern(std::uint8_t x, std::uint8_t y, std::uint8_t raw_color)
{
    auto fb = device<devices::Framebuffer>;
    auto draw_state = device<devices::DrawStateMisc>;

    if (draw_state.fill_pattern_at(x, y))
    {
        fb.set_pixel(x, y, raw_color >> 4);
    }
    else if (!draw_state.fill_zero_is_transparent())
    {
        fb.set_pixel(x, y, raw_color & 0b1111);
    }
}

void set_pixel_with_alpha(std::uint8_t x, std::uint8_t y, std::uint8_t raw_color)
{
    const auto color = raw_color & 0x0F;
    const bool transparent = (raw_color >> 4) != 0;

    if (!transparent)
    {
        device<devices::Framebuffer>.set_pixel(x, y, color & 0x0F);
    }
}

void draw_line(std::int16_t x0, std::int16_t y0, std::int16_t x1, std::int16_t y1, std::uint8_t raw_color)
{
    const auto clip = device<devices::ClippingRectangle>;

    const bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const auto dx = x1 - x0;
    const auto dy = std::abs(y1 - y0);

    auto error = dx / 2;
    const auto y_step = (y0 < y1) ? 1 : -1;

    std::int16_t y = y0;
    for (std::int16_t x = x0; x <= x1; ++x)
    {
        auto plot_x = x, plot_y = y;
        if (steep)
        {
            plot_x = y;
            plot_y = x;
        }

        if (clip.contains_integer(plot_x, plot_y))
        {
            detail::set_pixel_with_pattern(plot_x, plot_y, raw_color);
        }

        error -= dy;

        if (error < 0)
        {
            y += y_step;
            error += dx;
        }
    }

    //detail::set_pixel_with_alpha(x, y, color);
}

}

int y8_pset(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);
    const auto v = luaL_checkunsigned(state, 3);

    if (device<devices::ClippingRectangle>.contains(x, y))
    {
        device<devices::Framebuffer>.set_pixel(x, y, v % 16);
    }

    return 0;
}

int y8_pget(lua_State* state)
{
    const auto x = luaL_checkunsigned(state, 1);
    const auto y = luaL_checkunsigned(state, 2);

    std::uint8_t pixel = 0;
    if (device<devices::ClippingRectangle>.contains(x, y))
    {
        pixel = device<devices::Framebuffer>.get_pixel(x, y);
    }

    lua_pushunsigned(state, pixel);
    
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

    device<devices::Framebuffer>.clear(palette_entry);

    return 0;
}

int y8_line(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto draw_misc = device<devices::DrawStateMisc>;

    std::int16_t x0, y0;
    std::int16_t x1 = draw_misc.line_endpoint_x();
    std::int16_t y1 = draw_misc.line_endpoint_y();

    unsigned raw_color = device<devices::DrawStateMisc>.raw_pen_color();

    if (argument_count >= 5)
    {
        raw_color = lua_tounsigned(state, 5);
    }

    if (argument_count >= 4)
    {
        x0 = lua_tointegerx(state, 1, nullptr);
        y0 = lua_tointegerx(state, 2, nullptr);
        x1 = lua_tointegerx(state, 3, nullptr);
        y1 = lua_tointegerx(state, 4, nullptr);

        detail::draw_line(x0, y0, x1, y1, raw_color);

        draw_misc.set_line_endpoint(x1, y1);

        return 0;
    }

    if (argument_count >= 3)
    {
        raw_color = lua_tounsigned(state, 3);
    }

    if (argument_count >= 2)
    {
        x0 = lua_tointegerx(state, 1, nullptr);
        y0 = lua_tointegerx(state, 2, nullptr);

        detail::draw_line(x0, y0, x1, y1, raw_color);

        draw_misc.set_line_endpoint(x0, y0);

        return 0;
    }

    if (argument_count >= 1)
    {
        // FIXME: everywhere raw_color is used should WRITE BACK TO MEMORY
        raw_color = lua_tounsigned(state, 1);
    }

    if (argument_count >= 0)
    {
        draw_misc.set_line_endpoint_valid(false);
        return 0;
    }

    return 0;
}

int y8_rectfill(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    unsigned x0 = lua_tounsigned(state, 1) % 128;
    unsigned y0 = lua_tounsigned(state, 2) % 128;
    unsigned x1 = lua_tounsigned(state, 3) % 128;
    unsigned y1 = lua_tounsigned(state, 4) % 128;

    auto clip = device<devices::ClippingRectangle>;

    unsigned raw_color = device<devices::DrawStateMisc>.raw_pen_color();
    
    if (argument_count >= 5)
    {
        raw_color = lua_tounsigned(state, 5);
    }

    x0 = std::max(x0, unsigned(clip.x_begin()));
    y0 = std::max(y0, unsigned(clip.y_begin()));
    x1 = std::min(x1, unsigned(clip.x_end() + 1));
    y1 = std::min(y1, unsigned(clip.y_end() + 1));

    for (unsigned y = y0; y <= y1; ++y)
    {
        for (unsigned x = x0; x <= x1; ++x)
        {
            detail::set_pixel_with_pattern(x, y, raw_color);
        }
    }

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

    auto sprite = device<devices::Spritesheet>;
    auto palette = device<devices::DrawPalette>;
    auto clip = device<devices::ClippingRectangle>;

    const auto orig_x = std::max(raw_orig_x, int(clip.x_begin()));
    const auto orig_y = std::max(raw_orig_y, int(clip.y_begin()));

    const auto target_x = std::min(raw_orig_x + width, int(clip.x_end()));
    const auto target_y = std::min(raw_orig_y + height, int(clip.y_end()));

    constexpr std::size_t sprites_per_row = 128 / 8;
    const auto sprite_orig_x = (n % sprites_per_row) * 8;
    const auto sprite_orig_y = (n / sprites_per_row) * 8;

    for (std::uint8_t y = orig_y; y < target_y; ++y)
    {
        for (std::uint8_t x = orig_x; x < target_x; ++x)
        {
            // TODO: flip_x and flip_y
            const auto sprite_x = (x - orig_x) + sprite_orig_x;
            const auto sprite_y = (y - orig_y) + sprite_orig_y;

            const std::uint8_t palette_entry = palette.get_color(sprite.get_pixel(sprite_x, sprite_y));

            detail::set_pixel_with_alpha(x, y, palette_entry);
        }
    }

    return 0;
}

int y8_pal(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    // c0 seems to be %16 on pico-8
    
    // c1 can have the higher nibble non-zero for
    // - setting a pixel as transparent (draw palette)
    // - using the secret 16 colors (screen palette)

    const auto c0 = luaL_checkunsigned(state, 1) & 0b0000'1111;
    const auto c1 = luaL_checkunsigned(state, 2);

    std::uint8_t target_palette = 0;

    if (argument_count >= 3)
    {
        target_palette = luaL_checkunsigned(state, 3);
    }

    switch (target_palette)
    {
    case 0: device<devices::DrawPalette>.set_color(c0, c1); break;
    case 1: device<devices::ScreenPalette>.set_raw_color(c0, c1); break;
    default: break; // verified pico-8 behavior: other values do nothing
    }

    return 0;
}

int y8_clip(lua_State* state)
{
    std::uint8_t x = 0, y = 0;
    std::uint8_t width = 128, height = 128;

    const auto argument_count = lua_gettop(state);

    if (argument_count >= 4)
    {
        x = luaL_checkunsigned(state, 1);
        y = luaL_checkunsigned(state, 2);
        width = luaL_checkunsigned(state, 3);
        height = luaL_checkunsigned(state, 4);
    }

    // FIXME: clip_previous optional parameter

    const auto clip = device<devices::ClippingRectangle>;

    lua_pushunsigned(state, clip.x_begin());
    lua_pushunsigned(state, clip.y_begin());
    lua_pushunsigned(state, clip.width());
    lua_pushunsigned(state, clip.height());

    clip.x_begin() = x;
    clip.y_begin() = y;
    clip.x_end() = x + width;
    clip.y_end() = y + height;

    return 4;
}

}