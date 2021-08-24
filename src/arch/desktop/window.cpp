#include "window.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <video/palette.hpp>

namespace arch::desktop
{

Window::Window() :
    window(sf::VideoMode(128, 128), "yocto-8")
{
    window.setFramerateLimit(60);
}

void Window::present_frame(devices::Framebuffer fb)
{
    button_state =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left) << 0
        | sf::Keyboard::isKeyPressed(sf::Keyboard::Right) << 1
        | sf::Keyboard::isKeyPressed(sf::Keyboard::Up) << 2
        | sf::Keyboard::isKeyPressed(sf::Keyboard::Down) << 3
        | sf::Keyboard::isKeyPressed(sf::Keyboard::C) << 4
        | sf::Keyboard::isKeyPressed(sf::Keyboard::X) << 5;

    for (sf::Event ev; window.pollEvent(ev);)
    {

    }

    sf::Image image;
    image.create(128, 128);
    
    for (std::size_t y = 0; y < fb.frame_height; ++y)
    {
        for (std::size_t x = 0; x < fb.frame_width; ++x)
        {
            const auto palette_entry = fb.get_pixel(x, y);
            const auto color_rgb8 = video::pico8_palette_rgb8[palette_entry];
            image.setPixel(x, y, sf::Color(
                (color_rgb8 >> 16) & 0xFF,
                (color_rgb8 >> 8) & 0xFF,
                (color_rgb8 >> 0) & 0xFF
            ));
        }
    }

    fb_texture.loadFromImage(image);

    sf::View view = window.getView();
    view.setSize(128, 128);
    window.setView(view);

    sf::Sprite sprite(fb_texture);

    window.draw(sprite);
    window.display();
}

Window yolo_window;

}