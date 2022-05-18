#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <devices/image.hpp>
#include <devices/screenpalette.hpp>

namespace arch::desktop
{

class Window
{
    public:
    Window();

    void present_frame(devices::Framebuffer fb, devices::ScreenPalette pal);

    sf::RenderWindow window;
    sf::Texture fb_texture;
    std::uint8_t button_state;
};

extern Window yolo_window;

}