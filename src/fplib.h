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
#include <assert.h>

namespace fplib
{

/** signed fixed-point datatype class */
class SFix
{
public:
    /** create an undefined fixed-point data type.
    */
    SFix()
        : m_intBits(0), m_fracBits(0)
    {
    }

    /** create a fixed-point data type 
        @param[in] intBits the number of integer bits (may be negative).
        @param[in] fracBits the number of fractional bits (may be negative).

        Note: the total number of bits used is intBits + fracBits and must
        be greater than zero! No checking is performed by the library.
    */
    SFix(int32_t intBits, int32_t fracBits)
        : m_intBits(intBits), m_fracBits(fracBits)
    {
        setSize(intBits, fracBits);
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


    /** Clear/zero data and set size of the type
        @param[in] intBits the number of integer bits (may be negative).
        @param[in] fracBits the number of fractional bits (may be negative).

        Note: the total number of bits used is intBits + fracBits and must
        be greater than zero! No checking is performed by the library.
    */
    void setSize(int32_t intBits, int32_t fracBits)
    {
        m_data.clear();
        int32_t N = intBits + fracBits;
        if (N > 0)
        {
            m_data.resize(1+((N-1)/32));
        }
    }

    /** copy the internal value from another SFix.
        note: the precisions must match otherwise
        a runtime_error is thrown.
    */
    void copyValueFrom(const SFix& v)
    {
        // make sure the precision specifiers are
        // the same
        if ((v.fracBits() != m_fracBits) || (v.intBits() != m_intBits))
        {
            throw std::runtime_error("SFix::copyValue error: precision does not match!\n");
        }
        m_data = v.m_data;
    }

    /** copy the internal value from another SFix.
        note: the precisions must match otherwise
        a runtime_error is thrown.
    */
    void copyValueFrom(const SFix *v)
    {
        if (v == NULL)
        {
            throw std::runtime_error("SFix::copyValue error: input is NULL!\n");
        }
        // make sure the precision specifiers are
        // the same
        if ((v->fracBits() != m_fracBits) || (v->intBits() != m_intBits))
        {
            throw std::runtime_error("SFix::copyValue error: precision does not match!\n");
        }
        m_data = v->m_data;
    }

    /** Multiplication: Q(n1,m1) * Q(n2,m2) -> Q(n1+n2-1, m1+m2) */
    SFix operator*(const SFix& rhs)
    {
        SFix tmp(m_intBits+rhs.m_intBits-1, m_fracBits+rhs.m_fracBits);
        internal_mul(*this, rhs, tmp);

        assert(tmp.isOk());

        return tmp;
    }

    /** Addition: Q(n1,m1) + Q(n2,m2) -> Q( max(n1,n2)+1, max(m1,m2) ) */
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

        assert(result.isOk());

        return result;
    }

    /** Subtraction: Q(n1,m1) - Q(n2,m2) -> Q( max(n1,n2)+1, max(m1,m2) ) */
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

        assert(result.isOk());

        return result;
    }

    /** compare two SFix numbers */
    bool operator ==(const SFix &rhs) const
    {
        if ((rhs.m_intBits != m_intBits) ||
            (rhs.m_fracBits != m_fracBits))
        {
            return false;
        }

        if (rhs.m_data.size() != m_data.size())
        {
            return false;
        }

        const uint32_t N=m_data.size();
        for(uint32_t i=0; i<N; i++)
        {
            if (rhs.m_data[i] != m_data[i])
            {
                return false;
            }
        }
        return true;
    }

    /** check if two SFix numbers are not equal */
    bool operator !=(const SFix &rhs) const
    {
        return !(*this == rhs);
    }

    /** Return the negated number */
    SFix negate() const;

    /** Extend LSBs / fractional bits */
    SFix extendLSBs(uint32_t bits) const;

    /** Extend MSBs / integer bits */
    SFix extendMSBs(uint32_t bits) const;

    /** Remove LSBs / fractional bits */
    SFix removeLSBs(uint32_t bits) const;

    /** Remove MSBs / integer bits */
    SFix removeMSBs(uint32_t bits) const;

