
#include "reftest.h"
#include "../src/fplib.h"

using namespace fplib;

void displayNumber(const SFix &num)
{
    // multiply by 10
    SFix a = num;

    if (a.isNegative())
    {
        printf("-");
        a = a.negate();
    }

    // extract the integer part
    if (a.intBits() > 0)
    {
        SFix b(0,0);

        if (a.fracBits() > 0)
        {
            b = a.removeLSBs(a.fracBits());
        }
        else if (a.fracBits() < 0)
        {
            b = a.extendLSBs(-a.fracBits());
        }
        // print integer part

    }
    else
    {
        printf("0.");
    }

    // strip MSB / remove fractional part
    a = a.removeMSBs(a.intBits());

    // log2(10) = 3.32 bits
    // so each digit consumes 3.32 bits
    // we keep a running total and stop
    // producing digits when we run out of
    // preicision
    //

    int32_t org_fbits = a.fracBits();
    int32_t cnv_fbits = 0; // number of converted fractional bits
    while(a.fracBits() > 0)
    {
        SFix x8 = a.reinterpret(a.intBits()+3, a.fracBits()-3);    // mul by 8
        SFix x2 = a.reinterpret(a.intBits()+1, a.fracBits()-1);    // mul by 2
        a  = x8+x2;
        SFix digit = a.removeLSBs(a.fracBits());    // just the integer bits please!
        a = a - digit;
        if (digit.getInternalValue(0) < 10)
        {
            printf("%d", digit.getInternalValue(0));
        }
        else
        {
            printf("X");
        }

        cnv_fbits += 3;
        if (cnv_fbits > org_fbits)
        {
            break;
        }
    }
    printf("\n");
}

bool testExtend()
{
    SFix a(1,31);

    // extend a 32-bit aligned negative word
    a.setInternalValue(0,0x8A5A5A5A);
    SFix r = a.extendMSBs(11);
    if (r.toHexString() != "ffffffff8a5a5a5a")
    {
        printf("test 1\n");
        std::string s = r.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted ffffffff8a5a5a5a\n");
        return false;
    }

    SFix b(1,32);
    b.setInternalValue(0,0x5A5A5A5A);
    b.setInternalValue(1,0xFFFFFFFF);
    SFix r2 = b.extendMSBs(31);
    if (r2.toHexString() != "ffffffff5a5a5a5a")
    {
        printf("test 2\n");
        std::string s = r2.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted ffffffff5a5a5a5a\n");
        return false;
    }

    SFix r3 = b.removeLSBs(1);
    if (r3.toHexString() != "ad2d2d2d")
    {
        printf("test 3\n");
        std::string s = r3.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted ad2d2d2d\n");
        return false;
    }

    SFix c(8,0);
    c.setInternalValue(0,7);
    SFix r4 = c.extendLSBs(126);
    if (r4.toHexString() != "00000001c0000000000000000000000000000000")
    {
        printf("test 4\n");
        std::string s = r4.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 00000001c0000000000000000000000000000000\n");
        return false;
    }

    // sign-extend test a small negative number
    c.setInternalValue(0,0xFFFFFF82);
    SFix r5 = c.extendMSBs(11);
    if (r5.toHexString() != "ffffff82")
    {
        printf("test 5\n");
        std::string s = r5.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted ffffff82\n");
        return false;        
    }

    return true;
}

bool testRemove()
{
    SFix a(8,48);
    a.setInternalValue(1,0x7f003f);
    a.setInternalValue(0,0xffffffff);
    SFix r = a.removeLSBs(4+3+32-8);    // Q(8,17)
    if (r.toHexString() != "00fe007f")
    {
        printf("test 1\n");
        std::string s = r.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 00fe007f\n");
        return false;
    }

    SFix b(7,21);
    b.setInternalValue(0, 0x003c0802);
    SFix r2 = b.removeLSBs(13);
    if (r2.toHexString() != "000001e0")
    {
        printf("test 2\n");
        std::string s = r2.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 000001e0\n");
        return false;
    }    

    SFix c(1,8);
    c.setInternalValue(0,0xFFFFFF03);
    SFix r3 = c.removeMSBs(2);
    if (r3.toHexString() != "ffffffc3")
    {
        printf("test 3\n");
        std::string s = r3.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted ffffffc3\n");
        return false;
    }

    // test truncate Q(7,21) -> Q(1,8)
    SFix d(7,21);
    d.setInternalValue(0, 0x003c0802);
    SFix r4 = d.removeMSBs(6);
    r4 = r4.removeLSBs(13);
    if (r4.toHexString() != "000000e0")
    {
        printf("test 4\n");
        std::string s = r4.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 000000e0\n");
        return false;
    }

    return true;
}

