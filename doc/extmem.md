# How does yocto-8 use SPI RAM?

## Memory requirements

The pico-8 VM allows allocating up to 2MB of "Lua memory", which apparently includes heap, stack and bytecode data.  
Unfortunately, the RP2040 has a fraction of this available as SRAM, so we need a way to hook up more.

## The problem: no external RAM interface on the RP2040

A common way on other microcontrollers (like some STM32 chips) is to hook up external RAM (through SPI or another protocol), and to use the microcontroller-provided external memory interface. This way, external RAM is mapped to some area in memory, meaning your program can access that memory "naturally".  
For instance, you could have 8MiB mapped from address `0x2F000000`. The same instructions that could perform reads and writes to the internal microcontroller SRAM can also do these operations to that memory area just fine, because the ARM CPU can interact with the memory interface through its bus, and that memory interface does all the dirty work to abstract the actual memory operations.

The RP2040 does *not* have an external memory interface we can use, unfortunately.

If you don't have an external memory interface, the most obvious option is to rewrite your entire codebase to use some memory read and write functions you wrote yourself. This could be absolutely fine if you only intended to use the RAM as, say, a framebuffer or some big cache that not a lot of code needs to interact with.  
In my case, I'd essentially have to rewrite half of the Lua codebase, so I quickly gave up on that idea.

The RP2040 *does* have a QPI Flash interface, which maps the Flash in memory. This makes use of a cache to speed up accesses. Unfortunately, this is only useful for Flash and does not seem to allow mapping writes, which makes it rather useless for RAM. Plus, you'd need some way of having both RAM and Flash live on that interface, or find a way to copy the entire program from Flash to RAM to be able to swap permanently afterwards.

## The cursed solution: trap on memory accesses and emulate them!

We are not screwed yet, however. On ARM Cortex-M0+ processors (including the RP2040), performing a memory access outside of any mapped area causes a hard fault to be raised, and so we can gracefully handle obvious crashes, as it is an exception we can handle pretty much like any other.  
On a microcontroller, this could be useful to perform some cleanup, reset some external signals, etc. before hanging the chip or resetting it.  
In my case, I figured out that you can return from the hard fault handler, and we happen to be able to manipulate the state of the program that caused the fault to occur entirely.

The very cursed idea I had is to then use some unused, unmapped memory area (arbitrarily: starting from `0x2F000000`), and to inspect the program state to emulate the memory operation it tried to do. This way, you could tell a memory allocator to use that memory area for allocations, tell Lua to use that memory allocator, and the hard fault handler would magically emulate memory accesses performed there.  
I implemented it, and it worked.

## What exactly happens now?

Let's write this code as an example:

```cpp
int foo()
{
    return *((volatile int*)0x2F000020);
}
```

This function will return a 32-bit integer at `0x2F000020` - which is within the memory area we're trying to emulate. The generated assembly could be:

```arm
foo():
        @ Load the memory address we want to read from into r0.
        ldr     r0, .LCPI0_0

        @ Perform a 32-bit memory read with address r0, and store the result in r0.
        ldr     r0, [r0]

        @ Return, and note that here, r0 contains the returned value, as per the calling convention.
        bx      lr
.LCPI0_0:
        .long   788529184                       @ 0x2f000020
```

When hitting the second `ldr` op, a hard fault is raised, because `0x2F000020` is not mapped in memory.  
This causes `isr_hardfault` to be executed (as per the Pico SDK), which we override in our code.

Here is what we need to be able to do in our handler:
- Interpret the faulting instruction: What memory operation was it? What registers did it interact with?
- Being able to read the registers as they were when the faulting instruction was being executed.
- Being able to write back data to the registers when returning to normal execution.

The idea here is that `isr_hardfault` is a simple pure assembly function which passes a pointer to the register state to the `hard_fault_handler_c` function.

As the CPU enters `isr_hardfault`, it automatically pushes some registers (and restores them as we return from the exception handler), some of which we really care about:
- `r0`, `r1`, `r2` and `r3`: Those are some of the registers that may be manipulated by the faulting instruction.
- `pc`: This is the address of the faulting *instruction*. We need this, because we need to be able to tell exactly what it was trying to do so we can emulate it.

`isr_hardfault` also pushes `r4`, `r5`, `r6` and `r7`, because, luckily, in the 16-bit instruction set the RP2040 uses, all memory instructions we care about only interact with registers `r0` through `r7`.

It then calls `hard_fault_handler_c` and passes a pointer to all that stuff on the stack as a parameter. This function disassembles the faulting instruction to figure out what memory operation it has tried to do, and *finally* calls some functions that perform the actual memory operation with the SPI RAM, reading and writing values through the pointer passed by `isr_hardfault`.

There is some caching going on. It is currently very simple 1-way, 32KiB caching, but those details have not really been optimized yet.

## Performance

As of `2021-07-18`, this:

```cpp
const auto time_start = get_absolute_time();
for (int i = 0; i < 10'000'000; ++i)
{
    *(volatile int*)(0x2F000000);
}
const auto time_end = get_absolute_time();
printf("emulated 10e6/%lldµs\n", absolute_time_diff_us(time_start, time_end));
```

shows about `10e6/8920003µs`, or about `1.1MHz` 32-bit memory reads a second. Note that this is not a really realistic benchmark and hits the best case scenario where all reads are cache hits.

Depending on your expectations, this may be surprisingly much or utter garbage.

How can the real world effect of this hack be reduced?

- The memory allocator used for this heap (tinyalloc) is probably not too efficient.
- Currently, cache misses are rather expensive, because it involves a blocking page write and page read with the SPI RAM. Write DMA accesses could possibly be performed in the background, which could make cache misses up to twice as fast.
- That external RAM is currently used in SPI mode, because the RP2040 does not have an extra QPI interface we could use. It does however have PIO, which could be used to write a QPI interface, which could make cache misses up to 4 times as fast.
- The external RAM is currently driven at a low-ish frequency, due to either a software bug or dupont cables in the prototype being a limiting factor.
- In real world scenarios, the caching algorithm and tweakables could use some improvement in order to improve the cache hit rate.
- Currently, the disassembly logic is probably slower than it could be. Through some deep inspection of the generated assembly, it could be possible to improve the common code path by quite a bit, maybe even using some cursed sort of JIT.