# yocto-8

A pico-8 console based on the Raspberry Pi Pico/RP2040 using a custom and optimized implementation.  
This does not aim to be a replacement of pico-8 and will not include any editor tools (in current plans anyway).  
This is currently in its very early stages.

yocto = pico².

# Current progress

**Pretty much nothing is done, both on the software and hardware side.**

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
- This will not be as stable as the real thing.
- You won't get the full pico-8 experience.
- Getting those PSRAM modules is a pain in the ass in the EU. ESP-PSRAM64H on aliexpress was the only good option I found which is not really a good sign.
- But hey, it's going to look cool. Hopefully. If I even have the motivation to do progress past the 3 first days. `"A" * 9999`

This is not really intended to be practical, though.

# Design challenges

I can imagine a few:
- Memory requirements are probably the largest issue. The pico-8 "CPU" "RAM" easily fits into the RP2040 SRAM, and the rest of the runtime should not be much of a problem wrt RAM requirements. Lua memory (which seems to be for the program + any lua allocs), on the other hand, can apparently reach 2MB. This is absolutely not going to fit in the RP2040 SRAM, hence the SPI RAM requirement - I am thinking of doing some sort of caching with the remaining SRAM and swap to/back from the SPI RAM as required. I do not know whether this will be fast enough.
- The above issue may have funny consequences and may require me to hack or reimplement the Lua VM. (... after some digging, it looks like pico-8 does modify Lua itself for things like its 32-bit fixed point implementation.)
- On that matter, I don't know if the Lua VM is suitable at all for such an embedded project.
- Some level of optimization will probably be required to run the pico-8 at full speed. Looking at the pico-8 "CPU" timings it seems quite feasible. Regardless, squeezing as much performance as possible is interesting in the scope of this project as power draw scales with the CPU frequency somewhat, and a lower power draw is nice when running off a battery. I have _no idea_ how the battery lifetime would turn out for, say, the 3.7V 1800mAh battery I got, but it should hopefully be good.
- I haven't thought or cared about mouse support for now.
