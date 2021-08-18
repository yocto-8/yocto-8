# yocto-8

A physical, handheld pico-8 console based on the Raspberry Pi Pico/RP2040 using a custom reimplementation.

[pico-8](https://www.lexaloffle.com/pico-8.php) is a fantasy game console from Lexaloffle which allows you to create, edit, share and play small games in a virtual console.  
yocto-8 does not aim to be a perfect replacement: I only hope to make it a working *player* for most pico-8 cartridges, *eventually*. In other words, if I ever try to make this project usable for other people than me, the idea is to have a niche but cheap player for pico-8 enthusiasts to provide a fun on-the-go experience.  
Ultimately, you should be able to copy a .p8 (or .p8.png later down the road) on the yocto-8 plugged through USB and run it right away.

yocto = pico².

# Current progress

This is currently in an early stage: there is no user-friendly way to get this working yet and most cartridges will not work.

Some demos and cartridges work with some modifications, most notably, Celeste Classic runs (with no font rendering or any audio is implemented yet) at full speed even on a Pico at stock frequency.

# Limitations

- This does currently only aim to be a pico-8-compatible game runner, not an editor.
- There is currently no plan to support games that require the mouse or extra keyboard keys to function.
- No emulation of "CPU cycles" as calculated by pico-8 is planned for now: Game performance will be limited by how fast the RP2040 can run it. This is usually slower than official pico-8 anyways, but *may* be faster at some operations, namely draw calls.
- Yocto-8 uses an external SPI RAM chip to provide more Lua memory when the malloc pool limit is reached (pico-8 allows allocating up to 2MB of Lua memory). As this requires expensive emulation on the RP2040, games that heavily allocate may run *significantly* slower.

# Why?

This is an excellent question.

# Hardware requirements

The project aims to make use of the following bare-minimum requirements:
- A Raspberry Pi Pico.
- A SSD1351 SPI OLED display (128x128). Using cheaper or larger LCD modules should be feasible.
- An I2S DAC/amp with a speaker or a 3.5" jack plug, for audio support. Audio through PWM could be done, but I don't know what audio quality you'd get.
- At least 2MB of SPI RAM (as "lua memory" for the pico-8 is 2MB), e.g. ESP-PSRAM64H (64mbit PSRAM). I do not know exactly how those Lua memory constraints work in Pico-8 for now, so the 8MB PSRAM IC is probably good to be on the safe side depending on how the Lua implementation turns out.
- A bunch of buttons for input.
- You probably want a battery of some sort to power everything up - I use a cheap Li-Po battery with a lithium battery charge controller and feed this into the Pico (as it has a buck converter) and power the rest through the 3V3 output.

# Should I use this over something based on a Raspberry Pi Zero (or any similar SBC)?

Probably not, this solution is barely cheaper but may have a few differences:
- Power draw would _probably_ be better than what you would get with an SBC.
- Instant boot-up! There is no OS to load.
- This will not be as stable as the real thing. There is a lot of edge cases to handle where yocto-8 may behave slightly differently.
- You won't get the full pico-8 experience.
- This is a specific-purpose device, an SBC will allow you running emulators and such. This is really niche.
- Getting those PSRAM modules is a pain in the ass in the EU. ESP-PSRAM64H on aliexpress was the only good option I found which is not really a good sign. Mouser provides similar chips, but if you live in the EU, you would have to pack in more stuff in your order, otherwise, you will have to pay a lot for shipping a 1€ chip.
- But hey, it's going to look cool. Hopefully.

This is not really intended to be practical, though.

# Technical articles

- [How does yocto-8 use SPI RAM?](doc/extmem.md)