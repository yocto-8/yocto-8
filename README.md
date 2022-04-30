# yocto-8

An open-source PICO-8 cartridge runner for the Raspberry Pi Pico.

[PICO-8](https://www.lexaloffle.com/pico-8.php) is a fantasy game console from Lexaloffle which allows you to create, edit, share and play small games in a virtual console.

Currently, yocto-8 does not aim to provide editor tools. What it eventually *will* be is a compatible reimplementation of PICO-8 that should be able to run most games at "near-native" performance (assuming an overclocked RP2040) and serve as a console.

yocto = pico².

# Current progress and plans

With some modifications, some demos and games run.

It is possible to build and run yocto-8 on the desktop which is currently the preferred way for implementing new API features due to facilitated debugging and allowing a faster development cycle in general. The main target remains the embedded implementation.

This is currently in an early stage: there is no user-friendly way to get this working yet and most cartridges will not work.

A significant challenge is the usage of [a hack](doc/extmem.md) to enable using of external SPI RAM which causes very significant slowdowns when hit and may compromise the usage of the RP2040 for this project, as opposed to a different microcontroller with higher amounts of user SRAM and a memory controller.  
There is a lot of room for optimization for the RAM hack routine, however.

There are (uncertain) plans to design real hardware (with a PCB and an enclosure) based on the RP2040 or a STM32 or an ESP32 because of performance and better RAM capabilities. The current silicon shortage seems to hit ST harder, though, so this may be a no-go for now. And as those are significantly more expensive, the BOM would likely be driven up, lessening the purpose of such a project.

# Limitations

- This does currently only aim to be a pico-8-compatible game runner, not an editor.
- There is currently no plan to support games that require the mouse or extra keyboard keys to function.
- No emulation of "CPU cycles" as calculated by pico-8 is planned for now: Game performance will be limited by how fast the RP2040 can run it. This is usually slower than official pico-8 anyways, but *may* be faster at some operations, namely draw calls.
- Yocto-8 uses an external SPI RAM chip to provide more Lua memory when the malloc pool limit is reached (pico-8 allows allocating up to 2MB of Lua memory). As this requires expensive emulation on the RP2040, games that heavily allocate may run *significantly* slower.

# Current prototype hardware

The project aims to make use of the following bare-minimum requirements:
- A Raspberry Pi Pico.
- A SSD1351 SPI OLED display (128x128). Using cheaper or larger LCD modules should be feasible.
- Audio support is unimplemented, but an I2S DAC/amp is considered for audio output.
- At least 2MB of SPI RAM (as "lua memory" for the pico-8 is 2MB), e.g. ESP-PSRAM64H (64mbit PSRAM). I do not know exactly how those Lua memory constraints work in Pico-8 for now, so the 8MB PSRAM IC is probably good to be on the safe side depending on how the Lua implementation turns out.
- A bunch of buttons for input.

# Should I use this over something based on a Raspberry Pi Zero (or any similar SBC)?

For now, probably not. Using a SBC-based solution would have a few advantages:
- Using a Pi Zero or a comparable board, the cost should not be *that* different.
- Official PICO-8 binaries are more complete (with editor tools) and more stable.
- An SBC is general purpose and you may be able to use game console emulators, etc. on the same console, and make use of Wi-Fi and Bluetooth peripherals.
- You won't need to get some annoying components like PSRAM modules that are a bit hard to find in the EU (but easier elsewhere).

... but this would have a few advantages:
- Instant boot-up. There is no Linux to boot so you can flip on the power and get started in less than a second.
- Lower power draw, and thus longer battery lifetime. Currently, an overclocked RP2040 draws a fair bit of power but it is possible to imagine frequency scaling per-game, better usage of sleep states and usage of an SMPS for digital core power supplies instead of the RP2040 LDO to cut MCU power draw in half (or better). The PMOLED display draws a fair amount of power when driven with a high brightness, too.
- Slightly better pricing. The RP2040 is really good value for this.
- Ultimately, if the software becomes good, this could allow a seamless experience.