# yocto-8

<img align="left" src="assets/logo-v2-readme.png">

## (WIP) high-performance PICO-8 cartridge runner for the Raspberry Pi Pico 2.

yocto-8 is a high-performance PICO-8 implementation that aims to run unmodified games.

[PICO-8](https://www.lexaloffle.com/pico-8.php) is a fantasy game console from Lexaloffle which allows you to create, edit, share and play small games in a virtual console. **This project and its developers are unaffiliated with PICO-8.**

That's the plan anyway -- see [the current progress](#plans).

The main implementation goals are:
- To reach good PICO-8 compatibility.
- To be really fast, even if that entails modding Lua.
- To minimize the dynamic memory footprint as much as possible.
    - Dynamic allocations are basically forbidden outside of Lua.
    - Dual-heap support, prioritizing a faster heap.
- To be generally portable to platforms with a high quality C++20 toolchain (it's _not great_ yet, but the plumbing is there for porting)

The current plan is not to be a PICO-8 devkit, that is, it will not provide any editor tools.  
[Support PICO-8 development, buy PICO-8 (and maybe Picotron or Voxatron)!](https://www.lexaloffle.com/pico-8.php)

yocto = pico².

<h1 id="plans">Progress and plans</h1>

With some modifications, **some** demos and games run. **Don't expect it to run much of anything,** manual intervention is often needed to get a cartridge to work (but hopefully fewer as time goes on).

- PICO-8 Lua extensions are not fully tested and complete yet.
- Many APIs are left unimplemented.
- Many APIs are not fully tested and may be incorrect.
- Audio support is completely non-existent currently.
- Filesystem support is an early WIP and completely unimplemented on pico.
- There is no UI (yet).

It is possible to build and run yocto-8 on the desktop which is currently the preferred way for implementing new API features due to facilitated debugging and allowing a faster development cycle in general. The main target remains the embedded implementation.

This is currently in an early stage: there is no user-friendly way to get this working yet and most cartridges will not work.

On the hardware side, the idea is to prototype a RP2350-based handheld.

<details>

<summary>Obsolete PSRAM segment, relevant for RP2040 (discontinued)</summary>

A significant problem is the reliance on [a hack](doc/extmem.md) that enables mapping of SPI RAM.  
At the moment, this hack is very slow and may compromise the usage of the RP2040 for a real handheld project. Many games and demos rely on using way more memory than the RP2040 SRAM could provide no matter how many optimizations are done.  
There is a lot of room for optimization for the RAM hack routine. It is currently not known what real world performance can be theoretically reached.

There are (uncertain) plans to design a real handheld. The main two contenders are the RP2040 and the ESP32-S3 (which supports QSPI RAM), but there are drawbacks to both of these.

</details>

# Limitations

- This does currently only aim to be a pico-8-compatible **game runner**, not an editor.
- There is currently no plan to support games that require the "devkit"'s mouse or extra keyboard keys.
- No emulation of "CPU cycles" as calculated by pico-8 is planned for now: Game performance will be limited by how fast the hardware can run it.

# Supported platforms

Not all platforms are regularly tested, there might be regressions, etc. etc.

<div align="center">

![](assets/picosystem.png)  
yocto-8 running [Celeste Classic](https://mattmakesgames.itch.io/celesteclassic) on the [PicoSystem](https://shop.pimoroni.com/products/picosystem).

</div>

- `Y8_ARCH=desktop`: Desktop (`y8` SFML frontend, `y8-headless` no graphics), primarily for testing
- `Y8_ARCH=pico`: Raspberry Pi Pico based platforms
    - `Y8_PLATFORM=asupico`: RP2350 M33, my setup ([Pimoroni Pico Plus 2](https://shop.pimoroni.com/products/pimoroni-pico-plus-2?variant=42092668289107) with onboard PSRAM+SSD1351 display+push buttons although not yet)
    - `Y8_PLATFORM=picosystem`: [Pimoroni PicoSystem](https://shop.pimoroni.com/products/picosystem)

> [!IMPORTANT]
> RP2040 ports for yocto-8 no longer support external PSRAM.  
> RP2350 with PSRAM is recommended.  
> Because PICO-8 emulates a Lua heap limit of 2MB; no amount of magic will allow
> all games run on a MCU with a tenth of that.

# Pros and cons against a SBC-based solution

Pros:
- Extremely fast bootup
- Fully hackable firmware
- The fuzzy warm feeling of a fully custom specific-purpose game console
- Possible to make it open hardware
- Probably lower cost
- Probably lower power

Cons:
- SBCs-based solutions are more general purpose (e.g. cheap emulation handhelds)
- SBCs are more versatile (WiFi & Bluetooth, etc)
- Will probably never be *fully* compatible with all PICO-8 games
- No editor tools, etc.
- More standard hardware