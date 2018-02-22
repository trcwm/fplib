/*

    FPLIB: a library providing a fixed-point datatype.

*/

#include <sstream>
#include <iostream>
#include <iomanip>
#include "fplib.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
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

#if _MSC_VER
    if (_addcarry_u32(carry_in ? 1:0, a, b, &result) != 0)
        return true;
#else
    if (__builtin_ia32_addcarryx_u32(carry_in ? 1:0, a, b, &result) != 0)
        return true;
#endif

    return false;
}


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


SFix SFix::extendMSBs(uint32_t bits) const
{
    SFix result(m_intBits+bits, m_fracBits);
    uint32_t N=result.m_data.size();

    // pre-fill the result with all bits set
    // if the number is negative so we
    // don't have to handle sign-extension later
    if (isNegative())
    {
        for(uint32_t i=0; i<N; i++)
        {
            result.m_data[i] = 0xFFFFFFFF;
        }
    }

    // directly copy 32-bit chunks from the source
    // until we run out..
    int32_t obits = m_intBits+m_fracBits;
    uint32_t idx = 0;
    while(obits >= 32)
    {
        result.m_data[idx] = m_data[idx];
        idx++;
        obits -= 32;
    }

    // patch the remaining (upper) bits of the
    // source number into the result.
    if (obits > 0)
    {
        // zero the relevant bits so we can later OR
        // the remaining upper bits into the word.
        result.m_data[idx] &= (0xFFFFFFFFUL << obits);
        result.m_data[idx] |= m_data[idx] & (0xFFFFFFFFUL >> (32-obits));
    }
    return result;
}


SFix SFix::removeLSBs(uint32_t bits) const
{
    SFix result(m_intBits, m_fracBits-bits);

    uint32_t idx = bits / 32;   // index of first word to copy
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


SFix SFix::removeMSBs(uint32_t bits) const
{
    SFix result(m_intBits-bits, m_fracBits);

    uint32_t N=result.m_data.size();
    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[i] = m_data[i];
    }

    if (isNegative())
    {
        // Make sure we set all the sign bits within the
        // top-most 32-bit word.
        uint32_t topBits = N*32 - (m_intBits-bits + m_fracBits);
        result.m_data[N-1] |= (0xFFFFFFFFUL << (31-topBits));
    }
    else
    {
        // Make sure we reset all the sign bits within the
        // top-most 32-bit word.
        uint32_t topBits = N*32 - (m_intBits-bits + m_fracBits);
        result.m_data[N-1] &= ~((0xFFFFFFFFUL << (31-topBits)));
    }
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


std::string SFix::toBinString() const
{
    std::string v;
    //const uint32_t N=m_data.size();         // number of 32-bit words

    int32_t bits = m_intBits + m_fracBits;  // total number of bits
    uint32_t idx = 0;                       // index of current 32-bit word
    uint32_t wbitsLeft = 32;                // number of bits left in word to serialize
    uint32_t wbits = m_data[idx++];         // current 32-bit word
    while(bits > 0)
    {
        if (wbitsLeft == 0)
        {
            wbits = m_data[idx++];
            wbitsLeft = 32;
        }
        if (wbits & 1)
        {
            v = "1" + v;
        }
        else
        {
            v = "0" + v;
        }
        wbits >>= 1;
        wbitsLeft--;
        bits--;
    }
    return v;
}

void SFix::internal_add(const SFix &a, const SFix &b, SFix &result) const
{
    // sanity check:
    if ((a.m_fracBits != b.m_fracBits) || (a.m_fracBits != result.m_fracBits))
    {
        std::stringstream ss;
        ss << "SFix::internal_add fractional bits not equalized!";
        throw std::runtime_error(ss.str());
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
        std::stringstream ss;
        ss << "SFix::internal_add fractional bits not equalized!";
        throw std::runtime_error(ss.str());
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
        std::stringstream ss;
        ss << "SFix::internal_sub fractional bits not equalized!";
        throw std::runtime_error(ss.str());
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

