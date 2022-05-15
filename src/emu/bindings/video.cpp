#include "video.hpp"
#include "hal/hal.hpp"

#include <cassert>
#include <cmath>
#include <concepts>
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
#include <video/defaultfont.hpp>

using util::Point;

namespace emu::bindings
{

namespace detail
{

// TODO: the raw_color naming is confusing:
// - one refers to having the fill pattern color in the upper nibble (set_pixel_with_pattern)
// - one refers to having the transparency info (set_pixel_with_alpha) (UPDATE: now to finish renaming to pen_color)
// it is currently past 1am and my brain infodumps better than it chooses names

inline void set_pixel(Point p, std::uint8_t pen_color)
{
    const auto resolved_color = device<devices::DrawPalette>.get_color(pen_color & 0x0F);
    device<devices::Framebuffer>.set_pixel(p.x, p.y, resolved_color);
}

inline void set_pixel_with_pattern(Point p, std::uint8_t raw_color)
{
    auto draw_state = device<devices::DrawStateMisc>;

    if (draw_state.fill_pattern_at(p.x, p.y))
    {
        set_pixel(p, raw_color >> 4);
    }
    else if (!draw_state.fill_zero_is_transparent())
    {
        set_pixel(p, raw_color & 0b1111);
    }
}

inline void set_pixel_with_alpha(Point p, std::uint8_t pen_color)
{
    const bool transparent = (pen_color >> 4) != 0;

    if (!transparent)
    {
        set_pixel(p, pen_color);
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

inline Point worldspace_to_screenspace(Point p)
{
    const auto draw_misc = device<devices::DrawStateMisc>;
    return p.with_offset(-draw_misc.camera_x(), -draw_misc.camera_y());
}

inline Point screenspace_to_worldspace(Point p)
{
    const auto draw_misc = device<devices::DrawStateMisc>;
    return p.with_offset(draw_misc.camera_x(), draw_misc.camera_y());
}

inline Point sprite_index_to_position(int sprite_index)
{
    constexpr int sprites_per_row = 128 / 8;
    return {
        (sprite_index % sprites_per_row) * 8,
        (sprite_index / sprites_per_row) * 8
    };
}

[[gnu::always_inline]]
inline void draw_sprite(
    Point sprite_origin,
    int sprite_width, int sprite_height,
    Point unclipped_origin,
    int target_width, int target_height,
    bool x_flip = false, bool y_flip = false)
{
    auto sprite = device<devices::Spritesheet>;
    auto palette = device<devices::DrawPalette>;
    auto clip = device<devices::ClippingRectangle>;

    const Point top_left = unclipped_origin.max(clip.top_left());
    const Point bottom_right = (unclipped_origin.with_offset(target_width, target_height)).min(clip.bottom_right());

    const auto main_loop = [&](bool x_flip, bool y_flip) {
        for (int y = top_left.y; y < bottom_right.y; ++y)
        {
            for (int x = top_left.x; x < bottom_right.x; ++x)
            {
                const auto x_offset = (LuaFix16(x - top_left.x) * sprite_width) / LuaFix16(bottom_right.x - top_left.x);
                const auto y_offset = (LuaFix16(y - top_left.y) * sprite_height) / LuaFix16(bottom_right.y - top_left.y);

                int sprite_x, sprite_y;

                if (!x_flip)
                {
                    sprite_x = sprite_origin.x + int(x_offset);
                }
                else
                {
                    sprite_x = sprite_origin.x + sprite_width - 1 - int(x_offset);
                }

                if (!y_flip)
                {
                    sprite_y = sprite_origin.y + int(y_offset);
                }
                else
                {
                    sprite_y = sprite_origin.y + sprite_height - 1 - int(y_offset);
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

inline void draw_circle(
    Point origin,
    int radius,
    std::invocable<Point> auto point_plotter,
    std::invocable<Point, Point> auto symmetric_point_plotter
)
{
    // i don't know how this works but it should
    int f = 1 - radius;
    int ddf_x = 0;
    int ddf_y = -2 * radius;

    int x = 0;
    int y = radius;

    point_plotter(Point(origin.x, origin.y - radius));
    point_plotter(Point(origin.x, origin.y + radius));
    symmetric_point_plotter(Point(origin.x - radius, origin.y), Point(origin.x + radius, origin.y));

    while (x < y)
    {
        if (f >= 0)
        {
            --y;
            ddf_y += 2;
            f += ddf_y;
        }
        ++x;

        ddf_x += 2;
        f += ddf_x + 1;

        symmetric_point_plotter(Point(origin.x - x, origin.y + y), Point(origin.x + x, origin.y + y));
        symmetric_point_plotter(Point(origin.x - x, origin.y - y), Point(origin.x + x, origin.y - y));
        symmetric_point_plotter(Point(origin.x - y, origin.y + x), Point(origin.x + y, origin.y + x));
        symmetric_point_plotter(Point(origin.x - y, origin.y - x), Point(origin.x + y, origin.y - x));
    }
}

// returns the glyph width in pixels
// FIXME: uhhhhhhh this should return height too, or just transform the point.
inline int draw_glyph(
    Point origin,
    std::uint8_t glyph,
    std::span<const std::uint8_t, 2048> font,
    std::uint8_t pen_color
)
{
    const auto clip = device<devices::ClippingRectangle>;

    const auto glyph_offset = std::size_t(glyph) * 8;

    // TODO: this works differently on custom fonts, the default font should be modified to have the
    // same properties header in the special glyph 0 and this function should read it.
    // when that is done, make sure to min(,8)
    int glyph_width = (glyph >= 128) ? 8 : 4;
    int glyph_height = 6;

    for (int local_y = 0; local_y < glyph_height; ++local_y)
    {
        for (int local_x = 0; local_x < glyph_width; ++local_x)
        {
            const Point p = origin.with_offset(local_x, local_y);

            const bool pixel_set = (font[glyph_offset + local_y] >> local_x) & 0b1;

            if (pixel_set && clip.contains(p))
            {
                set_pixel(p, pen_color);
            }
        }
    }

    return glyph_width;
}

// returns the new mouse cursor
inline Point draw_text(
    Point origin,
    std::string_view text,
    std::span<const std::uint8_t, 2048> font,
    std::uint8_t pen_color,
    bool terminal
)
{
    Point current_position = origin;

    int glyph_height = 5;

    for (const char c : text)
    {
        int glyph_width = 0;

        // P8SCII handling
        switch (c)
        {
        case 0: break;
        default:
        {
            glyph_width = draw_glyph(current_position, c, font, pen_color);
            break;
        }
        }

        if (terminal)
        {
            if (current_position.x > 127 - glyph_width)
            {
                current_position.x = origin.x;
                current_position.y += glyph_height + 1;
            }
            else
            {
                current_position.x += glyph_width;
            }

            // TODO: scroll text with some memcpy/memset to the framebuffer when needed
        }
        else
        {
            current_position.x += glyph_width;
        }
    }

    current_position.x = origin.x;
    current_position.y += glyph_height + 1;

    return current_position;
}

}

int y8_camera(lua_State* state)
{
    const std::int16_t x = lua_tointeger(state, 1);
    const std::int16_t y = lua_tointeger(state, 2);

    device<devices::DrawStateMisc>.set_camera_position(Point(x, y));

    return 0;
}

int y8_color(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    auto& pen_color = device<devices::DrawStateMisc>.raw_pen_color();
    const auto old_pen_color = pen_color;

    if (argument_count >= 1)
    {
        pen_color = lua_tounsigned(state, 1);
    }
    else
    {
        pen_color = 6;
    }

    lua_pushinteger(state, old_pen_color);

    return 1;
}

int y8_pset(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const Point world_point(lua_tointeger(state, 1), lua_tointeger(state, 2));
    const Point p = detail::worldspace_to_screenspace(world_point);

    auto& color = device<devices::DrawStateMisc>.raw_pen_color();

    if (argument_count >= 3)
    {
        color = lua_tounsigned(state, 3);
    }

    if (device<devices::ClippingRectangle>.contains(p))
    {
        detail::set_pixel(p, color);
    }

    return 0;
}

int y8_pget(lua_State* state)
{
    const Point world_point(lua_tointeger(state, 1), lua_tointeger(state, 2));
    const Point p = detail::worldspace_to_screenspace(world_point);
    
    std::uint8_t pixel = 0;
    if (device<devices::ClippingRectangle>.contains(p))
    {
        pixel = device<devices::Framebuffer>.get_pixel(p.x, p.y);
    }

    lua_pushunsigned(state, pixel);
    
    return 1;
}

int y8_sset(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const Point p(lua_tointeger(state, 1), lua_tointeger(state, 2));
    auto& color = device<devices::DrawStateMisc>.raw_pen_color();

    if (argument_count >= 3)
    {
        color = lua_tounsigned(state, 3);
    }

    if (p.x >= 0 && p.y >= 0 && p.x < 128 && p.y < 128)
    {
        device<devices::Spritesheet>.set_pixel(p.x, p.y, color);
    }

    return 0;
}

int y8_sget(lua_State* state)
{
    const Point p(lua_tointeger(state, 1), lua_tointeger(state, 2));

    if (p.x >= 0 && p.y >= 0 && p.x < 128 && p.y < 128)
    {
        lua_pushunsigned(state, device<devices::Spritesheet>.get_pixel(p.x, p.y));
    }
    else
    {
        lua_pushunsigned(state, 0);
    }

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

    device<devices::DrawStateMisc>.set_text_point({0, 0});
    device<devices::Framebuffer>.clear(palette_entry);

    return 0;
}

int y8_line(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto draw_misc = device<devices::DrawStateMisc>;

    auto& raw_color = device<devices::DrawStateMisc>.raw_pen_color();

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
            raw_color = lua_tounsigned(state, 1);
        }

        draw_misc.set_line_endpoint_valid(false);
    }

    return 0;
}

int y8_circfill(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const Point world_origin(lua_tointeger(state, 1), lua_tointeger(state, 2));
    const Point screen_origin = detail::worldspace_to_screenspace(world_origin);

    const auto clip = device<devices::ClippingRectangle>;

    int radius = 4;

    auto& raw_color = device<devices::DrawStateMisc>.raw_pen_color();

    if (argument_count >= 3)
    {
        radius = lua_tointeger(state, 3);
    }

    if (argument_count >= 4)
    {
        raw_color = lua_tounsigned(state, 4);
    }

    const auto plot_point = [&](Point p) {
        if (clip.contains(p))
        {
            detail::set_pixel_with_pattern(p, raw_color);
        }
    };

    detail::draw_circle(
        screen_origin,
        radius,
        [&](Point p) { plot_point(p); },
        [&](Point a, Point b) {
            const int x_min = std::max(std::min(a.x, b.x), int(clip.x_begin()));
            const int x_max = std::min(std::max(a.x, b.x), int(clip.x_end()));
            
            for (int x = x_min; x <= x_max; ++x)
            {
                detail::set_pixel_with_pattern(Point(x, a.y), raw_color);
            }
        }
    );

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

    auto& raw_color = device<devices::DrawStateMisc>.raw_pen_color();
    
    if (argument_count >= 5)
    {
        raw_color = lua_tounsigned(state, 5);
    }

    top_left.x = std::max(top_left.x, int(clip.x_begin()));
    top_left.y = std::max(top_left.y, int(clip.y_begin()));
    bottom_right.x = std::min(bottom_right.x, int(clip.x_end()));
    bottom_right.y = std::min(bottom_right.y, int(clip.y_end()));

    for (int y = top_left.y; y < bottom_right.y; ++y)
    {
        for (int x = top_left.x; x < bottom_right.x; ++x)
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

    detail::draw_sprite(
        detail::sprite_index_to_position(sprite_index),
        width, height,
        unclipped_origin,
        width, height,
        x_flip, y_flip);

    return 0;
}

int y8_sspr(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const int sprite_origin_x = lua_tounsigned(state, 1);
    const int sprite_origin_y = lua_tounsigned(state, 2);
    const int sprite_width = lua_tounsigned(state, 3);
    const int sprite_height = lua_tounsigned(state, 4);
    const int world_origin_x = lua_tointeger(state, 5);
    const int world_origin_y = lua_tointeger(state, 6);
    int target_width = sprite_width;
    int target_height = sprite_height;
    
    const auto unclipped_origin = detail::worldspace_to_screenspace(Point(world_origin_x, world_origin_y));

    bool x_flip = false, y_flip = false;

    // when providing only 4 arguments, pico-8 does not produce an error, but the sprite is not visible
    // we assume no game rely on this behavior (why would any)

    if (argument_count >= 7)
    {
        target_width = lua_tointeger(state, 7);
    }

    if (argument_count >= 8)
    {
        target_height = lua_tointeger(state, 8);
    }

    if (argument_count >= 9)
    {
        x_flip = lua_toboolean(state, 9);
    }

    if (argument_count >= 10)
    {
        y_flip = lua_toboolean(state, 10);
    }

    detail::draw_sprite(
        Point(sprite_origin_x, sprite_origin_y),
        sprite_width, sprite_height,
        unclipped_origin,
        target_width, target_height,
        x_flip, y_flip);

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
    // FIXME: case 2 should handle the 2ndary palette
    default: break; // verified pico-8 behavior: other values do nothing
    }

    return 0;
}

int y8_palt(lua_State* state)
{
    const auto draw_palette = device<devices::DrawPalette>;

    const auto argument_count = lua_gettop(state);

    // case: palt(col, [transparent])
    if (argument_count >= 1)
    {
        const auto color = lua_tounsigned(state, 1) % 16;
        const auto is_transparent = lua_toboolean(state, 2);
        draw_palette.set_transparent(color, is_transparent);
    }
    // case: palt()
    else
    {
        draw_palette.set_transparent(0, true);
        for (std::size_t i = 1; i < 16; ++i)
        {
            draw_palette.set_transparent(i, false);
        }
        return 0;
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

int y8_flip(lua_State* state)
{
    emu::emulator.flip();
    return 0;
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

            detail::draw_sprite(
                detail::sprite_index_to_position(tile),
                8, 8,
                Point(screen_x_offset, screen_y_offset),
                8, 8,
                false, false);
        }
    }

    return 0;
}

int y8_cursor(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    const auto draw_misc = device<devices::DrawStateMisc>;

    lua_pushinteger(state, draw_misc.text_x());
    lua_pushinteger(state, draw_misc.text_y());
    lua_pushinteger(state, draw_misc.raw_pen_color());

    draw_misc.text_x() = lua_tointeger(state, 1);
    draw_misc.text_y() = lua_tointeger(state, 2);

    if (argument_count >= 3)
    {
        draw_misc.raw_pen_color() = lua_tointeger(state, 3);
    }
    
    return 3;
}

int y8_print(lua_State* state)
{
    const std::span font(video::pico8_builtin_font);
    const auto draw_misc = device<devices::DrawStateMisc>;

    const auto argument_count = lua_gettop(state);

    // case: print()
    if (argument_count == 0)
    {
        return 0;
    }

    // FIXME: this should allow numbers etc and do the same stuff as printh(?)
    const char* str = lua_tostring(state, 1);

    if (str == nullptr)
    {
        return 0;
    }

    auto& raw_color = device<devices::DrawStateMisc>.raw_pen_color();

    Point world_point;
    bool terminal_scrolling;

    // case: print(text, x, y, [col])
    if (argument_count >= 3)
    {
        if (argument_count >= 4)
        {
            raw_color = lua_tounsigned(state, 4);
        }

        world_point = Point(lua_tointeger(state, 2), lua_tointeger(state, 3));
        terminal_scrolling = false;
    }
    // case: print(text, [col])
    else
    {
        if (argument_count >= 2)
        {
            raw_color = lua_tounsigned(state, 2);
        }

        world_point = draw_misc.get_text_point();
        terminal_scrolling = true;
    }

    const Point screen_point(detail::worldspace_to_screenspace(world_point));
    const Point updated_point = detail::screenspace_to_worldspace(
        detail::draw_text(screen_point, str, font, raw_color, terminal_scrolling)
    );
    draw_misc.set_text_point(updated_point);

    return 0;
}

int y8_rgbpal(lua_State* state)
{
    const auto argument_count = lua_gettop(state);

    auto& palette = emu::emulator.palette();

    // case: rgbpal(entry, [r], [g], [b])
    if (argument_count >= 1)
    {
        const unsigned entry = lua_tounsigned(state, 1) % 32;

        const auto old_value = palette[entry];

        if (argument_count >= 4)
        {
            const unsigned r = lua_tounsigned(state, 2) % 256;
            const unsigned g = lua_tounsigned(state, 3) % 256;
            const unsigned b = lua_tounsigned(state, 4) % 256;

            palette[entry] = (r << 16) | (g << 8) | b;

            hal::load_rgb_palette(palette);
        }

        lua_pushunsigned(state, (old_value >> 16) & 0xFF);
        lua_pushunsigned(state, (old_value >> 8) & 0xFF);
        lua_pushunsigned(state, old_value & 0xFF);

        return 3;
    }

    return 0;
}

}