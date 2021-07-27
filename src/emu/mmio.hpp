#pragma once

#include <gsl/gsl>

#include <video/framebufferview.hpp>

namespace emu
{

class MMIO
{
    public:
    using View = gsl::span<std::uint8_t, 65536>;

    constexpr MMIO(View view) :
        view(view)
    {}

    constexpr gsl::span<std::uint8_t, 16> draw_palette()
    {
        return {view.subspan<0x5F00, 16>()};
    }

    constexpr video::FramebufferView frame_buffer()
    {
        return {view.subspan<0x6000, 8192>()};
    }

    /// @brief Returns a FramebufferView for the 128x128 spritesheet
    /// @details FramebufferView is reused as the framebuffer and the spritesheet have the same behavior.
    /// The lower row of the spritesheet overlaps with the lower half of the map.
    constexpr video::FramebufferView sprite_sheet()
    {
        return {view.subspan<0x0000, 8192>()};
    }

    constexpr gsl::span<std::uint8_t, 8> random_state()
    {
        return {view.subspan<0x5f44, 8>()};
    }

    constexpr std::uint8_t& button_state(std::uint8_t player_id = 0)
    {
        return static_cast<std::uint8_t&>(view[0x5F4C + player_id]);
    }

    View view;
};

}