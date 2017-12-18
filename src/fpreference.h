/*

    FPLIB: a library providing a fixed-point datatype.

    Reference implementation based on single binary digit
    strings. This is just for checking the fplib library
    results.

    N.A. Moseley 2017
    License: T.B.D.

*/

#ifndef fpreference_h
#define fpreference_h

#include <stdint.h>
#include <string>
#include <algorithm>
#include <vector>

namespace fplib
{

/** Slow reference implementation for library checking / running tests only,
    based on 1-bit bool vectors. The implementation is a lot simpler than
    operating directly on 32-bit words and easier to debug, therefore more managable
    to get correct. It is also a lot slower..

*/
class SFixRef
{
public:
    SFixRef(int32_t intBits, int32_t fracBits)
        : m_intBits(intBits), m_fracBits(fracBits)
    {
        m_bits.resize(intBits+fracBits);
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
    SFixRef operator*(const SFixRef& rhs)
    {
        SFixRef tmp(m_intBits+rhs.m_intBits-1, m_fracBits+rhs.m_fracBits);
        internal_mul(*this, rhs, tmp);
        return tmp;
    }

    /** addition */
    SFixRef operator+(const SFixRef& rhs)
    {
        uint32_t intBits  = std::max(m_intBits, rhs.intBits())+1;
        uint32_t fracBits = std::max(m_fracBits, rhs.fracBits());

        SFixRef result(intBits, fracBits);

        // equalise the LSBs
        if (m_fracBits > rhs.m_fracBits)
        {
            SFixRef tmp = rhs.extendLSBs(m_fracBits-rhs.m_fracBits);
            internal_add(tmp, *this, result);
        }
        else if (m_fracBits < rhs.m_fracBits)
        {
            SFixRef tmp = extendLSBs(rhs.m_fracBits-m_fracBits);
            internal_add(tmp, rhs, result);
        }
        else
        {
            internal_add(*this, rhs, result);
        }

        return result;
    }

    /** subtraction */
    SFixRef operator-(const SFixRef& rhs)
    {
        uint32_t intBits  = std::max(m_intBits, rhs.intBits())+1;
        uint32_t fracBits = std::max(m_fracBits, rhs.fracBits());
        SFixRef result(intBits, fracBits);

        // equalise the LSBs
        if (m_fracBits > rhs.m_fracBits)
        {
            SFixRef tmp = rhs.extendLSBs(m_fracBits-rhs.m_fracBits);
            internal_sub(*this, tmp, result);
        }
        else if (m_fracBits < rhs.m_fracBits)
        {
            SFixRef tmp = extendLSBs(rhs.m_fracBits-m_fracBits);
            internal_sub(tmp, rhs, result);
        }
        else
        {
            internal_sub(*this, rhs, result);
        }

        return result;
    }

    /** negate a number */
    SFixRef negate() const;

    /** extend LSBs / fractional bits and return the result */
    SFixRef extendLSBs(uint32_t bits) const;

    /** extend MSBs / integer bits */
    SFixRef extendMSBs(uint32_t bits) const;

    /** remove LSBs / fractional bits */
    SFixRef removeLSBs(uint32_t bits) const;

    /** remove MSBs / integer bits */
    SFixRef removeMSBs(uint32_t bits) const;

    /** change the Q(intBits,fracBits) qualifier to cheaply
        shift the factional point */
    SFixRef reinterpret(uint32_t intBits, uint32_t fracBits)
    {
        SFixRef result(intBits, fracBits);
        int32_t N = intBits + fracBits;
        if (N == (m_intBits + m_fracBits))
        {
            result.m_bits = m_bits;
        }
        else
        {
            //TODO: handle error.
        }
        return result;
    }

    /** check the sign bit */
    bool isNegative() const
    {
        if (m_bits.size() > 0)
            return (m_bits.back() == 1); // check sign bit
        else
            return false;
    }

    /** load number from string */
    void fromBinString(const std::string &hex);

    /** to hex string */
    std::string toBinString() const;

    /** load number from a hex string */
    bool fromHexString(const std::string &hex);

    /** convert the fixed point number to a hex string */
    std::string toHexString() const;

    /** convert the fixed-point number to a decimal string */
    std::string toDecString() const;

protected:
    /** add a to b producing a result. Note: this will only
        handle unsigned a and b correctly! post processing
        is required to compensate for negative numbers.

        when invA is true, 'a' is inverted.
        when invB is true, 'b' is inverted.
    */
    void internal_umul(const SFixRef &a, const SFixRef &b, bool invA, bool invB, SFixRef &result);


    /** uses internal_umul with compensation to handle signed numbers */
    void internal_mul(const SFixRef &a, const SFixRef &b, SFixRef &result);

    /** add a to b producing a result. */
    void internal_add(const SFixRef &a, const SFixRef &b, SFixRef &result) const;

    /** add a to result. */
    void internal_add(const SFixRef &a, bool invA, SFixRef &result);

    /** subtract b from a producing a result. */
    void internal_sub(const SFixRef &a, const SFixRef &b, SFixRef &result) const;

    void internal_increment(SFixRef &result) const;

    void internal_invert(SFixRef &result) const;

    std::vector<bool> m_bits;   // 1-bit vector containing fixed-point number,
                                // bit with the lowest index is LSB.

    int32_t m_intBits;
    int32_t m_fracBits;
};

} // namespace

#endif
