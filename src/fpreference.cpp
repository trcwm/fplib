/*

    FPLIB: a library providing a fixed-point datatype.

    Reference implementation based on single binary digit
    strings. This is just for checking the fplib library
    results.

    N.A. Moseley 2017
    License: T.B.D.

*/

#include <stdexcept>
#include <algorithm>
#include "fpreference.h"

using namespace fplib;

void SFixRef::internal_umul(const SFixRef &a, const SFixRef &b, bool invA, bool invB, SFixRef &result)
{
    // assumption is that both a and b are unsigned values
    // and the corrections needed for handling signed values
    // are applied after calling this functions
    //
    // result must have the size of Q(a,b) * Q(c,d) -> Q(a+c-1,b*d);

    const uint32_t N  = a.m_bits.size();
    const uint32_t N2 = b.m_bits.size();
    const uint32_t N3 = result.m_bits.size();

    if (N3 != (N+N2-1))
    {
        std::runtime_error("SFixRef::internal_mul detected incorrect size of result. This is an interal error.\n");
    }

    for(uint32_t i=0; i<N; i++)
    {
        for(uint32_t j=0; j<N2; j++)
        {
            // multiply aa by bb
            bool aa = a.m_bits[i] != invA;
            bool bb = b.m_bits[j] != invB;
            bool m = bb ? aa : false;

            if (!m) continue; // no need to do anything if resulting
                              // product term is zero!

            // add m to result and propagate carry
            bool carry = false;
            uint32_t idx = i+j;
            do
            {
                bool r = result.m_bits[idx];
                result.m_bits[idx] = (m != r) != carry;
                carry = (m && r) || (carry && (m || r));
                m = false; // add 1-bit product term only once! :)
                idx++;
            } while(carry && (idx < N3));
        }
    }
}

void SFixRef::internal_mul(const SFixRef &a, const SFixRef &b, SFixRef &result)
{
    bool Ainv = a.isNegative();
    bool Binv = b.isNegative();

    if (Ainv != Binv)
    {
        if (Ainv)
        {
            // A negative, B positive
            SFixRef aa = a.negate();
            SFixRef tmp(a.intBits() + b.intBits()-1,
                        a.fracBits() + b.fracBits());
            internal_umul(aa,b,false,false,tmp);
            result = tmp.negate();
        }
        else
        {
            // A positive, B negative
            SFixRef bb = b.negate();
            SFixRef tmp(a.intBits() + b.intBits()-1,
                        a.fracBits() + b.fracBits());
            internal_umul(a,bb,false,false,tmp);
            result = tmp.negate();
        }
    }
    else
    {
        if (Ainv)
        {
            // both A and B are negative
            SFixRef aa = a.negate();
            SFixRef bb = b.negate();
            SFixRef tmp(a.intBits() + b.intBits()-1,
                        a.fracBits() + b.fracBits());
            internal_umul(aa,bb,false,false,result);
        }
        else
        {
            internal_umul(a,b,false,false,result);
        }
    }

#if 0
    bool Ainv = a.isNegative();
    bool Binv = b.isNegative();

    internal_umul(a,b,Ainv,Binv,result);
    if (Ainv && Binv)
    {
        // ~[(~a+1)*(~b+1)]+1 = ~[~a*~b + ~b + ~a + 1]+1
        // ~[~a*~b] + ~b + ~a - 1
#if 0
        internal_invert(result);
        internal_add(a, false, result);
        internal_add(b, false, result);
        // TODO: decrement by one
#else
        internal_add(a, true, result);
        internal_add(b, true, result);
        internal_increment(result);
        internal_invert(result);
        internal_increment(result);
#endif
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
        //                     = ~[a*~b] + ~a + 1
        internal_invert(result);
        internal_add(a, true, result);
        internal_increment(result);
    }
#endif
}

void SFixRef::internal_add(const SFixRef &a, bool invA, SFixRef &result)
{
    const uint32_t N  = a.m_bits.size();
    const uint32_t N2 = result.m_bits.size();
    bool carry = false;
    uint32_t idx = 0;
    for(uint32_t i=0; i<N; i++)
    {
        bool aa = a.m_bits[idx] != invA;
        bool bb = result.m_bits[idx];
        result.m_bits[i] = (aa != bb) != carry;
        carry = (aa && bb) || (carry && (aa || bb));
        idx++;
    }

    bool aa = a.isNegative() != invA; // get sign bit so we can sign-extend
    while(carry & (idx < N2))
    {
        bool bb = result.m_bits[idx];
        result.m_bits[idx] = (aa != bb) != carry;
        carry = (aa && bb) || (carry && (aa || bb));
        idx++;
    }
}