    /** convert the fixed point number to a binary string */
    std::string toBinString() const;

    /** convert the fixed point number to a hex string.
        returns hex digits in 32-bit chunks.
    */
    std::string toHexString() const;

    /** Set the value of the fixed-point number using a hexadecimal string */
    void fromHexString(const std::string &hex);

    /** Check the sign bit */
    bool isNegative() const
    {
        if ((m_intBits == 0) && (m_fracBits == 0))
        {
            // handle uninitialized case!
            return false;
        }
        return getBitValue(m_intBits + m_fracBits - 1);
    }

    /** Change the Q(intBits,fracBits) qualifier to cheaply
        shift the factional point */
    SFix reinterpret(int32_t intBits, int32_t fracBits)
    {
        SFix result(intBits, fracBits);
        int32_t N = intBits + fracBits;
        if (N == (m_intBits + m_fracBits))
        {
            result.m_data = m_data;
        }
        else
        {
            //TODO: handle error when input and output
            //      are not of the same length
        }
        return result;
    }

    /** set one of the N internal 32-bit values.
        used for debugging. */
    void setInternalValue(uint32_t idx, uint32_t v)
    {
        m_data[idx] = v;
    }

    /** get one of the N internal 32-bit values.
        used for debugging. */
    uint32_t getInternalValue(uint32_t idx) const
    {
        return m_data[idx];
    }

    /** Add (or subtract) a power of two without affecting
        the precision of the number. This function is needed
        to support Canonical Signed Digit formats.

        note: range checking is performed. if the power is
        outside the SFix representation range, the function
        will return false.
    */
    bool addPowerOfTwo(int32_t power, bool negative);

    /** set to random value - used for fuzzing */
    void randomizeValue();

    /** Return the number of integer bits necessary to represent the
        current value. This assumes the fractional bits all necessary.
        note: this function is primarily used to determine precision
              required for the min/max values a fixed-point variable
              can attain.

        method: move towards the LSB as long as the current bit
                equals the sign bit.
    */
    int32_t determineMinimumIntegerBits() const
    {
        int32_t requiredIntBits = m_intBits;
        uint32_t bitIndex = m_intBits + m_fracBits - 1; // sign bit
        bool msb = getBitValue(bitIndex);
        if (bitIndex == 0)
        {
            return requiredIntBits;
        }
        bitIndex--;

        while((msb == getBitValue(bitIndex)) && (bitIndex != 0))
        {
            requiredIntBits--;
            bitIndex--;
        }
        return requiredIntBits;
    }

    /** check the integrity of the sign bits */
    bool inline isOk() const
    {
        uint32_t wordIdx = (m_intBits + m_fracBits - 1) / 32;
        uint32_t signBitIndex = (m_intBits + m_fracBits - 1) % 32;
        return hasEqualSignBits(signBitIndex, m_data[wordIdx]);
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

    /** get the value of a bit in the internal representation,
        given it's offset w.r.t. the LSB. */
    bool getBitValue(uint32_t offset) const
    {
        uint32_t bit = (offset % 32);
        uint32_t idx = (offset / 32);
        if ((m_data[idx] >> bit) & 0x01)
        {
            return true;
        }
        return false;
    }

    /** generate a bit mask that spans all the
        sign bits in a 32-bit word. For example,
        signBitIndex = 3: 0b1111_1111_1111_1111_1111_1111_1111_0000
    */
    uint32_t inline genSignMask(uint8_t signBitIndex) const
    {
        return 0xFFFFFFFFUL << signBitIndex;
    }

    /** check if all the sign bits are equal in value
        i.e. no overflow occurred
    */
    bool inline hasEqualSignBits(uint8_t signBitIndex, uint32_t word) const
    {
        uint32_t mask  = genSignMask(signBitIndex);
        uint32_t sbits = word & mask;
        return ((sbits == 0) || (sbits == mask));
    }

    int32_t m_intBits;      ///< number of integer bits
    int32_t m_fracBits;     ///< number of fractional bits

    std::vector<uint32_t>   m_data; ///< fixed-point value represented by 32-bit words.
};

} // end namespace

#endif
