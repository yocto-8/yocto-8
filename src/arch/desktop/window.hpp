#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <devices/image.hpp>

namespace arch::desktop
{

class Window
{
    public:
    Window();

    void present_frame(devices::Framebuffer fb);

    sf::RenderWindow window;
    sf::Texture fb_texture;
};

extern Window yolo_window;

}