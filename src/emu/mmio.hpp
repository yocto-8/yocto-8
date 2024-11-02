#pragma once

#include "coredefs.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <util/endian.hpp>

namespace emu {

using namespace y8;

enum class NibbleOrder {
	LSB_FIRST = 0,
	MSB_FIRST = 1,
};

template <std::size_t MapLength> struct MMIODevice {
	static constexpr auto map_length = MapLength;
	using View = std::span<std::uint8_t, map_length>;
	using ClonedArray = std::array<std::uint8_t, map_length>;

	explicit constexpr MMIODevice(View view) : raw_view(view) {}

	constexpr void clone_into(ClonedArray &target) const {
		std::memcpy(target.data(), raw_view.data(), target.size());
	}

	template <std::size_t Size>
	constexpr std::span<u8, Size> sized_subspan(std::size_t byte_offset) const {
		return std::span<u8, Size>{raw_view.subspan(byte_offset, Size)};
	}

	template <class T = u8>
	util::UnalignedLEWrapper<T> get(std::size_t byte_offset) const {
		return {sized_subspan<sizeof(T)>(byte_offset)};
	}

	template <class T = u8> T get_raw(std::size_t byte_offset) const {
		return T(get<T>(byte_offset));
	}

	u8 &get_byte(std::size_t byte_offset) const {
		return raw_view[byte_offset];
	}

	void fill(u8 value, std::size_t offset = 0,
	          std::size_t bytes = map_length) const {
		std::memset(raw_view.data() + offset, value, bytes);
	}

	void set_nibble(std::size_t nibble_index, std::uint8_t value,
	                NibbleOrder nibble_order) const {
		std::uint8_t &pixel_pair_byte = get_byte(nibble_index / 2);

		// even nibble: lower nibble if lsb_first, else upper nibble
		if (nibble_index % 2 == (nibble_order == NibbleOrder::MSB_FIRST)) {
			pixel_pair_byte &= 0xF0;         // clear lower nibble
			pixel_pair_byte |= value & 0x0F; // set lower nibble
		} else {
			pixel_pair_byte &= 0x0F;       // clear upper nibble
			pixel_pair_byte |= value << 4; // set upper nibble
		}
	}

	std::uint8_t get_nibble(std::size_t i, NibbleOrder nibble_order) const {
		std::uint8_t pixel_pair_byte = get_byte(i / 2);

		return (i % 2 == (nibble_order == NibbleOrder::MSB_FIRST))
		           ? (pixel_pair_byte & 0x0F)
		           : (pixel_pair_byte >> 4);
	}

	View raw_view;
};

struct Memory : MMIODevice<65536> {
	using MMIODevice::MMIODevice;
	static constexpr std::uint16_t map_address = 0;

	void memcpy(std::uint16_t dst, std::uint16_t src, std::size_t bytes) const {
		std::memmove(raw_view.data() + dst, raw_view.data() + src, bytes);
	}

	template <class Device>
	constexpr Device
	device(PicoAddr base_addr = Device::default_map_address) const {
		return Device{sized_subspan<Device::map_length>(base_addr)};
	}
};

} // namespace emu