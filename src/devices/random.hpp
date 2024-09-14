#pragma once

#include <cstdint>
#include <emu/mmio.hpp>
#include <fix16.h>

// PRNG logic adapted from FAKE-08

namespace devices {

struct Random : emu::MMIODevice<8> {
	using MMIODevice::MMIODevice;

	static constexpr std::uint16_t default_map_address = 0x5F44;

	auto rng_state() const { return get<std::uint64_t>(0x00); }

	void set_seed(std::uint32_t seed) const {
		get<std::uint32_t>(0) = seed != 0 ? seed : 0xDEADBEEF;
		get<std::uint32_t>(4) = get<std::uint32_t>(0) ^ 0xBEAD29BA;

		for (int i = 0; i < 32; ++i) {
			step();
		}
	}

	void step() const {
		get<std::uint32_t>(0) =
			get_raw<std::uint16_t>(0) << 16 | get_raw<std::uint16_t>(2);
		get<std::uint32_t>(0) = get<std::uint32_t>(0) + get<std::uint32_t>(4);
		get<std::uint32_t>(4) = get<std::uint32_t>(0) + get<std::uint32_t>(4);
	}

	fix16_t next(fix16_t range) const {
		step();

		if (range <= 0) {
			return 0;
		}

		return fix16_mod(get<fix16_t>(4), range);
	}
};

} // namespace devices