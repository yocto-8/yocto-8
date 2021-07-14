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

## PSRAM handling

*Note: most of this is implemented and working, except for the actual memory transfers with SPI RAM, and some of the caching details.*

The pico-8 VM allows allocating up to 2MB of "Lua memory", which apparently includes heap, stack and bytecode data.  
Unfortunately, the RP2040 has a fraction of this available as SRAM, so we need a way to hook up more.

A common way on other microcontrollers (like some STM32 chips) is to hook up external SPI RAM, and to use the microcontroller-provided external memory interface. This way, external RAM appears within the same address space, meaning your program can access that memory "naturally".

If you don't have an external memory interface, the most obvious option is to write a function for every kind of memory access and replace everything in your program to use those functions. This is very unpractical, and hopelessly hard for an entire codebase like Lua.

The RP2040 does *not* have an external memory interface we can use.  
However, it *does* have a QSPI Flash interface, which maps the Flash in memory. This makes use of a 4KiB cache to speed up accesses. Unfortunately, this is only useful for Flash and does not seem to allow mapping writes, which makes it rather useless for RAM (provided you had a way to move the program from Flash to SPI RAM in the first place).

We are not screwed yet, however. On ARM Cortex-M0+ processors like the RP2040, performing a memory access outside of any mapped area causes a hard fault to be raised, and so we can gracefully handle obvious crashes, as this invokes an exception handler (or ISR).  
On a microcontroller, this could be useful to perform some cleanup, reset some external signals, etc.  
However, in our case, I figured out that you can return from the hard fault handler, and we happen to be able to manipulate the state of the program that caused the fault to occur entirely.

The very cursed idea I had is to then use some unused, unmapped memory area, and to inspect the program state to emulate the memory operation it tried to do. This way, you could tell a memory allocator to use that memory area for allocations, tell Lua to use that memory allocator, and the hard fault handler would magically emulate memory accesses performed there.  
I implemented it, and it worked.

So, what happens when a memory access occurs within that memory area?

First, a hard fault is raised. The "faulting instruction" is the instruction in our program that tried to perform a memory access in our reserved area.  
This causes some registers to be pushed to the stack before invoking an exception handler. Those few registers pushed to the stack includes the instruction pointer at time of fault.  
In other words, we can tell exactly which instruction in the program caused the crash. We can also read or write other registers the faulting instruction might have tried to manipulate.

The Raspberry Pi Pico SDK "binds" the hard fault handler to the `isr_hardfault` function which we can redefine. I wrote it as a pure assembly stub function, which writes some remaining registers to the stack and passes a handy pointer to a C++ function to all that state, `hard_fault_handler_c`, and will deal with writing back the modified state when it returns.

`hard_fault_handler_c` does most of the dirty job: It disassembles the faulting instruction to figure out what exactly it was trying to do: a memory read or a write? Of how many bytes? Reading/writing from which register? etc.  
Then, it calls some functions that perform the actual memory operation with the SPI RAM.

There is some caching going on, so the idea is to use some kilobytes of the internal RP2040 memory as cache so we do not need to actually read or write through SPI *every time* and instead preferring to move large chunks of memory at a time (which is faster, and which we can do in the background using DMA).

Performance is bad, but not as bad as it could be. With very simple test caching, it could reach over 1MHz worth of emulated memory operations for something somewhat unoptimized. It is currently unknown whether this performance is practical enough.

If it isn't as-is, I suspect that modifying the Lua VM in some hot spots to use the emulated memory accesses directly would help with performance a little, as there is significant overhead to the exception handler and to the instruction disassembly.

Also, the lower the program footprint is in main RAM, the more allocations could go to an allocator living in SRAM rather than in the cursed emulated memory area. Hopefully it could be possible to stuff enough data in there to reduce the performance penalty of going through emulations, while providing enough RAM to allow larger programs to run at all.