/** add a to b producing a result. */
void SFixRef::internal_add(const SFixRef &a, const SFixRef &b, SFixRef &result) const
{
    // sanity check:
    if ((a.m_fracBits != b.m_fracBits) || (a.m_fracBits != result.m_fracBits))
    {
        // internal error!
    }

    const uint32_t N  = std::min(a.m_bits.size(), b.m_bits.size());   // #words in smallest operand
    const uint32_t N2 = std::max(a.m_bits.size(), b.m_bits.size());   // #words in largest operand
    const uint32_t N3 = result.m_bits.size();

    uint32_t idx = 0;
    bool carry = false;
    while(idx < N)
    {
        bool aa = a.m_bits[idx];
        bool bb = b.m_bits[idx];
        result.m_bits[idx] = (aa != bb) != carry;
        carry = (aa && bb) || (carry && (aa || bb));
        //carry = (aa&&bb) || (aa&&carry) || (bb&&carry);
        idx++;
    }

    // were, one of the two operands might have run out of bits
    // check which one, if any
    bool s_exta = a.isNegative();
    bool s_extb = b.isNegative();
    if (a.m_bits.size() == N)
    {
        // operand a is smaller, and should be sign extended
        while(idx < N2)
        {
            bool bb = b.m_bits[idx];
            result.m_bits[idx] = (s_exta != bb) != carry;
            carry = (s_exta && bb) || (carry && (s_exta || bb));
            idx++;
        }
    }
    else if (b.m_bits.size() == N)
    {
        // operand b is smaller, and should be sign extended
        while(idx < N2)
        {
            bool aa = a.m_bits[idx];
            result.m_bits[idx] = (aa != s_extb) != carry;
            carry = (aa && s_extb) || (carry && (aa || s_extb));
            idx++;
        }
    }

    //finally, propagate carry to final output word
    //if there is room
    if ((carry) && (idx < N3))
    {
        result.m_bits[idx] = (s_exta != s_extb) != carry;
        carry = (s_exta && s_extb) || (carry && (s_exta || s_extb));
        idx++;
    }
}

/** subtract b from a producing a result. */
void SFixRef::internal_sub(const SFixRef &a, const SFixRef &b, SFixRef &result) const
{
    // sanity check:
    if ((a.m_fracBits != b.m_fracBits) || (a.m_fracBits != result.m_fracBits))
    {
        // internal error!
    }

    SFixRef tmp = b.negate();
    internal_add(a,tmp,result);
}

/** negate a number */
SFixRef SFixRef::negate() const
{
    SFixRef result(m_intBits, m_fracBits);
    const uint32_t N=result.m_bits.size();

    for(uint32_t i=0; i<N; i++)
    {
        result.m_bits[i] = !m_bits[i];
    }
    internal_increment(result);
    return result;
}

/** increment by one */
void SFixRef::internal_increment(SFixRef &result) const
{
    const uint32_t N = result.m_bits.size();
    uint32_t idx = 0;

    bool aa = false;
    bool carry = true;
    while(carry & (idx < N))
    {
        bool bb = result.m_bits[idx];
        result.m_bits[idx] = (aa != bb) != carry;
        carry = (aa && bb) || (carry && (aa || bb));
        idx++;
    }
}

/** invert bits */
void SFixRef::internal_invert(SFixRef &result) const
{
    const uint32_t N=result.m_bits.size();

    for(uint32_t i=0; i<N; i++)
    {
        result.m_bits[i] = !result.m_bits[i];
    }
}

/** load number from string */
void SFixRef::fromBinString(const std::string &bin)
{
    auto iter = bin.rbegin();
    uint32_t idx = 0;
    const uint32_t N = m_bits.size();
    while(iter != bin.rend())
    {
        if (idx >= N) return; // safety

        m_bits[idx++] = (*iter == '1');
        iter++;
    }
}

/** to hex string */
std::string SFixRef::toBinString() const
{
    std::string hex;
    auto iter = m_bits.rbegin();
    while(iter != m_bits.rend())
    {
        hex.push_back((*iter) ? '1' : '0');
        iter++;
    }
    return hex;
}

/** extend LSBs / fractional bits and return the result */
SFixRef SFixRef::extendLSBs(uint32_t bits) const
{
    SFixRef result(m_intBits, m_fracBits+bits);
    for(uint32_t i=0; i<m_bits.size(); i++)
    {
        result.m_bits[bits + i] = m_bits[i];
    }
    return result;
}

