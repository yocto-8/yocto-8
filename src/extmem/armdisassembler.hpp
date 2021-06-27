#pragma once

#include <cstdint>

namespace extmem::arm
{

struct ImmediateMemoryOp
{
    /// Base register. Its value is read and used as the address with the memory offset added to it.
    std::uint8_t rt;

    /// The source/target register (depending on the op being a store/load respectively)
    std::uint8_t rn;

    /// Memory offset that is added to the value of the base register.
    /// This should be multiplied by 2 for half-word ops and by 4 for word ops.
    std::uint32_t raw_memory_offset;

    explicit ImmediateMemoryOp(std::uint16_t instruction) :
        rt((instruction >> 3) & 0b111),
        rn(instruction & 0b111),
        raw_memory_offset((instruction >> 6) & 0b11111)
    {}
};

struct RegisterMemoryOp
{
    /// The source/target register (depending on the op being a store/load respectively)
    std::uint8_t rt;

    /// Base register. Its value is read and used as the address with the memory offset added to it.
    std::uint8_t rn;

    /// Memory offset that is added to the value of the base register.
    /// Unlike with ImmediateMemoryOp::raw_memory_offset, this value does not get multiplied.
    std::uint8_t rm;

    explicit RegisterMemoryOp(std::uint16_t instruction) :
        rt(instruction & 0b111),
        rn((instruction >> 3) & 0b111),
        rm((instruction >> 6) & 0b111)
    {}
};

struct MultipleMemoryOp
{
    /// LSB r0..r7 MSB bitfield
    std::uint8_t register_list;

    /// Base register, which is the starting address for the multiple load/store.
    std::uint8_t base_register;

    explicit MultipleMemoryOp(std::uint16_t instruction) :
        register_list(instruction & 0b1111111),
        base_register((instruction >> 8) & 0b111)
    {}
};

}