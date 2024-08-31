#pragma once

#include "hardware/gpio.h"

namespace arch::pico::io {

class PushButton {
	public:
	void init(int pin) {
		_pin = pin;
		gpio_init(_pin);
		gpio_set_dir(_pin, GPIO_IN);
		gpio_set_pulls(_pin, true, false);
	}

	operator bool() const { return !gpio_get(_pin); }

	private:
	int _pin;
};

} // namespace arch::pico::io