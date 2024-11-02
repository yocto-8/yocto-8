#include "window.hpp"
#include "devices/screenpalette.hpp"
#include "video/palette.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <array>

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

static const std::array<std::pair<sf::Keyboard::Scancode, int>, 6>
	scancode_to_p8{{{sf::Keyboard::Scan::Left, 0},
                    {sf::Keyboard::Scan::Right, 1},
                    {sf::Keyboard::Scan::Up, 2},
                    {sf::Keyboard::Scan::Down, 3},
                    {sf::Keyboard::Scan::C, 4},
                    {sf::Keyboard::Scan::X, 5}}};

static int get_p8_key(sf::Keyboard::Scancode code) {
	for (const auto [match, i] : scancode_to_p8) {
		if (code == match) {
			return i;
		}
	}

	return -1;
}

Window::Window()
	: window(sf::VideoMode(128, 128), "yocto-8", sf::Style::Titlebar) {
	window.setSize(sf::Vector2u(128 * 4, 128 * 4));
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
	fb_texture.create(128, 128);

	filter_shader.loadFromMemory(vertex_shader, sf::Shader::Vertex);
	filter_shader.loadFromFile("4x-scale.glsl", sf::Shader::Fragment);
	pixel_grid_texture.loadFromFile("4x-filter.png");
	filter_shader.setUniform("pixelGridFilter", pixel_grid_texture);
}

void Window::dispatch_tick_events() {
	pressed_key_mask = 0;

	for (sf::Event ev; window.pollEvent(ev);) {
		switch (ev.type) {
		case sf::Event::EventType::KeyPressed: {
			if (const auto i = get_p8_key(ev.key.scancode); i >= 0) {
				held_key_mask |= 1 << i;
				pressed_key_mask |= 1 << i;
			}
			break;
		}

		case sf::Event::EventType::KeyReleased: {
			if (const auto i = get_p8_key(ev.key.scancode); i >= 0) {
				held_key_mask &= ~(1 << i);
			}
			break;
		}

		default:
			break;
		}
	}

	// For keys that were very shortly pressed; still register them for btn for
	// one tick
	held_key_mask |= pressed_key_mask;
}

void Window::present_frame(devices::Framebuffer fb,
                           devices::ScreenPalette pal) {
	std::array<std::uint8_t, fb.frame_width * fb.frame_height * 4> converted_fb;

	for (std::size_t i = 0; i < fb.frame_bytes * 2; ++i) {
		const auto palette_entry =
			fb.get_nibble(i, emu::NibbleOrder::LSB_FIRST);
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