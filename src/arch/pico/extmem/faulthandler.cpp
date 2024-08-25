#include "faulthandler.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <hardware/structs/xip_ctrl.h>
#include <RP2040.h>
#include "hardware/timer.h"
#include "pico/platform.h"
#include "pico/time.h"

#include "armdisassembler.hpp"
#include "extmem/spiram.hpp"
#include "extmem/paging.hpp"

namespace arch::pico::extmem
{
extern "C"
{
void isr_hardfault()
{
    asm(
        "push {lr}\n"

        // Push registers we want to be able to manipulate (in additions to the one that can be found on the stack)
        "push {r4-r7}\n"

        "mrs r0, msp\n"
        "bl hard_fault_handler_c\n"

        // Restore registers that may have been edited by the memory access emulator
        "pop {r4-r7}\n"

        "pop {pc}\n"
    );
}

enum RegisterOffset : std::uint8_t
{
    R4 = 0,
    R5,
    R6,
    R7,

    EH_LR = 4,

    EH_FRAME = 5,
    R0 = EH_FRAME,
    R1,
    R2,
    R3,
    R12,
    LR,
    PC,
    PSR
};

struct DispatchTableHelper
{
    using Table = std::array<void*, 128>;

    constexpr DispatchTableHelper(void* default_case) :
        table{}
    {
        for (std::size_t i = 0; i < table.size(); ++i)
        {
            table[i] = default_case;
        }
    }

    constexpr DispatchTableHelper& with(std::uint16_t index, void* value)
    {
        table[index] = value;
        return *this;
    }

    constexpr DispatchTableHelper& with_range(std::uint16_t from, std::uint16_t to_inclusive, void* value)
    {
        for (std::size_t i = from; i <= to_inclusive; ++i)
        {
            table[i] = value;
        }
        
        return *this;
    }
    
    constexpr Table done()
    {
        return table;
    }

    Table table;
};

void hard_fault_handler_c(std::uint32_t* args)
{
    const auto*& pc = reinterpret_cast<const std::uint16_t*&>(args[PC]);
    //printf("Trying to recover from fault for op @%p\n", pc);

    /*if (!is_xip_mode && std::uintptr_t(pc) >= XIP_BASE && std::uintptr_t(pc) < XIP_BASE + 0x01000000)
    {
        mpu_to_xip_mode();
        return;
    }*/

    if (get_core_num() != 0)
    {
        __breakpoint();
    }

    if (!is_in_ram_mode())
    {
        enable_ram_mode();
        // don't return: if we just exited RAM mode, we will need to map something anyway
    }

    const std::uint16_t first_word = *pc;

    // NOTE: this is the 7 MSBs, so not strictly the opcode as per the ARMv6 ARM (6 MSBs).
    //       the reason is that some ops (e.g. LDR) makes use of those bits to perform a
    //       different memory operation entirely.
    const auto opcode = first_word >> 9;

    const auto get_low_register = [&](std::size_t index) -> std::uint32_t& {
        static constexpr std::array lo_register_offsets{R0, R1, R2, R3, R4, R5, R6, R7};
        return args[lo_register_offsets[index]];
    };

    const auto resolve_address = [&](const arm::ImmediateMemoryOp& op, std::uint32_t offset_multiplier) -> std::uintptr_t {
        return get_low_register(op.rn) + op.raw_memory_offset * offset_multiplier;
    };

    const auto resolve_relative_address = [&](const arm::RegisterMemoryOp& op) {
        return get_low_register(op.rn) + get_low_register(op.rm);
    };

    // consider this a switch where the first parameter of with() is the opcode and the second parameter is its handler
    // building a dispatch table ourselves seems more efficient than the switch-case that gcc generates
    static constexpr auto dispatch = DispatchTableHelper(&&default_case)
        .with(0b0101'000, &&op_str_reg)
        .with(0b0101'001, &&op_strh_reg)
        .with(0b0101'010, &&op_strb_reg)
        .with(0b0101'011, &&op_ldrsb_reg)
        .with(0b0101'100, &&op_ldr_reg)
        .with(0b0101'101, &&op_ldrh_reg)
        .with(0b0101'110, &&op_ldrb_reg)
        .with(0b0101'111, &&op_ldrsh_reg)
        .with_range(0b0110'000, 0b0110'011, &&op_str_imm)
        .with_range(0b0110'100, 0b0110'111, &&op_ldr_imm)
        .with_range(0b0111'000, 0b0111'011, &&op_strb_imm)
        .with_range(0b0111'100, 0b0111'111, &&op_ldrb_imm)
        .with_range(0b1000'000, 0b1000'011, &&op_strh_imm)
        .with_range(0b1000'100, 0b1000'111, &&op_ldrh_imm)
        .with_range(0b1100'000, 0b1100'011, &&op_stm)
        .with_range(0b1100'100, 0b1100'111, &&op_ldm)
        .done();

    goto *dispatch[opcode];

    // TODO: implement tests for all this.
    //       debugging this is as bad as debugging random memory corruptions mind you.

    // not implemented: we do not support executing code in RAM
    // LDR(literal): 0b01001xx

    // not implemented: we do not support having the stack live in the emulated area
    // STR(imm,sprel): 0b10010xx
    // LDR(imm,sprel): 0b10011xx
    // same for PUSH/POP

    op_str_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }

    op_strh_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }

    op_strb_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }

    op_ldrsb_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }

    op_ldr_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }

    op_ldrh_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }
    
    op_ldrb_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }

    op_ldrsh_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        page_in(resolve_relative_address(op));
        return;
    }

    op_str_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        page_in(resolve_address(op, 4));
        return;
    }

    op_ldr_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        page_in(resolve_address(op, 4));
        return;
    }

