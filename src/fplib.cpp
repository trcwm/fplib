/*

    FPLIB: a library providing a fixed-point datatype.

*/

#include <sstream>
#include <iostream>
#include <iomanip>
#include "fplib.h"

using namespace fplib;

bool SFix::addUWords(uint32_t a, uint32_t b, bool carry_in, uint32_t &result) const
{
    bool carry_out = false;

    result = a + b + (carry_in ? 1:0);
    if (result < a)
        carry_out = true;
    if (result < b)
        carry_out = true;        

    return carry_out;
}

/** extend LSBs / fractional bits */
SFix SFix::extendLSBs(uint32_t bits) const
{
    SFix result(m_intBits, m_fracBits+bits);

    uint32_t addWords  = bits / 32;
    uint32_t shiftBits = bits % 32;

    uint32_t idx = addWords;    // index into data array
    const uint32_t N = m_data.size();  // number of words of original
    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[idx] = m_data[i] << shiftBits;
        if (i > 0)
        {
            result.m_data[idx] |= (m_data[i-1] >> (32-shiftBits));
        }
        idx++;
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
    uint32_t shift = bits % 32;

    uint32_t N=result.m_data.size();
    uint32_t N2=m_data.size();
    for(uint32_t i=0; i<N; i++)
    {
        result.m_data[i] = m_data[i] >> shift;
        if (i+1 < N2)
        {
            result.m_data[i] |= m_data[i+1] << (32-shift);
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

std::string SFix::toHexString() const
{
    std::stringstream stream;
    const uint32_t N=m_data.size();
    stream << std::hex << std::setfill ('0');
    for(uint32_t i=0; i<N; i++)
    {
        stream << std::setw(sizeof(uint32_t)*2);
        stream << m_data[N-i-1];
    }
    return stream.str();
}

void SFix::internal_add(const SFix &a, const SFix &b, SFix &result)
{
    // sanity check:
    if ((a.m_fracBits != b.m_fracBits) || (a.m_fracBits != result.m_fracBits))
    {
        // internal error!
    }

    uint32_t N = 1+std::min(a.m_intBits, b.m_intBits)/32;
    bool carry = false;
    for(uint32_t i=0; i<N; i++)
    {
        carry = addUWords(a.m_data[i], b.m_data[i], carry, result.m_data[i]);
    }

    if ((carry) && (N < result.m_data.size()))
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

    uint32_t N = 1+std::min(a.m_intBits, result.m_intBits)/32;
    bool carry = false;
    for(uint32_t i=0; i<N; i++)
    {
        if (invA)
            carry = addUWords(~a.m_data[i], result.m_data[i], carry, result.m_data[i]);
        else
            carry = addUWords(a.m_data[i], result.m_data[i], carry, result.m_data[i]);
    }

    if ((carry) && (N < result.m_data.size()))
    {
        result.m_data[N]++;
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

void SFix::internal_sub(const SFix &a, const SFix &b, SFix &result)
{
    // sanity check:
    if ((a.m_fracBits != b.m_fracBits) || (a.m_fracBits != result.m_fracBits))
    {
        // internal error!
    }

    // Note:
    //
    // a - b = a + (~b + 1) using 2's complement
    // so by inverting B and setting the carry,
    // we can use addUWords to subtract!
    //

    uint32_t N = 1+std::min(a.m_intBits, b.m_intBits)/32;
    bool carry = false;
    for(uint32_t i=0; i<N; i++)
    {
        carry = addUWords(a.m_data[i], ~b.m_data[i], !carry, result.m_data[i]);
    }

    if ((!carry) && (N < result.m_data.size()))
    {
        result.m_data[N]--;
    }
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
    bool Ainv = a.isNegative();
    bool Binv = b.isNegative();

    internal_umul(a,b,Ainv,Binv,result);
    if (Ainv && Binv)
    {
        // ~[(~a+1)*(~b+1)]+1 = ~[~a*~b + b + a + 1]+1
        // ~[~a*~b] + ~b + ~a
        internal_add(a, false, result);
        internal_add(b, false, result);
    }
    else if (Ainv && (!Binv))
    {
        // ~[b * (~a + 1)] + 1 = ~[~a*b + b] + 1
        //                     = ~[~a*b] + ~b + 1
        internal_invert(result);
        internal_add(b, true, result);
        internal_increment(result);
    }
    else if ((!Ainv) && Binv)
    {
        // ~[a * (~b + 1)] + 1 = ~[a*~b + a] + 1
        //                     = ~[~a*b] + ~a + 1
        internal_invert(result);
        internal_add(a, true, result);
        internal_increment(result);
    }
}
