#include "video.hpp"

#include <cmath>
#include <lua.h>
#include <lauxlib.h>

#include <emu/emulator.hpp>
#include <devices/clippingrectangle.hpp>
#include <devices/drawpalette.hpp>
#include <devices/drawstatemisc.hpp>
#include <devices/screenpalette.hpp>
#include <devices/image.hpp>
#include <devices/spriteflags.hpp>
#include <devices/map.hpp>
#include <util/point.hpp>

using util::Point;

namespace emu::bindings
{

namespace detail
{

// TODO: the raw_color naming is confusing:
// - one refers to having the fill pattern color in the upper nibble (set_pixel_with_pattern)
// - one refers to having the transparency info (set_pixel_with_alpha)
// it is currently past 1am and my brain infodumps better than it chooses names

// FIXME: most bindings do not properly set the pen color as they should.

inline void set_pixel_with_pattern(Point p, std::uint8_t raw_color)
{
    auto fb = device<devices::Framebuffer>;
    auto draw_state = device<devices::DrawStateMisc>;

    if (draw_state.fill_pattern_at(p.x, p.y))
    {
        fb.set_pixel(p.x, p.y, raw_color >> 4);
    }
    else if (!draw_state.fill_zero_is_transparent())
    {
        fb.set_pixel(p.x, p.y, raw_color & 0b1111);
    }
}

inline void set_pixel_with_alpha(Point p, std::uint8_t raw_color)
{
    const auto color = raw_color & 0x0F;
    const bool transparent = (raw_color >> 4) != 0;

    if (!transparent)
    {
        device<devices::Framebuffer>.set_pixel(p.x, p.y, color & 0x0F);
    }
}

inline void draw_line(Point a, Point b, std::uint8_t raw_color)
{
    const auto clip = device<devices::ClippingRectangle>;

    const bool steep = std::abs(b.y - a.y) > std::abs(b.x - a.x);

    if (steep)
    {
        a = a.yx();
        b = b.yx();
    }

    if (a.x > b.x)
    {
        std::swap(a, b);
    }

    const auto dx = b.x - a.x;
    const auto dy = std::abs(b.y - a.y);

    auto error = dx / 2;
    const auto y_step = (a.y < b.y) ? 1 : -1;

    int y = a.y;
    for (int x = a.x; x <= b.x; ++x)
    {
        Point plot_point(x, y);
        if (steep)
        {
            plot_point = plot_point.yx();
        }

        if (clip.contains(plot_point))
        {
            detail::set_pixel_with_pattern(plot_point, raw_color);
        }

        error -= dy;

        if (error < 0)
        {
            y += y_step;
            error += dx;
        }
    }
}

// TODO: make a Vec class already that does the transforms jeez
inline Point worldspace_to_screenspace(Point p)
{
    const auto draw_misc = device<devices::DrawStateMisc>;

    return Point(
        p.x - draw_misc.camera_x(),
        p.y - draw_misc.camera_y()
    );
}

[[gnu::always_inline]]
inline void draw_sprite(int sprite_index, Point unclipped_origin, int width = 8, int height = 8, bool x_flip = false, bool y_flip = false)
{
    auto sprite = device<devices::Spritesheet>;
    auto palette = device<devices::DrawPalette>;
    auto clip = device<devices::ClippingRectangle>;

    const Point top_left = unclipped_origin.max(clip.top_left());
    const Point bottom_right = (unclipped_origin.with_offset(width, height)).min(clip.bottom_right());

    constexpr int sprites_per_row = 128 / 8;
    const int sprite_orig_x = (sprite_index % sprites_per_row) * 8;
    const int sprite_orig_y = (sprite_index / sprites_per_row) * 8;

    const auto main_loop = [&](bool x_flip, bool y_flip) {
        for (int y = top_left.y; y < bottom_right.y; ++y)
        {
            for (int x = top_left.x; x < bottom_right.x; ++x)
            {
                const int x_offset = x - top_left.x;
                const int y_offset = y - top_left.y;

                int sprite_x, sprite_y;

                if (!x_flip)
                {
                    sprite_x = sprite_orig_x + x_offset;
                }
                else
                {
                    sprite_x = sprite_orig_x + width - 1 - x_offset;
                }

                if (!y_flip)
                {
                    sprite_y = sprite_orig_y + y_offset;
                }
                else
                {
                    sprite_y = sprite_orig_y + height - 1 - y_offset;
                }

                const std::uint8_t palette_entry = palette.get_color(sprite.get_pixel(sprite_x, sprite_y));

                detail::set_pixel_with_alpha(Point(x, y), palette_entry);
            }
        }
    };

    // we manually specialize the main loop for all x_flip/y_flip combinations
    // this causes slight code bloat (not by a lot, actually) but improves performance.
    if (x_flip && y_flip)
    {
        main_loop(true, true);
    }
    else if (x_flip && !y_flip)
    {
        main_loop(true, false);
    }
    else if (!x_flip && y_flip)
    {
        main_loop(false, true);
    }
    else
    {
        main_loop(false, false);
    }
}

}

int y8_camera(lua_State* state)
{
    const std::int16_t x = lua_tointeger(state, 1);
    const std::int16_t y = lua_tointeger(state, 2);

    device<devices::DrawStateMisc>.set_camera_position(Point(x, y));

    return 0;
}

int y8_pset(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const Point world_point(lua_tointeger(state, 1), lua_tointeger(state, 2));
    const Point p = detail::worldspace_to_screenspace(world_point);

    std::uint8_t color = device<devices::DrawStateMisc>.raw_pen_color() % 16;

    if (argument_count >= 3)
    {
        color = lua_tounsigned(state, 3);
    }

    if (device<devices::ClippingRectangle>.contains(p))
    {
        device<devices::Framebuffer>.set_pixel(p.x, p.y, color);
    }

    return 0;
}

int y8_pget(lua_State* state)
{
    const int world_x = lua_tointeger(state, 1);
    const int world_y = lua_tointeger(state, 2);
    Point p = detail::worldspace_to_screenspace({world_x, world_y});

    std::uint8_t pixel = 0;
    if (device<devices::ClippingRectangle>.contains(p))
    {
        pixel = device<devices::Framebuffer>.get_pixel(p.x, p.y);
    }

    lua_pushunsigned(state, pixel);
    
    return 1;
}

int y8_fget(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto sprite_flags = device<devices::SpriteFlags>;

    // FIXME: how does this value wrap, too?
    const auto sprite_id = lua_tounsigned(state, 1);

    if (argument_count >= 2)
    {
        // FIXME: is this value wrapping %16 in pico-8?
        const auto flag_index = lua_tounsigned(state, 2);

        lua_pushboolean(state, sprite_flags.get_flag(sprite_id, flag_index));
        return 1;
    }

    lua_pushunsigned(state, sprite_flags.flags_for(sprite_id));
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

    unsigned raw_color = device<devices::DrawStateMisc>.raw_pen_color();

    if (argument_count >= 4)
    {
        if (argument_count >= 5)
        {
            raw_color = lua_tounsigned(state, 5);
        }

        const Point world_a(lua_tointeger(state, 1), lua_tointeger(state, 2));
        const Point world_b(lua_tointeger(state, 3), lua_tointeger(state, 4));

        detail::draw_line(
            detail::worldspace_to_screenspace(world_a),
            detail::worldspace_to_screenspace(world_b),
            raw_color
        );

        draw_misc.set_line_endpoint(world_b);
    }
    else if (argument_count >= 2)
    {
        if (argument_count >= 3)
        {
            raw_color = lua_tounsigned(state, 3);
        }

        const Point world_a(lua_tointeger(state, 1), lua_tointeger(state, 2));
        const Point world_b(draw_misc.line_endpoint_x(), draw_misc.line_endpoint_y());

        detail::draw_line(
            detail::worldspace_to_screenspace(world_a),
            detail::worldspace_to_screenspace(world_b),
            raw_color
        );

        draw_misc.set_line_endpoint(world_a);
    }
    else
    {
        if (argument_count >= 1)
        {
            // FIXME: everywhere raw_color is used should WRITE BACK TO MEMORY
            raw_color = lua_tounsigned(state, 1);
        }

        draw_misc.set_line_endpoint_valid(false);
    }

    return 0;
}

int y8_rectfill(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    auto world_x0 = lua_tointeger(state, 1);
    auto world_y0 = lua_tointeger(state, 2);
    auto world_x1 = lua_tointeger(state, 3);
    auto world_y1 = lua_tointeger(state, 4);

    Point top_left = detail::worldspace_to_screenspace(Point(world_x0, world_y0));
    Point bottom_right = detail::worldspace_to_screenspace(Point(world_x1, world_y1));

    auto clip = device<devices::ClippingRectangle>;

    unsigned raw_color = device<devices::DrawStateMisc>.raw_pen_color();
    
    if (argument_count >= 5)
    {
        raw_color = lua_tounsigned(state, 5);
    }

    top_left.x = std::max(top_left.x, int(clip.x_begin()));
    top_left.y = std::max(top_left.y, int(clip.y_begin()));
    bottom_right.x = std::min(bottom_right.x, int(clip.x_end() + 1));
    bottom_right.y = std::min(bottom_right.y, int(clip.y_end() + 1));

    for (int y = top_left.y; y <= bottom_right.y; ++y)
    {
        for (int x = top_left.x; x <= bottom_right.x; ++x)
        {
            detail::set_pixel_with_pattern(Point(x, y), raw_color);
        }
    }

    return 0;
}

int y8_spr(lua_State* state)
{
    // negative width does not flip the sprite, it just draws nothing

    const auto argument_count = lua_gettop(state);

    const auto sprite_index = luaL_checkunsigned(state, 1);

    const int world_origin_x = lua_tointeger(state, 2);
    const int world_origin_y = lua_tointeger(state, 3);
    const auto unclipped_origin = detail::worldspace_to_screenspace(Point(world_origin_x, world_origin_y));

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

    detail::draw_sprite(sprite_index, unclipped_origin, width, height, x_flip, y_flip);

    return 0;
}

int y8_pal(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    // TODO: double check that no args reset both palettes
    if (argument_count == 0)
    {
        device<devices::DrawPalette>.reset();
        device<devices::ScreenPalette>.reset();
        return 0;
    }

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

int y8_mset(lua_State* state)
{
    const auto x = lua_tointeger(state, 1);
    const auto y = lua_tointeger(state, 2);
    const std::uint8_t tile = lua_tounsigned(state, 3) % 256;

    const auto map = device<devices::Map>;

    if (map.in_bounds(x, y))
    {
        map.tile(x, y) = tile;
    }

    return 0;
}

int y8_mget(lua_State* state)
{
    const auto x = lua_tointeger(state, 1);
    const auto y = lua_tointeger(state, 2);

    const auto map = device<devices::Map>;

    if (map.in_bounds(x, y))
    {
        lua_pushunsigned(state, map.tile(x, y));
    }
    else
    {
        lua_pushunsigned(state, 0);
    }

    return 1;
}

int y8_map(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto sprite_flags = device<devices::SpriteFlags>;

    const auto tile_x_raw_origin = lua_tointeger(state, 1);
    const auto tile_y_raw_origin = lua_tointeger(state, 2);
    
    const Point world_origin(lua_tointeger(state, 3), lua_tointeger(state, 4));
    const Point screen_origin = detail::worldspace_to_screenspace(world_origin);
    
    int tile_width = 128;
    int tile_height = 128;

    unsigned layer_mask = 0;

    if (argument_count >= 7)
    {
        layer_mask = lua_tounsigned(state, 7);
    }

    if (argument_count >= 5)
    {
        tile_width = lua_tointeger(state, 5);
        tile_height = lua_tointeger(state, 6);

        if (tile_width < 0 || tile_height < 0)
        {
            return 0;
        }
    }

    const auto tile_x_origin = std::max(tile_x_raw_origin, 0);
    const auto tile_y_origin = std::max(tile_y_raw_origin, 0);
    const auto tile_x_end = std::min(tile_x_raw_origin + tile_width, 127);
    const auto tile_y_end = std::min(tile_y_raw_origin + tile_height, 127);

    const auto map = device<devices::Map>;

    for (int tile_y = tile_y_origin; tile_y < tile_y_end; ++tile_y)
    {
        for (int tile_x = tile_x_origin; tile_x < tile_x_end; ++tile_x)
        {
            const auto screen_x_offset = screen_origin.x + (tile_x - tile_x_raw_origin) * 8;
            const auto screen_y_offset = screen_origin.y + (tile_y - tile_y_raw_origin) * 8;

            const auto tile = map.tile(tile_x, tile_y);

            // is it the layer we want to render?
            if ((sprite_flags.flags_for(tile) & layer_mask) != layer_mask)
            {
                continue;
            }

            detail::draw_sprite(tile, Point(screen_x_offset, screen_y_offset));
        }
    }

    return 0;
}

}