    op_strb_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        page_in(resolve_address(op, 1));
        return;
    }

    op_ldrb_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        page_in(resolve_address(op, 1));
        return;
    }

    op_strh_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        page_in(resolve_address(op, 2));
        return;
    }

    op_ldrh_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        page_in(resolve_address(op, 2));
        return;
    }

    // so...
    // stm and ldm are kind of special. those are the only ops we need to emulate entirely
    // the reason for this is that they may fail across the page boundary, unlike everything else
    // there is no proper way to prevent that, as we cannot move the 16KiB boundary anyway
    // however, if STM/LDM fail mid op, it _should_ not be a problem that we emulate them all over again

    op_stm:
    {
        const arm::MultipleMemoryOp op(first_word);

        std::uintptr_t current_address = get_low_register(op.base_register);

        for (int i = 0; i < 8; ++i)
        {
            if ((op.register_list >> i) & 0b1)
            {
                if (!is_paged(current_address))
                {
                    page_in(current_address);
                }

                *reinterpret_cast<std::uint32_t*>(current_address) = get_low_register(i);
                current_address += 4;
            }
        }

        // writeback logic: STM always writes back to the base register
        get_low_register(op.base_register) = current_address;

        pc += 1;
        return;
    }

    op_ldm:
    {
        const arm::MultipleMemoryOp op(first_word);

        std::uintptr_t current_address = get_low_register(op.base_register);

        for (int i = 0; i < 8; ++i)
        {
            // note that if the base register is present in this list and it is not the lowest bit set
            // in the bitfield, the value read from memory will be undefined.
            // this does not really matter in our case.
            
            if ((op.register_list >> i) & 0b1)
            {
                if (!is_paged(current_address))
                {
                    page_in(current_address);
                }

                get_low_register(i) = *reinterpret_cast<std::uint32_t*>(current_address);
                current_address += 4;
            }
        }

        // writeback logic: LDM writes back to the base register if it is not present in the register list
        if (((op.register_list >> op.base_register) & 0b1) == 0)
        {
            get_low_register(op.base_register) = current_address;
        }

        pc += 1;
        return;
    }

    default_case: [[unlikely]]
    {
        printf("Hard fault handler failed to recover (instr 0x%04x)\n", first_word);
        __breakpoint();
        for (;;)
            ;
    }
}
}

}