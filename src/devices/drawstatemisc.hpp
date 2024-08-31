#pragma once

#include <cstdint>
#include <emu/mmio.hpp>
#include <util/endian.hpp>
#include <util/point.hpp>

namespace devices {

struct DrawStateMisc : emu::MMIODevice<64> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x5F00;

	void reset() const {
		raw_pen_color() = 6; // light gray
	}

	std::uint8_t &raw_pen_color() const { return data[0x25]; }

	auto fill_pattern() const { return get<std::uint16_t>(0x31); }

	bool fill_pattern_at(std::uint8_t x, std::uint8_t y) const {
		const auto bit_index = (x & 0b11) | ((y & 0b11) << 2);
		return (fill_pattern() >> bit_index) & 0b1;
	}

	bool fill_zero_is_transparent() const { return (data[0x33] & 0b1) != 0; }

	auto camera_x() const { return get<std::int16_t>(0x28); }

	auto camera_y() const { return get<std::int16_t>(0x2A); }

	void set_camera_position(util::Point p) const {
		camera_x() = p.x;
		camera_y() = p.y;
	}

	bool is_line_endpoint_valid() const { return data[0x35] == 0; }

	void set_line_endpoint_valid(bool valid) const { data[0x35] = !valid; }

	auto line_endpoint_x() const { return get<std::int16_t>(0x3C); }

	auto line_endpoint_y() const { return get<std::int16_t>(0x3E); }

	void set_line_endpoint(util::Point p) const {
		set_line_endpoint_valid(true);
		line_endpoint_x() = p.x;
		line_endpoint_y() = p.y;
	}

	std::int8_t &text_x() const {
		return reinterpret_cast<std::int8_t &>(data[0x26]);
	}

	std::int8_t &text_y() const {
		return reinterpret_cast<std::int8_t &>(data[0x27]);
	}

	util::Point get_text_point() const { return {text_x(), text_y()}; }

	void set_text_point(util::Point p) const {
		text_x() = std::int8_t(p.x);
		text_y() = std::int8_t(p.y);
	}
};

} // namespace devices