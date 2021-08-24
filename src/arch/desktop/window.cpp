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
    fb_texture.create(128, 128);
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

    std::array<std::uint8_t, fb.frame_width * fb.frame_height * 4> converted_fb;

    for (std::size_t i = 0; i < fb.frame_bytes * 2; ++i)
    {
        const auto palette_entry = fb.get_nibble(i);
        const auto color_rgb8 = video::pico8_palette_rgb8[palette_entry];
        converted_fb[i * 4 + 0] = (color_rgb8 >> 16) & 0xFF;
        converted_fb[i * 4 + 1] = (color_rgb8 >> 8) & 0xFF;
        converted_fb[i * 4 + 2] = (color_rgb8 >> 0) & 0xFF;
        converted_fb[i * 4 + 3] = 255;
    }

    sf::View view = window.getView();
    view.setSize(128, 128);
    window.setView(view);

    fb_texture.update(converted_fb.data());
    sf::Sprite sprite(fb_texture);

    window.draw(sprite);
    window.display();
}

Window yolo_window;

}