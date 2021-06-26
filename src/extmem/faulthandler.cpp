#include "faulthandler.hpp"

#include <cstdio>

#include "cachedinterface.hpp"

namespace extmem
{
extern "C"
{
void isr_hardfault()
{
    asm(
        "push {lr}\n" // TODO: unnecessary

        // Push registers we want to be able to manipulate
        "mov r0, r8\n"
        "mov r1, r9\n"
        "mov r2, r10\n"
        "mov r3, r11\n"
        "push {r0-r7}\n"

        "mrs r0, msp\n"
        "bl hard_fault_handler_c\n"

        // Restore registers that may have been edited by the memory access emulator
        "pop {r0-r7}\n"
        "mov r8, r0\n"
        "mov r9, r1\n"
        "mov r10, r2\n"
        "mov r11, r3\n"

        "pop {pc}\n"
    );
}

void hard_fault_handler_c(std::uint32_t* args)
{
    auto& r4 = args[0];
    auto& r5 = args[1];
    auto& r6 = args[2];
    auto& r7 = args[3];

    auto& r8 = args[4];
    auto& r9 = args[5];
    auto& r10 = args[6];
    auto& r11 = args[7];

    auto& eh_lr = args[8];

    std::uint32_t* exception_frame = &args[9];

    auto& r0 = exception_frame[0];
    auto& r1 = exception_frame[1];
    auto& r2 = exception_frame[2];
    auto& r3 = exception_frame[3];
    auto& r12 = exception_frame[4];
    auto& lr = exception_frame[5];
    auto& pc = exception_frame[6];
    auto& psr = exception_frame[7];

    // printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr, pc, psr);

    auto*& instr_address = reinterpret_cast<const std::uint16_t*&>(pc);
    //printf("Trying to recover from fault for op @%p\n", instr_address);

    const std::uint16_t first_word = *instr_address;

    // NOTE: this is the 7 MSBs, so not strictly the opcode as per the ARMv6 ARM (6 MSBs).
    //       the reason is that some ops (e.g. LDR) makes use of those bits to perform a
    //       different memory operation entirely.
    const auto opcode = first_word >> 9;

    const auto get_low_register = [&](std::uint8_t index) -> std::uint32_t& {
        switch (index)
        {
        case 0: return r0;
        case 1: return r1;
        case 2: return r2;
        case 3: return r3;
        case 4: return r4;
        case 5: return r5;
        case 6: return r6;
        case 7: return r7;
        default: __builtin_unreachable();
        }
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
    // LDR(literal):     01001xx
    case 0b0100'100:
    case 0b0100'101:
    case 0b0100'110:
    case 0b0100'111:
    {
        todo("LDR(literal)");
        break;
    }

    // STR(register):    0101000
    case 0b0101'000:
    {
        todo("STR(register)");
        break;
    }

    // STRH(register):   0101001
    case 0b0101'001:
    {
        todo("STRH(register)");
        break;
    }

    // STRB(register):   0101010
    case 0b0101'010:
    {
        todo("STRB(register)");
        break;
    }

    // LDRSB(register):  0101011
    case 0b0101'011:
    {
        todo("LDRSB(register)");
        break;
    }

    // LDR(register):    0101100
    case 0b0101'100:
    {
        todo("LDR(register)");
        break;
    }

    // LDRH(register):   0101101
    case 0b0101'101:
    {
        todo("LDRH(register)");
        break;
    }

    // LDRB(register):   0101110
    case 0b0101'110:
    {
        todo("LDRB(register)");
        break;
    }

    // LDRSH(register):  0101111
    case 0b0101'111:
    {
        todo("LDRSH(register)");
        break;
    }

    // STR(imm):         01100xx
    case 0b0110'000:
    case 0b0110'001:
    case 0b0110'010:
    case 0b0110'011:
    {
        todo("STR(imm)");
        break;
    }

    // LDR(imm):         01101xx
    case 0b0110'100:
    case 0b0110'101:
    case 0b0110'110:
    case 0b0110'111:
    {
        const std::uint8_t addr_base_reg = (first_word >> 3) & 0b111;
        const std::uint8_t target_reg = first_word & 0b111;
        const std::uint32_t addr_offset = ((first_word >> 6) & 0b11111) * 4;

        const std::uintptr_t resolved_address = get_low_register(addr_base_reg) + addr_offset;

        assert_address_within_bank(resolved_address);

        get_low_register(target_reg) = get_temporary_access<std::uint32_t>(resolved_address);

        instr_address += 1;
        break;
    }

    // STRB(imm):        01110xx
    case 0b0111'000:
    case 0b0111'001:
    case 0b0111'010:
    case 0b0111'011:
    {
        todo("STRB(imm)");
        break;
    }

    // LDRB(imm):        01111xx
    case 0b0111'100:
    case 0b0111'101:
    case 0b0111'110:
    case 0b0111'111:
    {
        todo("LDRB(imm)");
        break;
    }

    // STRH(imm):        10000xx
    case 0b1000'000:
    case 0b1000'001:
    case 0b1000'010:
    case 0b1000'011:
    {
        todo("STRH(imm)");
        break;
    }

    // LDRH(imm):        10001xx
    case 0b1000'100:
    case 0b1000'101:
    case 0b1000'110:
    case 0b1000'111:
    {
        todo("LDRH(imm)");
        break;
    }

    // STR(imm,sprel):   10010xx
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
    }

    // STM:              11000xx
    case 0b1100'000:
    case 0b1100'001:
    case 0b1100'010:
    case 0b1100'011:
    {
        todo("STM*");
        break;
    }

    // LDM:              11001xx
    case 0b1100'100:
    case 0b1100'101:
    case 0b1100'110:
    case 0b1100'111:
    {
        todo("LDM*");
        break;
    }

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