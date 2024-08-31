#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <util/endian.hpp>

namespace emu {

template <std::size_t MapLength> struct MMIODevice {
	static constexpr auto map_length = MapLength;
	using View = std::span<std::uint8_t, map_length>;

	explicit constexpr MMIODevice(View data) : data(data) {}

	using ClonedArray = std::array<std::uint8_t, map_length>;

	constexpr void clone_into(ClonedArray &target) const {
		std::memcpy(target.data(), data.data(), target.size());
	}

	template <std::size_t Size>
	std::span<std::uint8_t, Size> sized_subspan(std::size_t byte_offset) const {
		return std::span<std::uint8_t, Size>(data.subspan(byte_offset, Size));
	}

	template <class T = std::uint8_t>
	util::UnalignedLEWrapper<T> get(std::uint16_t byte_offset) const {
		return {sized_subspan<sizeof(T)>(byte_offset)};
	}

	template <class T = std::uint8_t>
	T get_raw(std::uint16_t byte_offset) const {
		return T(get<T>(byte_offset));
	}

	View data;
};

struct Memory : MMIODevice<65536> {
	using MMIODevice::MMIODevice;
	static constexpr std::uint16_t map_address = 0;

	void memcpy(std::uint16_t dst, std::uint16_t src, std::size_t bytes) const {
		std::memmove(&data[dst], &data[src], bytes);
	}

	void memset(std::uint16_t dst, std::uint16_t val, std::size_t bytes) const {
		std::memset(&data[dst], val, bytes);
	}
};

} // namespace emu