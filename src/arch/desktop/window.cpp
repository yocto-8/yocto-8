#include "window.hpp"
#include "devices/screenpalette.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <video/palette.hpp>

namespace arch::desktop {

// https://www.sfml-dev.org/tutorials/2.5/graphics-shader.php#minimal-shaders
constexpr const char *vertex_shader = R"(
void main()
{
    // transform the vertex position
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
}
)";

Window::Window()
	: window(sf::VideoMode(128, 128), "yocto-8", sf::Style::Titlebar) {
	window.setSize(sf::Vector2u(128 * 4, 128 * 4));
	window.setFramerateLimit(60);
	fb_texture.create(128, 128);

	filter_shader.loadFromMemory(vertex_shader, sf::Shader::Vertex);
	filter_shader.loadFromFile("4x-scale.glsl", sf::Shader::Fragment);
	pixel_grid_texture.loadFromFile("4x-filter.png");
	filter_shader.setUniform("pixelGridFilter", pixel_grid_texture);
}

void Window::present_frame(devices::Framebuffer fb,
                           devices::ScreenPalette pal) {
	button_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Left) << 0 |
	               sf::Keyboard::isKeyPressed(sf::Keyboard::Right) << 1 |
	               sf::Keyboard::isKeyPressed(sf::Keyboard::Up) << 2 |
	               sf::Keyboard::isKeyPressed(sf::Keyboard::Down) << 3 |
	               sf::Keyboard::isKeyPressed(sf::Keyboard::C) << 4 |
	               sf::Keyboard::isKeyPressed(sf::Keyboard::X) << 5;

	for (sf::Event ev; window.pollEvent(ev);) {
	}

	std::array<std::uint8_t, fb.frame_width * fb.frame_height * 4> converted_fb;

	for (std::size_t i = 0; i < fb.frame_bytes * 2; ++i) {
		const auto palette_entry = fb.get_nibble(i);
		const auto color_rgb8 =
			video::pico8_palette_rgb8[pal.get_color(palette_entry)];
		converted_fb[i * 4 + 0] = (color_rgb8 >> 16) & 0xFF;
		converted_fb[i * 4 + 1] = (color_rgb8 >> 8) & 0xFF;
		converted_fb[i * 4 + 2] = (color_rgb8 >> 0) & 0xFF;
		converted_fb[i * 4 + 3] = 255;
	}

	/*sf::View view = window.getView();
	view.setSize(128, 128);
	window.setView(view);*/

	fb_texture.update(converted_fb.data());
	sf::Sprite sprite(fb_texture);

	window.draw(sprite, &filter_shader);
	window.display();
}

Window yolo_window;

} // namespace arch::desktop