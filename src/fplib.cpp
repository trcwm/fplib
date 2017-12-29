/*

    FPLIB: a library providing a fixed-point datatype.

*/

#include <sstream>
#include <iostream>
#include <iomanip>
#include "fplib.h"

#ifdef WIN32
#include <intrin.h>
#else
#include <adxintrin.h>
#endif

using namespace fplib;

bool SFix::addUWords(uint32_t a, uint32_t b, bool carry_in, uint32_t &result) const
{
#if 0
    bool carry_out = false;

    result = a + b;
    if (result < a)
        carry_out = true;
    if (result < b)
        carry_out = true;

    if (carry_in)
    {
        if (result == 0xFFFFFFFFUL)
        {
            carry_out = true;
        }
        result++;
    }
    return carry_out;
#endif

#if WIN32
    if (_addcarry_u32(carry_in ? 1:0, a, b, &result) != 0)
        return true;
#else
    if (__builtin_ia32_addcarryx_u32(carry_in ? 1:0, a, b, &result) != 0)
        return true;
#endif

    return false;
}

/** extend LSBs / fractional bits */
SFix SFix::extendLSBs(uint32_t bits) const
{
    SFix result(m_intBits, m_fracBits+bits);

    uint32_t addWords  = bits / 32;
    uint32_t shiftBits = bits % 32;

    uint32_t idx = addWords;    // index into data array
    const uint32_t N = m_data.size();  // number of words of original
    const uint32_t N2 = result.m_data.size();
    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[idx] |= m_data[i] << shiftBits;
        idx++;
        if (idx < N2)
        {
            result.m_data[idx] = (m_data[i] >> (32-shiftBits));
        }
    }

    return result;
}

/** extend MSBs / integer bits */
SFix SFix::extendMSBs(uint32_t bits)
{
    SFix result(m_intBits+bits, m_fracBits);
    uint32_t N=result.m_data.size();

    if (isNegative())
    {
        for(uint32_t i=0; i<N; i++)
        {
            result.m_data[i] = 0xFFFFFFFF;
        }
    }

    int32_t obits = m_intBits+m_fracBits;
    uint32_t idx = 0;
    while(obits >= 32)
    {
        result.m_data[idx] = m_data[idx];
        idx++;
        obits -= 32;
    }
    if (obits > 0)
    {
        result.m_data[idx] |= m_data[idx] & (0xFFFFFFFFUL >> (32-obits));
    }
    return result;
}

/** remove LSBs / fractional bits */
SFix SFix::removeLSBs(uint32_t bits)
{
    SFix result(m_intBits, m_fracBits-bits);

    uint32_t idx = bits / 32;
    const uint32_t shift = bits % 32;

    uint32_t N=result.m_data.size();
    uint32_t N2=m_data.size();
    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[i] = m_data[idx] >> shift;
        idx++;
        if ((shift != 0) && (idx < N2))
        {            
            result.m_data[i] |= (m_data[idx] << (32-shift));
        }
    }

    return result;
}

/** remove MSBs / integer bits */
SFix SFix::removeMSBs(uint32_t bits)
{
    SFix result(m_intBits-bits, m_fracBits);

    uint32_t N=result.m_data.size();
    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[i] = m_data[i];
    }

    // note: we don't mask off the
    // top MSB bits that re still left
    // from the original data type
    // these are ignored because
    // Q(intBits,fracBits) is still correct.

    return result;
}

SFix SFix::negate() const
{
    SFix result(m_intBits, m_fracBits);
    uint32_t N=result.m_data.size();

    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[i] = ~m_data[i];
    }
    internal_increment(result);
    return result;
}

bool SFix::addPowerOfTwo(int32_t power, bool negative)
{
    int32_t lowPower  = -m_fracBits;
    int32_t highPower = m_intBits;
    if (lowPower > power)
    {
        return false; // out of range
    }
    if (highPower < power)
    {
        return false; // out of range
    }

    SFix tmp(m_intBits, m_fracBits);
    uint32_t bitIndex  = (power+m_fracBits) % 32;
    uint32_t wordIndex = (power+m_fracBits) / 32;
    tmp.setInternalValue(wordIndex,1<<bitIndex);
    if (negative)
    {
        internal_sub(*this, tmp, *this);
    }
    else
    {
        internal_add(*this, tmp, *this);
    }
    return true;
}

void SFix::randomizeValue()
{
    const uint32_t N=m_data.size();
    for(uint32_t i=0; i<N; i++)
    {
        // the max returned by rand()
        // is implementation dependent
        // but guaranteerd to be greater
        // than 32767.
        if (RAND_MAX >= 65536)
        {
            m_data[i] = (rand() & 0xFFFF);
            m_data[i] |= (rand() & 0xFFFF)<<16;
        }
        else
        {
            m_data[i]  = (uint32_t)(rand() & 0xFF);
            m_data[i] |= (uint32_t)(rand() & 0xFF) << 8;
            m_data[i] |= (uint32_t)(rand() & 0xFF) << 16;
            m_data[i] |= (uint32_t)(rand() & 0xFF) << 24;
        }
    }

    // force the upper MSB to conform to the
    // correct sign
    uint32_t signBitIndex = (m_fracBits+m_intBits-1) % 32;
    if (isNegative())
    {
        // clear top bits
        uint32_t mask = (1UL << signBitIndex) - 1;
        m_data[N-1] &= mask;
    }
    else
    {
        // set top bits
        uint32_t mask = (1UL << signBitIndex) - 1;
        m_data[N-1] |= ~mask;
    }
}

std::string SFix::toHexString() const
{
    std::stringstream stream;
    const uint32_t N=m_data.size();
    stream << std::hex << std::setfill ('0');
    for(uint32_t i=0; i<N; i++)
    {
        stream << std::setw(sizeof(uint32_t)*2);
        stream << m_data[N-i-1];
        //if (i != (N-1)) stream << "_";
    }
    return stream.str();
}