bool testMul()
{
    SFix a(1,63);
    SFix b(1,63);

    // ************************************************************
    //   test max positive numbers
    // ************************************************************

    // set a to largest positive number
    a.setInternalValue(0,0xFFFFFFFF);
    a.setInternalValue(1,0x7FFFFFFF);

    // set b to largest positive number
    b.setInternalValue(0,0xFFFFFFFF);
    b.setInternalValue(1,0x7FFFFFFF);

    // check that they're indeed positive
    if (a.isNegative())
        return false;

    // check that they're indeed positive
    if (b.isNegative())
        return false;

    SFix r = a*b;
    if (r.toHexString() != "3fffffffffffffff0000000000000001")
    {
        std::string s = r.toHexString();
        printf("a,b positive\n");
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");
        return false;
    }

    // ************************************************************
    //   test max one negative argument
    // ************************************************************

    // set a to largest negative number
    a.setInternalValue(0,0x00000001);
    a.setInternalValue(1,0x80000000);

    // set b to largest positive number
    b.setInternalValue(0,0xFFFFFFFF);
    b.setInternalValue(1,0x7FFFFFFF);

    SFix r2 = a*b;
    SFix r3 = r2.negate();
    if (r3.toHexString() != "3fffffffffffffff0000000000000001")
    {
        std::string s = r3.toHexString();
        printf("a negative, b positive\n");
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");
        return false;
    }

    SFix r4 = b*a;
    SFix r5 = r4.negate();
    if (r5.toHexString() != "3fffffffffffffff0000000000000001")
    {
        std::string s = r5.toHexString();
        printf("a negative, b positive - reversed\n");
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");
        return false;
    }

    // ************************************************************
    //   test max two negative arguments
    // ************************************************************

    // set a to large negative number
    a.setInternalValue(0,0x00000001);
    a.setInternalValue(1,0x80000000);

    // set b to large negative number
    b.setInternalValue(0,0x00000001);
    b.setInternalValue(1,0x80000000);

    SFix r6 = a*b;
    if (r6.toHexString() != "3fffffffffffffff0000000000000001")
    {
        std::string s = r6.toHexString();
        printf("a negative, b negative\n");
        printf("Error: got %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");

        return false;
    }    

    return true;
}

bool testAdd()
{
    SFix a(1,63);
    SFix b(1,63);

    // a largest positive value
    a.setInternalValue(0, 0xFFFFFFFF);
    a.setInternalValue(1, 0x7FFFFFFF);

    // b largest positive value
    b.setInternalValue(0, 0xFFFFFFFF);
    b.setInternalValue(1, 0x7FFFFFFF);

    SFix r = a+b;
    if (r.toHexString() != "00000000fffffffffffffffe")
    {
        printf("test 1\n");
        std::string s = r.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted 00000000fffffffffffffffe\n");

        return false;
    }

    // a largest negative value
    a.setInternalValue(0, 0xFFFFFFFF);
    a.setInternalValue(1, 0xFFFFFFFF);

    // b largest positive value
    b.setInternalValue(0, 0xFFFFFFFF);
    b.setInternalValue(1, 0xFFFFFFFF);

    SFix r2 = a+b;
    if (r2.toHexString() != "fffffffffffffffffffffffe")
    {
        printf("test 2\n");
        std::string s = r2.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted fffffffffffffffffffffffe\n");

        return false;
    }

    return true;
}

