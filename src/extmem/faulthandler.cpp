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

    /*Reads
Reads are defined as memory operations that have the semantics of a load. For ARMv6-M and Thumb these
are:
•
 LDR{S}B, LDR{S}H, LDR
•
 LDM, POP
Writes
Writes are defined as operations that have the semantics of a store. For ARMv6-M and Thumb these are:
•
 STRB, STRH, STR
•
 STM, PUSH
*/

    const std::uint16_t first_word = *instr_address;

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

    //printf("Opcode %d\n", int(opcode));

    switch (opcode)
    {
    // LDR(imm) || 2 lsb to ignore
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

       // printf("%d %d %d\n", addr_base_reg, target_reg, addr_offset);

        // TODO: JITing out the access seems like a good plan
        //       this shit is cursed enough already so

        instr_address += 1;
        break;
    }

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