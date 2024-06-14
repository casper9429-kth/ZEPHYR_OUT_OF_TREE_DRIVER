#ifndef FLOX_LIB_BITWISE_H
#define FLOX_LIB_BITWISE_H

#include <stdint.h>
#include <stdbool.h>

/**
 *  Returns uint32_t with n:th bit set to 1, other bits are 0.
 */
uint32_t bit_uint32_t(unsigned int n)
{
    uint32_t reg = 1;
    return reg << n;
}

/**
 *  Returns n:th bit in reg as bool.
 */
bool get_bit_uint32_t(uint32_t reg, unsigned int n)
{
    return reg & bit_uint32_t(n);
}

/**
 *  n:th bit in reg are set to val.
 */
uint32_t set_bit_uint32_t(uint32_t reg, unsigned int n, bool val)
{
    uint32_t bit = bit_uint32_t(n);
    if (val) 
        return reg | bit;
    else
        return reg & ~bit;
}

/**
 *  Reg is shifted right and bits indicated by mask are returned. 
 */
uint32_t get_bits_uint32_t(uint32_t reg, uint32_t mask, unsigned int shift)
{
    reg >>= shift;
    return reg & mask;
}

/**
 *  Mask and val are shifted left, bits in reg indicated by shifted mask are set to shifted val.
 */
uint32_t set_bits_uint32_t(uint32_t reg, uint32_t mask, unsigned int shift, uint32_t val)
{
    mask <<= shift;
    val  <<= shift;
    val  &=  mask;
    reg  &= ~mask;
    return reg | val;
}

#endif // FLOX_LIB_BITWISE_H