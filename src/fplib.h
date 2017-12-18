/*

    FPLIB: a library providing a fixed-point datatype.

    N.A. Moseley 2017
    License: T.B.D.

*/

#ifndef fplib_h
#define fplib_h

#include <stdint.h>
#include <algorithm>
#include <vector>

namespace fplib
{

/** signed fixed-point datatype class */
class SFix
{
public:

    /** create a fixed-point data type 
        @param[in] intBits the number of integer bits (may be negative).
        @param[in] fracBits the number of fractional bits (may be negative).

        Note: the total number of bits used is intBits + fracBits and must
        be greater than zero!
    */
    SFix(int32_t intBits, int32_t fracBits)
    : m_intBits(intBits), m_fracBits(fracBits)
    {
        int32_t N = intBits + fracBits;
        if (N > 0)
        {
            m_data.resize(1+((N-1)/32));
        }
    }

    /** return the number of integer bits */
    int32_t intBits() const
    {
        return m_intBits;
    }

    /** return the number of fractional bits */
    int32_t fracBits() const
    {
        return m_fracBits;
    }

    /** multiplication */
    SFix operator*(const SFix& rhs)
    {
        SFix tmp(m_intBits+rhs.m_intBits-1, m_fracBits+rhs.m_fracBits);
        internal_mul(*this, rhs, tmp);
        return tmp;
    }

    /** addition */
    SFix operator+(const SFix& rhs)
    {
        uint32_t intBits  = std::max(m_intBits, rhs.intBits())+1;
        uint32_t fracBits = std::max(m_fracBits, rhs.fracBits());

        SFix result(intBits, fracBits);

        // equalise the LSBs
        if (m_fracBits > rhs.m_fracBits)
        {
            SFix tmp = rhs.extendLSBs(m_fracBits-rhs.m_fracBits);
            internal_add(tmp, *this, result);
        }
        else if (m_fracBits < rhs.m_fracBits)
        {
            SFix tmp = extendLSBs(rhs.m_fracBits-m_fracBits);
            internal_add(tmp, rhs, result);
        }
        else
        {
            internal_add(*this, rhs, result);
        }

        return result;
    }

    /** subtraction */
    SFix operator-(const SFix& rhs)
    {
        uint32_t intBits  = std::max(m_intBits, rhs.intBits())+1;
        uint32_t fracBits = std::max(m_fracBits, rhs.fracBits());
        SFix result(intBits, fracBits);

        // equalise the LSBs
        if (m_fracBits > rhs.m_fracBits)
        {
            SFix tmp = rhs.extendLSBs(m_fracBits-rhs.m_fracBits);
            internal_sub(*this, tmp, result);
        }
        else if (m_fracBits < rhs.m_fracBits)
        {
            SFix tmp = extendLSBs(rhs.m_fracBits-m_fracBits);
            internal_sub(tmp, rhs, result);
        }
        else
        {
            internal_sub(*this, rhs, result);
        }

        return result;
    }

    /** negate a number */
    SFix negate() const;

    /** extend LSBs / fractional bits */
    SFix extendLSBs(uint32_t bits) const;

    /** extend MSBs / integer bits */
    SFix extendMSBs(uint32_t bits);

    /** remove LSBs / fractional bits */
    SFix removeLSBs(uint32_t bits);

    /** remove MSBs / integer bits */
    SFix removeMSBs(uint32_t bits);

    /** convert the fixed point number to a hex string */
    std::string toHexString() const;

    /** set the value */
    void fromHexString(const std::string &hex);

    /** check the sign bit */
    bool isNegative() const
    {
        uint32_t bit = ((m_intBits + m_fracBits - 1) % 32);
        uint32_t idx = ((m_intBits + m_fracBits - 1) / 32);
        if ((m_data[idx] >> bit) & 0x01)
        {
            return true;
        }
        return false;
    }

    /** change the Q(intBits,fracBits) qualifier to cheaply
        shift the factional point */
    SFix reinterpret(uint32_t intBits, uint32_t fracBits)
    {
        SFix result(intBits, fracBits);
        int32_t N = intBits + fracBits;
        if (N == (m_intBits + m_fracBits))
        {
            result.m_data = m_data;
        }
        else
        {
            //TODO: handle error.
        }
        return result;
    }

    void setInternalValue(uint32_t idx, uint32_t v)
    {
        m_data[idx] = v;
    }

    uint32_t getInternalValue(uint32_t idx) const
    {
        return m_data[idx];
    }

protected:
    /** add two 32-bit words with carry input and produce a result.
        also return a carry value */
    bool addUWords(uint32_t a, uint32_t b, bool carry_in, uint32_t &result) const;

    /** add a to b producing a result. */
    void internal_add(const SFix &a, const SFix &b, SFix &result) const;

    /** add a to result. */
    void internal_add(const SFix &a, bool invA, SFix &result);

    /** subtract b from a producing a result. */
    void internal_sub(const SFix &a, const SFix &b, SFix &result) const;

    /** add a to b producing a result. Note: this will only
        handle unsigned a and b correctly! post processing
        is required to compensate for negative numbers.

        when invA is true, 'a' is inverted.
        when invB is true, 'b' is inverted.
    */
    void internal_umul(const SFix &a, const SFix &b, bool invA, bool invB, SFix &result);

    /** uses internal_umul with compensation to handle signed numbers */
    void internal_mul(const SFix &a, const SFix &b, SFix &result);

    /** increment by one */
    void internal_increment(SFix &result) const;

    /** invert bits */
    void internal_invert(SFix &result) const;

    int32_t m_intBits;    // number of integer bits
    int32_t m_fracBits;   // number of fractional bits

    std::vector<uint32_t>   m_data;
};

} // end namespace

#endif