void SFix::internal_add(const SFix &a, const SFix &b, SFix &result) const
{
    // sanity check:
    if ((a.m_fracBits != b.m_fracBits) || (a.m_fracBits != result.m_fracBits))
    {
        // internal error!
    }

    uint32_t N  = std::min(a.m_data.size(), b.m_data.size());   // #words in smallest operand
    uint32_t N2 = std::max(a.m_data.size(), b.m_data.size());   // #words in largest operand

    // add 32-bit words together until we run out of
    // words on one of the two operands a or b.
    // then continue sign-extending the operand
    // that has run out of bits...
    uint32_t idx = 0;
    bool carry = false;
    while(idx < N)
    {
        carry = addUWords(a.m_data[idx], b.m_data[idx], carry, result.m_data[idx]);
        idx++;
    }

    // were, one of the two operands might have run out of bits
    // check which one, if any
    if (a.m_data.size() == N)
    {
        uint32_t extended = a.isNegative() ? 0xFFFFFFFF : 0;
        // operand a is smaller, and should be sign extended
        while(idx < N2)
        {
            carry = addUWords(extended, b.m_data[idx], carry, result.m_data[idx]);
            idx++;
        }
    }
    else if (b.m_data.size() == N)
    {
        uint32_t extended = b.isNegative() ? 0xFFFFFFFF : 0;
        // operand a is smaller, and should be sign extended
        while(idx < N2)
        {
            carry = addUWords(a.m_data[idx], extended, carry, result.m_data[idx]);
            idx++;
        }
    }

    //finally, propagate carry to final output word
    if ((carry) && (idx < result.m_data.size()))
    {
        result.m_data[N]++;
    }
}

void SFix::internal_add(const SFix &a, bool invA, SFix &result)
{
    // sanity check:
    if (a.m_fracBits != result.m_fracBits)
    {
        // internal error!
    }

    // add 32-bit words together until we run out of bits
    uint32_t N  = a.m_data.size();
    uint32_t N2 = result.m_data.size();
    uint32_t idx = 0;
    bool carry = false;
    while(idx < N)
    {
        if (invA)
        {
            carry = addUWords(~a.m_data[idx], result.m_data[idx], carry, result.m_data[idx]);
        }
        else
        {
            carry = addUWords(a.m_data[idx], result.m_data[idx], carry, result.m_data[idx]);
        }
        idx++;
    }

    // check if result has more bits than operand a
    if (result.m_data.size() != N)
    {
        uint32_t extended = (a.isNegative() ^ invA) ? 0xFFFFFFFF : 0;
        // operand a is smaller, and should be sign extended
        while(idx < N2)
        {
            carry = addUWords(extended, result.m_data[idx], carry, result.m_data[idx]);
            idx++;
        }
    }

#if 0
    //TODO: carry propagation!
    if ((carry) && (N < result.m_data.size()))
    {
        result.m_data[N]++;
    }
#endif
}

void SFix::internal_invert(SFix &result) const
{
    const uint32_t N = result.m_data.size();
    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[i] = ~result.m_data[i];
    }
}

void SFix::internal_increment(SFix &result) const
{
    const uint32_t N = result.m_data.size();
    bool carry = true;
    uint32_t i=0;
    while(carry && (i<N))
    {
        carry = addUWords(0, result.m_data[i], carry, result.m_data[i]);
        i++;
    }
}

void SFix::internal_sub(const SFix &a, const SFix &b, SFix &result) const
{
    // sanity check:
    if ((a.m_fracBits != b.m_fracBits) || (a.m_fracBits != result.m_fracBits))
    {
        // FIXME: internal error!
    }

    SFix tmp = b.negate();
    internal_add(a,tmp,result);
}

void SFix::internal_umul(const SFix &a, const SFix &b, bool invA, bool invB, SFix &result)
{
    const uint32_t N1 = a.m_data.size();
    const uint32_t N2 = b.m_data.size();
    const uint32_t N3 = result.m_data.size();

    for(uint32_t i=0; i<N1; i++)
    {
        for(uint32_t j=0; j<N2; j++)
        {
            uint32_t op1,op2;
            if (invA)
            {
                op1 = ~a.m_data[i];
            }
            else
            {
                op1 = a.m_data[i];
            }

            if (invB)
            {
                op2 = ~b.m_data[j];
            }
            else
            {
                op2 = b.m_data[j];
            }

            uint64_t m = (uint64_t)op1*op2;
            // process first 32-bit result of partial product
            bool carry = addUWords(static_cast<uint32_t>(m), result.m_data[i+j], false, result.m_data[i+j]);
            // process second 32-bit result of partial product, if there is room in the result
            uint32_t idx = i+j+1;
            if (idx < N3)
            {
                carry = addUWords(static_cast<uint32_t>(m>>32), result.m_data[idx], carry, result.m_data[idx]);
                idx++;
            };
            // propagate carry if necessary
            while(carry & (idx < N3))
            {
                carry = addUWords(0, result.m_data[idx], carry, result.m_data[idx]);
                idx++;
            }
        }
    }
}

void SFix::internal_mul(const SFix &a, const SFix &b, SFix &result)
{
    bool finalNegate = false;
    SFix op1 = a;
    SFix op2 = b;

    if (op1.isNegative())
    {
        op1 = op1.negate();
        finalNegate = !finalNegate;
    }
    if (op2.isNegative())
    {
        op2 = op2.negate();
        finalNegate = !finalNegate;
    }

    internal_umul(op1,op2,false,false,result);
    if (finalNegate)
    {
        result = result.negate();
    }
}

