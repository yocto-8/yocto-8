# yocto-8

A physical, handheld pico-8 console based on the Raspberry Pi Pico/RP2040 using a custom reimplementation.

[pico-8](https://www.lexaloffle.com/pico-8.php) is a fantasy game console from Lexaloffle which allows you to create, edit, share and play small games in a virtual console.  
yocto-8 does not aim to be a perfect replacement: I only hope to make it a working *player* for most pico-8 cartridges, *eventually*. In other words, if I ever try to make this project usable for other people than me, the idea is to have a niche but cheap player for pico-8 enthusiasts to provide a fun on-the-go experience.

yocto = pico².

# Current progress

**This is currently in a very early stage!!** I only have an incomplete electronics prototype at the moment and development of the actual software has barely begun.

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
- You should get something close to instant bootup! Being this specific-purpose, the bootup sequence should be significantly shorter and faster than starting up an entire Linux distro.
- This will not be as stable as the real thing.
- You won't get the full pico-8 experience.
- This is a specific-purpose device, an SBC will allow you running emulators and such. This is really niche.
- Getting those PSRAM modules is a pain in the ass in the EU. ESP-PSRAM64H on aliexpress was the only good option I found which is not really a good sign.
- But hey, it's going to look cool. Hopefully. If I even have the motivation to do progress past the 3 first days. `"A" * 9999`

This is not really intended to be practical, though.

# Design challenges

I can imagine a few:
- Memory requirements are probably the largest issue. The pico-8 "CPU" "RAM" easily fits into the RP2040 SRAM, and the rest of the runtime should not be much of a problem wrt RAM requirements. Lua memory (which seems to be for the program + any lua allocs), on the other hand, can apparently reach 2MB. This is absolutely not going to fit in the RP2040 SRAM, hence the SPI RAM requirement - I am thinking of doing some sort of caching with the remaining SRAM and swap to/back from the SPI RAM as required. I do not know whether this will be fast enough.
- Lua will need to be patched heavily to provide compat with pico-8 lua because of the above point and due to a [significant number of changes done in pico-8](https://gist.github.com/josefnpat/bfe4aaa5bbb44f572cd0).
- Some level of optimization will probably be required to run the pico-8 at full speed. Looking at the pico-8 "CPU" timings it seems quite feasible. Regardless, squeezing as much performance as possible is interesting in the scope of this project as power draw scales with the CPU frequency somewhat, and a lower power draw is nice when running off a battery. I have _no idea_ how the battery lifetime would turn out for, say, the 3.7V 1800mAh battery I got, but it should hopefully be good.
- I haven't thought or cared about mouse support for now.

# Technical articles

- [How does yocto-8 use SPI RAM?](doc/extmem.md)