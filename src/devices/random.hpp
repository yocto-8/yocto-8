#pragma once

#include <bit>
#include <cstdint>
#include <emu/mmio.hpp>
#include <fix16.h>

// PRNG logic adapted from https://www.lexaloffle.com/bbs/?tid=51113

namespace devices {

struct Random : emu::MMIODevice<8> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x5F44;

	auto lo_ref() const { return get<std::uint32_t>(4); }
	auto hi_ref() const { return get<std::uint32_t>(0); }

	void set_seed(std::uint32_t seed) const {
		if (seed == 0) {
			lo_ref() = 0xDEADBEEF;
			hi_ref() = 0x60009755;
		} else {
			lo_ref() = seed;
			hi_ref() = seed ^ 0xBEAD29BA;
		}

		for (int i = 0; i < 32; ++i) {
			step();
		}
	}

	void step() const {
		std::uint32_t lo = lo_ref(), hi = hi_ref();

		// swap nibbles and add low
		hi = ((hi << 16) | (hi >> 16)) + lo;
		lo += hi;

		lo_ref() = lo;
		hi_ref() = hi;
	}

	fix16_t next(fix16_t range) const {
		step();

		if (range <= 0) {
			return 0;
		}

		return fix16_mod(std::bit_cast<fix16_t>(uint32_t(hi_ref())), range);
	}
};

} // namespace devices