#include "faulthandler.hpp"

#include <cstdio>

#include "armdisassembler.hpp"
#include "cachedinterface.hpp"

namespace extmem
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
    //printf("Trying to recover from fault for op @%p\n", instr_address);

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

    // consider this a switch where the first parameter of with() is the opcode and the second parameter is its handler
    // building a dispatch table ourselves seems more efficient than the switch-case that gcc generates
    static constexpr auto dispatch = DispatchTableHelper(&&default_case)
        .with(0b0101'000, &&op_str_reg)
        .with(0b0101'001, &&op_strh_reg)
        .with(0b0101'010, &&op_strb_reg)
        .with(0b0101'011, &&op_strsb_reg)
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
        get_temporary_ref<std::uint32_t>(get_low_register(op.rn) + get_low_register(op.rm)) = get_low_register(op.rt);

        pc += 1;
        return;
    }

    op_strh_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_temporary_ref<std::uint16_t>(get_low_register(op.rn) + get_low_register(op.rm)) = get_low_register(op.rt);

        pc += 1;
        return;
    }

    op_strb_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_temporary_ref<std::uint8_t>(get_low_register(op.rn) + get_low_register(op.rm)) = get_low_register(op.rt);

        pc += 1;
        return;
    }

    op_strsb_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::int8_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        return;
    }

    op_ldr_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::uint32_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        return;
    }

    op_ldrh_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::uint16_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        return;
    }

    op_ldrb_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::uint8_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        return;
    }

    op_ldrsh_reg:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::int16_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        return;
    }

    op_str_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_temporary_ref<std::uint32_t>(resolve_address(op, 4)) = get_low_register(op.rt);

        pc += 1;
        return;
    }

    op_ldr_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::uint32_t>(resolve_address(op, 4));

        pc += 1;
        return;
    }

    op_strb_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_temporary_ref<std::uint8_t>(resolve_address(op, 1)) = get_low_register(op.rt);

        pc += 1;
        return;
    }

    op_ldrb_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_low_register(op.rt) = std::uint32_t(get_temporary_ref<std::uint8_t>(resolve_address(op, 1)));

        pc += 1;
        return;
    }

    op_strh_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_temporary_ref<std::uint16_t>(resolve_address(op, 2)) = get_low_register(op.rt);

        pc += 1;
        return;
    }

    op_ldrh_imm:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_low_register(op.rt) = std::uint32_t(get_temporary_ref<std::uint16_t>(resolve_address(op, 2)));

        pc += 1;
        return;
    }

    op_stm:
    {
        const arm::MultipleMemoryOp op(first_word);

        std::uintptr_t current_address = get_low_register(op.base_register);

        for (int i = 0; i < 8; ++i)
        {
            if ((op.register_list >> i) & 0b1)
            {
                get_temporary_ref<std::uint32_t>(current_address) = get_low_register(i);
                current_address += 4;
            }
        }

        // writeback logic
        if (((op.register_list >> op.base_register) & 0b1) == 0)
        {
            get_low_register(op.base_register) = current_address;
        }

        pc += 1;
        return;
    }

    op_ldm:
    {
        const arm::MultipleMemoryOp op(first_word);

        std::uintptr_t current_address = get_low_register(op.base_register);

        for (int i = 0; i < 8; ++i)
        {
            if ((op.register_list >> i) & 0b1)
            {
                get_low_register(i) = get_temporary_ref<std::uint32_t>(current_address);
                current_address += 4;
            }
        }

        // writeback logic
        if (((op.register_list >> op.base_register) & 0b1) == 0)
        {
            get_low_register(op.base_register) = current_address;
        }

        pc += 1;
        return;
    }

    default_case:
    {
        printf("Hard fault handler failed to recover (opcode 0x%02x)\n", opcode);
        for (;;)
            ;
    }
}
}
}