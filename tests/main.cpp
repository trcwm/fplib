
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
    }
    printf("\n");
}

bool testExtend()
{
    SFix a(1,31);

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
    b.setInternalValue(1,0x00000001);
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
    if (r3.toHexString() != "3fffffffffffffff0000000000000002")
    {
        std::string s = r3.toHexString();
        printf("a negative, b positive\n");
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000002\n");
        return false;
    }

    SFix r4 = b*a;
    SFix r5 = r4.negate();
    if (r5.toHexString() != "3fffffffffffffff0000000000000002")
    {
        std::string s = r5.toHexString();
        printf("a negative, b positive - reversed\n");
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000002\n");
        return false;
    }

    // ************************************************************
    //   test max two negative arguments
    // ************************************************************

    // set a to largest negative number
    a.setInternalValue(0,0x00000001);
    a.setInternalValue(1,0x80000000);

    // set b to largest positive number
    b.setInternalValue(0,0x00000001);
    b.setInternalValue(1,0x80000000);

    SFix r6 = a*b;
    if (r6.toHexString() != "c000000000000000ffffffffffffffff")
    {
        std::string s = r6.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted c000000000000000ffffffffffffffff\n");

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
    if (r2.toHexString() != "00000000fffffffffffffffe")
    {
        printf("test 2\n");
        std::string s = r2.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted 00000000fffffffffffffffe\n");

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
    SFix b(8,0);
    SFix two(3,0);
    b.setInternalValue(0,14);   // 14.0
    two.setInternalValue(0,2);  // 2.0

    const uint32_t intbits = 8;
    const uint32_t precision = 256;

    SFix x(intbits,precision);
    x.setInternalValue((precision/32)-1,0x00000100);    // ?
    for(uint32_t i=0; i<30; i++)
    {
        //x = x*(two-b*x);
        x = x.reinterpret(x.intBits()+1, x.fracBits()-1) - x*x*b;
        x = x.removeMSBs(x.intBits()-intbits);
        x = x.removeLSBs(x.fracBits()-precision);

        std::string s = x.toHexString();
        printf("x -> %s\n", s.c_str());
    }

    displayNumber(x);
}

int main()
{
    if (tests::doTests())
    {
        printf("Reference tests passed\n");
    }
    else
    {
        printf("Reference tests failed\n");
    }


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

    if (powerCheck())
    {
        printf("powerCheck test passed\n");
    }
    else
    {
        printf("powerCheck test failed\n");
    }

    oneDivXTest();

    return 0;
}
