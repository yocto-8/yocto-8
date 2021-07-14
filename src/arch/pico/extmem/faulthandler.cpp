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

        // Push registers we want to be able to manipulate
        /*"mov r0, r8\n"
        "mov r1, r9\n"
        "mov r2, r10\n"
        "mov r3, r11\n"
        "push {r0-r7}\n"*/
        "push {r4-r7}\n"

        "mrs r0, msp\n"
        "bl hard_fault_handler_c\n"

        // Restore registers that may have been edited by the memory access emulator
        /*"pop {r0-r7}\n"
        "mov r8, r0\n"
        "mov r9, r1\n"
        "mov r10, r2\n"
        "mov r11, r3\n"*/
        "pop {r4-r7}\n"

        "pop {pc}\n"
    );
}

void hard_fault_handler_c(std::uint32_t* args)
{
    enum RegisterOffset : std::uint8_t
    {
        /*R8 = 0,
        R9,
        R10,
        R11,*/

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

    // printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr, pc, psr);

    const auto*& pc = reinterpret_cast<const std::uint16_t*&>(args[PC]);
    //printf("Trying to recover from fault for op @%p\n", instr_address);

    const std::uint16_t first_word = *pc;

    // NOTE: this is the 7 MSBs, so not strictly the opcode as per the ARMv6 ARM (6 MSBs).
    //       the reason is that some ops (e.g. LDR) makes use of those bits to perform a
    //       different memory operation entirely.
    const auto opcode = first_word >> 9;

    const auto get_low_register = [&](std::size_t index) -> std::uint32_t& {
        return args[std::array{R0, R1, R2, R3, R4, R5, R6, R7}[index]];
    };

    const auto resolve_address = [&](const arm::ImmediateMemoryOp& op, std::uint32_t offset_multiplier) -> std::uintptr_t {
        return get_low_register(op.rn) + op.raw_memory_offset * offset_multiplier;
    };

    const auto todo = [](const char* str) {
        printf("Unimplemented memory op to emulate (%s)\n", str);
        for (;;)
            ;
    };

    // TODO: implement tests for all this.
    //       debugging this is as bad as debugging random memory corruptions mind you.
    
    // TODO: implement all todo() instructions here

    //printf("Opcode %d\n", int(opcode));

    switch (opcode)
    {
    // not implemented: we do not support executing code in RAM
    /*// LDR(literal):     01001xx
    case 0b0100'100:
    case 0b0100'101:
    case 0b0100'110:
    case 0b0100'111:
    {
        todo("LDR(literal)");
        break;
    }*/

    // STR(register):    0101000
    /*case 0b0101'000:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_temporary_ref<std::uint32_t>(get_low_register(op.rn) + get_low_register(op.rm)) = get_low_register(op.rt);

        pc += 1;
        break;
    }

    // STRH(register):   0101001
    case 0b0101'001:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_temporary_ref<std::uint16_t>(get_low_register(op.rn) + get_low_register(op.rm)) = get_low_register(op.rt);

        pc += 1;
        break;
    }

    // STRB(register):   0101010
    case 0b0101'010:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_temporary_ref<std::uint8_t>(get_low_register(op.rn) + get_low_register(op.rm)) = get_low_register(op.rt);

        pc += 1;
        break;
    }

    // LDRSB(register):  0101011
    case 0b0101'011:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::int8_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        break;
    }

    // LDR(register):    0101100
    case 0b0101'100:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::uint32_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        break;
    }

    // LDRH(register):   0101101
    case 0b0101'101:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::uint16_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        break;
    }

    // LDRB(register):   0101110
    case 0b0101'110:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::uint8_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        break;
    }

    // LDRSH(register):  0101111
    case 0b0101'111:
    {
        const arm::RegisterMemoryOp op(first_word);
        get_low_register(op.rt) = get_temporary_ref<std::int16_t>(get_low_register(op.rn) + get_low_register(op.rm));

        pc += 1;
        break;
    }*/

    // STR(imm):         01100xx
    case 0b0110'000:
    case 0b0110'001:
    case 0b0110'010:
    case 0b0110'011:
    {
        const arm::ImmediateMemoryOp op(first_word);
        //printf("str r%d, [r%d, #%d]\n", op.rt, op.rn, op.raw_memory_offset * 4);
        get_temporary_ref<std::uint32_t>(resolve_address(op, 4)) = get_low_register(op.rt);

        pc += 1;
        break;
    }

    // LDR(imm):         01101xx
    case 0b0110'100:
    case 0b0110'101:
    case 0b0110'110:
    case 0b0110'111:
    {
        const arm::ImmediateMemoryOp op(first_word);
        //printf("ldr r%d, [r%d, #%d]\n", op.rt, op.rn, op.raw_memory_offset * 4);
        get_low_register(op.rt) = get_temporary_ref<std::uint32_t>(resolve_address(op, 4));

        pc += 1;
        break;
    }

    // STRB(imm):        01110xx
    /*case 0b0111'000:
    case 0b0111'001:
    case 0b0111'010:
    case 0b0111'011:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_temporary_ref<std::uint8_t>(resolve_address(op, 1)) = get_low_register(op.rt);

        pc += 1;
        break;
    }

    // LDRB(imm):        01111xx
    case 0b0111'100:
    case 0b0111'101:
    case 0b0111'110:
    case 0b0111'111:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_low_register(op.rt) = std::uint32_t(get_temporary_ref<std::uint8_t>(resolve_address(op, 1)));

        pc += 1;
        break;
    }*/

    // STRH(imm):        10000xx
    /*case 0b1000'000:
    case 0b1000'001:
    case 0b1000'010:
    case 0b1000'011:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_temporary_ref<std::uint16_t>(resolve_address(op, 2)) = get_low_register(op.rt);

        pc += 1;
        break;
    }

    // LDRH(imm):        10001xx
    case 0b1000'100:
    case 0b1000'101:
    case 0b1000'110:
    case 0b1000'111:
    {
        const arm::ImmediateMemoryOp op(first_word);
        get_low_register(op.rt) = std::uint32_t(get_temporary_ref<std::uint16_t>(resolve_address(op, 2)));

        pc += 1;
        break;
    }*/

    // not implemented: we do not support having the stack live in the emulated area
    /*// STR(imm,sprel):   10010xx
    case 0b1001'000:
    case 0b1001'001:
    case 0b1001'010:
    case 0b1001'011:
    {
        todo("STR(imm,sprel)");
        break;
    }

    // LDR(imm,sprel):   10011xx
    case 0b1001'100:
    case 0b1001'101:
    case 0b1001'110:
    case 0b1001'111:
    {
        todo("LDR(imm,sprel)");
        break;
    }*/

    // STM:              11000xx
    /*case 0b1100'000:
    case 0b1100'001:
    case 0b1100'010:
    case 0b1100'011:
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

        pc += 1;
        break;
    }

    // LDM:              11001xx
    case 0b1100'100:
    case 0b1100'101:
    case 0b1100'110:
    case 0b1100'111:
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

        pc += 1;
        break;
    }*/

    // POP/PUSH: not implemented, we don't need this for now

    default:
    {
        printf("Hard fault handler failed to recover (opcode %d)\n", opcode);
        for (;;)
            ;
    }
    }
}
}
}