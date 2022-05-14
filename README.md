# yocto-8

An open-source PICO-8 cartridge runner for the Raspberry Pi Pico.

[PICO-8](https://www.lexaloffle.com/pico-8.php) is a fantasy game console from Lexaloffle which allows you to create, edit, share and play small games in a virtual console.

Currently, yocto-8 does not aim to provide editor tools. What it eventually *will* be is a compatible reimplementation of PICO-8 that should be able to run most games at "near-native" performance (assuming an overclocked RP2040) and serve as a gaming handheld.

The main implementation goals are:
- To reach good PICO-8 compatibility
- To be as fast as possible, and to minimize the RAM footprint as much as reasonably possible
- To be generally portable to platforms with a high quality C++20 toolchain

yocto = pico².

# Current progress and plans

With some modifications, some demos and games run. **We're far from good compatibility, especially on real hardware.**

It is possible to build and run yocto-8 on the desktop which is currently the preferred way for implementing new API features due to facilitated debugging and allowing a faster development cycle in general. The main target remains the embedded implementation.

This is currently in an early stage: there is no user-friendly way to get this working yet and most cartridges will not work.

A significant problem is the reliance on [a hack](doc/extmem.md) that enables mapping of SPI RAM.  
At the moment, this hack is very slow and may compromise the usage of the RP2040 for a real handheld project. Many games and demos rely on using way more memory than the RP2040 SRAM could provide no matter how many optimizations are done.  
There is a lot of room for optimization for the RAM hack routine. It is currently not known what real world performance can be theoretically reached.

There are (uncertain) plans to design a real handheld. The main two contenders are the RP2040 and the ESP32-S3 (which supports QSPI RAM), but there are drawbacks to both of these.

# Limitations

- This does currently only aim to be a pico-8-compatible game runner, not an editor.
- There is currently no plan to support games that require the mouse or extra keyboard keys to function.
- No emulation of "CPU cycles" as calculated by pico-8 is planned for now: Game performance will be limited by how fast the RP2040 can run it. This is usually slower than official pico-8 anyways, but *may* be faster at some operations, namely draw calls.
- Yocto-8 uses an external SPI RAM chip to provide more Lua memory when the malloc pool limit is reached (pico-8 allows allocating up to 2MB of Lua memory). As this requires expensive emulation on the RP2040, games that heavily allocate may run *significantly* slower.

# Supported platforms

- `Y8_ARCH=desktop`: Desktop (both with a SFML frontend and headless)
- `Y8_ARCH=pico`: Raspberry Pi Pico based platforms
    - `Y8_PLATFORM=asupico`: My setup (Pico+SSD1351 display+8MB PSRAM+push buttons)
    - `Y8_PLATFORM=picosystem`: [Pimoroni PicoSystem](https://shop.pimoroni.com/products/picosystem?variant=32369546985555)

# Pros and cons against a SBC-based solution

Pros:
- ~Instant bootup
- Some extra appeal: hackable firmware and allowing to support a niche specific-purpose device
- Usually lower cost
- It should be possible to drive power draw quite a bit lower

Cons:
- SBCs-based solutions are more general purpose (e.g. cheap emulation handhelds)
- SBCs are more versatile (WiFi & Bluetooth, etc)
- Will probably never be *fully* compatible with all PICO-8 games
- No editor tools, etc.
- More standard hardware