/** extend MSBs / integer bits */
SFixRef SFixRef::extendMSBs(uint32_t bits) const
{
    SFixRef result(m_intBits+bits, m_fracBits);
    const uint32_t N = m_bits.size();

    for(uint32_t i=0; i<m_bits.size(); i++)
    {
        result.m_bits[i] = m_bits[i];
    }

    bool sext = isNegative();
    for(uint32_t i=0; i<bits; i++)
    {
        result.m_bits[N+i] = sext;
    }
    return result;
}

/** remove LSBs / fractional bits */
SFixRef SFixRef::removeLSBs(uint32_t bits) const
{
    SFixRef result(m_intBits, m_fracBits);  // first allocate full size number
    result.m_bits = m_bits;
    result.m_bits.erase(result.m_bits.begin(), result.m_bits.begin()+bits); // then erase..
    result.m_fracBits -= bits;
    return result;
}

/** remove MSBs / integer bits */
SFixRef SFixRef::removeMSBs(uint32_t bits) const
{
    SFixRef result(m_intBits-bits, m_fracBits);
    const uint32_t N = result.m_bits.size();
    for(uint32_t i=0; i<N; i++)
    {
        result.m_bits[i] = m_bits[i];
    }
    return result;
}

bool SFixRef::fromHexString(const std::string &hex)
{
    // use reverse iterator: start at LSB.
    auto iter = hex.rbegin();
    const uint32_t N = m_bits.size();
    uint32_t idx = 0;
    while(iter != hex.rend())
    {
        uint8_t nibble = 0;
        char c = *iter;
        if ((c>='0') && (c<='9'))
        {
            nibble = c - '0';
        }
        else if ((c>='A') && (c<='F'))
        {
            nibble = c - 'A' + 10;
        }
        else if ((c>='a') && (c<='f'))
        {
            nibble = c - 'a' + 10;
        }
        else
        {
            // non-hex character found!
            return false;
        }

        uint32_t count = N;
        for(uint32_t i=0; i<4; i++)
        {
            if (count > 0)
            {
                //m_bits[idx++] = (((nibble >> (3-i)) & 0x01) != 0);
                m_bits[idx++] = (((nibble >> i) & 0x01) != 0);
            }
            else
            {
                // cannot store any more bits
                return true;
            }
            count--;
        }
        iter++;
    }
    // end of hex string
    return true;
}

std::string SFixRef::toHexString() const
{
    const uint32_t N = m_bits.size();
    uint8_t nibble = 0;
    uint32_t idx = 0;
    uint32_t count = 0;
    std::string hex;
    while(idx < N)
    {
        nibble >>= 1;
        nibble |= m_bits[idx] ? 8:0;    // set the high bit in the nibble.
        count++;
        if (count == 4)
        {
            if ((nibble > 9) && (nibble < 16))
            {
                hex = static_cast<char>(nibble-10+'a') + hex;
            }
            else if (nibble < 10)
            {
                hex = static_cast<char>(nibble+'0') + hex;
            }
            else
            {
                hex = '?' + hex;
            }
            count = 0;
        }
        idx++;
    }
    return hex;
}

std::string SFixRef::toDecString() const
{
    std::string num;

#if 0
    // multiply by 10
    SFixRef a = *this;

    if (a.isNegative())
    {
        printf("-");
        a = a.negate();
    }

    // extract the integer part
    if (a.intBits() > 0)
    {
        SFixRef b(0,0);

        if (a.fracBits() > 0)
        {
            b = a.removeLSBs(a.fracBits());
        }
        else if (a.fracBits() < 0)
        {
            b = a.extendLSBs(-a.fracBits());
        }
        // process integer part

    }
    else
    {
        printf("0.");
    }
#endif

    SFixRef a = *this;

    if (a.isNegative())
    {
        printf("-");
        a = a.negate();
    }

    int32_t precisionBits3x = a.fracBits()*3;
    while(a.fracBits() > 0)
    {
        SFixRef x8 = a.reinterpret(a.intBits()+3, a.fracBits()-3);    // mul by 8
        SFixRef x2 = a.reinterpret(a.intBits()+1, a.fracBits()-1);    // mul by 2
        a  = x8+x2;
        SFixRef digit = a.removeLSBs(a.fracBits());    // just the integer bits please!
        digit = digit.removeMSBs(digit.intBits()-4);
        a = a - digit;
        num += digit.toHexString();
        precisionBits3x -= 10;
        if (precisionBits3x < 0)
        {
            break;
        }
    }
    return num;
}