bool testSubtract()
{
    SFix a(64+8+2,0);
    SFix b(64+8+2,0);

    a.setInternalValue(2, 0x123);
    a.setInternalValue(1, 0x456789ab);
    a.setInternalValue(0, 0xcdef0123);

    b.setInternalValue(2, 0x0000007E);
    b.setInternalValue(1, 0x47381958);
    b.setInternalValue(0, 0x37439183);

    SFix r1 = a-b;
    if (r1.toHexString() != "000000a4fe2f705396ab6fa0")
    {
        std::string s = r1.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 000000a4fe2f705396ab6fa0\n");

        return false;
    }

    SFix c(1,32);
    SFix d(1,32);
    c.setInternalValue(1, 0x00000000);
    c.setInternalValue(0, 0x0fffffff);
    d.setInternalValue(1, 0x00000001); // negative of c
    d.setInternalValue(0, 0xf0000001);

    SFix r2 = c-d;
    if (r2.toHexString() != "fffffffe1ffffffe")
    {
        std::string s = r2.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted fffffffe1ffffffe\n");

        return false;
    }

    // 
    SFix e(0,18);
    SFix f(0,18);
    e.setInternalValue(0, 0xfffee97b);
    f.setInternalValue(0, 0xfffd1948);
    SFix r3 = e-f;
    if (r3.toHexString() != "0001d033")
    {
        std::string s = r3.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 0001d033\n");

        return false;
    }    

    return true;
}

bool checkMinimumIntegerBits()
{
    SFix a(32,0);   // 32-bit integer
    a.setInternalValue(0,0x7FFFFFFF);

    int32_t bits = a.determineMinimumIntegerBits();
    if (bits != 32)
    {
        printf("test 1\n");
        printf("Error: got %d!\n", bits);
        printf("       wanted 32\n");
        return false;
    }

    a.setInternalValue(0,0);
    bits = a.determineMinimumIntegerBits();
    if (bits != 2)
    {
        printf("test 2\n");
        printf("Error: got %d!\n", bits);
        printf("       wanted 2\n");
        return false;
    }

    a.setInternalValue(0,0xFFFFFFFF);
    bits = a.determineMinimumIntegerBits();
    if (bits != 2)
    {
        printf("test 3\n");
        printf("Error: got %d!\n", bits);
        printf("       wanted 2\n");
        return false;
    }

    return true;
}

bool powerCheck()
{
    SFix a(64+8+2,0);
    a.setInternalValue(2,0x123);
    a.setInternalValue(1,0x456789ab);
    a.setInternalValue(0,0xcdef0123);
    SFix r1 = a*a;

    if (r1.toHexString() != "00014b66dc33f6acdca878385a55a1b72d5b4ac9")
    {
        printf("test 1\n");
        std::string s = r1.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 00014b66dc33f6acdca878385a55a1b72d5b4ac9\n");

        return false;
    }

    // ************************************************************
    //   test some other random number
    // ************************************************************

    SFix b(128,0);
    // 0x04104104_14514514_3cf3cf3d_3cf3cf3f
    b.setInternalValue(3,0x04104104);
    b.setInternalValue(2,0x14514514);
    b.setInternalValue(1,0x3cf3cf3d);
    b.setInternalValue(0,0x3cf3cf3f);

    SFix r2 = b*b;
    //   10 8310 51a8 26b3 2daa 7921 7b63 64f1 4f55 b867 bc46 6f11 5d75 d75e a160 f181
    //   10831051a826b32daa79217b6364f14f55b867bc466f115d75d75ea160f181
    if (r2.toHexString() != "0010831051a826b32daa79217b6364f14f55b867bc466f115d75d75ea160f181")
    {
        printf("test 2\n");
        std::string s = r2.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 0010831051a826b32daa79217b6364f14f55b867bc466f115d75d75ea160f181\n");

        return false;
    }

    return true;
}

