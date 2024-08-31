#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <devices/image.hpp>
#include <devices/screenpalette.hpp>

namespace arch::desktop {

class Window {
	public:
	Window();

	void present_frame(devices::Framebuffer fb, devices::ScreenPalette pal);

	sf::RenderWindow window;
	sf::Texture fb_texture;
	std::uint8_t button_state;

	sf::Shader filter_shader;
	sf::Texture pixel_grid_texture;
};

extern Window yolo_window;

} // namespace arch::desktop