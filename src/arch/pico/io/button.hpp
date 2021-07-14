#pragma once

#include "hardware/gpio.h"

class Button
{
    public:
    Button(int pin) :
        _pin(pin)
    {
        gpio_init(_pin);
        gpio_set_dir(_pin, false);
        gpio_set_pulls(_pin, true, false);
    }

    operator bool() const
    {
        return !gpio_get(_pin);
    }

    private:
    int _pin;
};