void oneDivXTest()
{
    // iterate using:
    //
    // x = x*(2-b*x)
    //
    // where 1/b is the desired quotient.
    // Note that the starting x _must_ be
    // larger than 1 and that b*x
    // must be smaller than x

    SFix b(8,0);
    b.setInternalValue(0,14);   // 14.0

    const uint32_t intbits = 8;
    const uint32_t precision = 256;

    printf("\n");
    printf("------------------------------------------------\n");
    printf(" Calculate 1/%d using an iterative method\n", 14);
    printf("------------------------------------------------\n");

    SFix x(intbits,precision);
    x.setInternalValue((precision/32)-1,0x000010000);    // ?
    for(uint32_t i=0; i<30; i++)
    {
        //x = x*(two-b*x);
        x = x.reinterpret(x.intBits()+1, x.fracBits()-1) - x*x*b;

        //std::string s = x.toHexString();
        //printf("x1 -> %s\n", s.c_str());
        x = x.removeMSBs(x.intBits()-intbits);

        //s = x.toHexString();
        //printf("x2 -> %s\n", s.c_str());
        x = x.removeLSBs(x.fracBits()-precision);

        std::string s = x.toHexString();
        printf("x -> %s\n", s.c_str());
    }

    printf("result = 0.");
    displayNumber(x);
}

void bisectionSqrt()
{
    //
    // calculate sqrt(rootOf) using the
    // bisection method.
    // https://en.wikipedia.org/wiki/Bisection_method
    //

    const uint32_t fbits = 2048;
    SFix l(8,fbits);   // left point of the interval
    SFix r(8,fbits);   // right point of the interval
    SFix c(8,0);    // root to be found

    uint32_t rootOf = 2;
    r.setInternalValue(fbits/32,rootOf);
    c.setInternalValue(0,rootOf);

    for(uint32_t i=0; i<fbits; i++)
    {
        // create a new middle point
        SFix m = r+l;
        m = m.reinterpret(m.intBits()-1, m.fracBits()+1);   // divide by two
        m = m.removeLSBs(m.fracBits()-fbits);

        SFix m2 = m*m-c;

        if (m2.isNegative())
        {
            // if m is negative, m is below the root
            // so we can move the left point to m
            l = m;
        }
        else
        {
            // if m is negative, m is above the root
            // so we can move the right point to m
            r = m;
        }
    }
    std::string s1 = l.toHexString();
    std::string s2 = r.toHexString();

    printf("\n");
    printf("------------------------------------------------\n");
    printf(" Square root calculation using bisection method\n");
    printf("------------------------------------------------\n\n");
    //printf("Sqrt(%d):\n", rootOf);
    //printf("l -> %s / 2^%d\n", s1.c_str(), fbits);
    //printf("r -> %s / 2^%d\n", s2.c_str(), fbits);

    printf("Sqrt(2) is approximately 1.");
    displayNumber(l);
}

int main()
{
    printf("------------------------------------------------\n");
    printf(" Slow reference library tests\n");
    printf("------------------------------------------------\n\n");
    if (tests::doTests())
    {
        printf("Reference tests passed\n");
    }
    else
    {
        printf("Reference tests failed\n");
    }

    printf("\n\n------------------------------------------------\n");
    printf(" Fast 32-bit library tests\n");
    printf("------------------------------------------------\n\n");

    if (testAdd())
    {
        printf("Add test passed\n");
    }
    else
    {
        printf("Add test failed\n");
    }

    if (testSubtract())
    {
        printf("Subtract test passed\n");
    }
    else
    {
        printf("Subtract test failed\n");
    }

    if (testMul())
    {
        printf("Mul test passed\n");
    }
    else
    {
        printf("Mul test failed\n");
    }

    if (testExtend())
    {
        printf("Extend test passed\n");
    }
    else
    {
        printf("Extend test failed\n");
    }

    if (testRemove())
    {
        printf("Remove test passed\n");
    }
    else
    {
        printf("Remove test failed\n");
    }

    if (powerCheck())
    {
        printf("powerCheck test passed\n");
    }
    else
    {
        printf("powerCheck test failed\n");
    }

    if (checkMinimumIntegerBits())
    {
        printf("checkMinimumIntegerBits test passed\n");
    }
    else
    {
        printf("checkMinimumIntegerBits test failed\n");
    }

    oneDivXTest();
    bisectionSqrt();

    return 0;
}
