
#include "../src/fplib.h"

using namespace fplib;

bool testExtend()
{
    SFix a(1,31);

    a.setInternalValue(0,0x80000000);
    SFix r = a.extendMSBs(11);
    if (r.toHexString() != "ffffffff80000000")
    {
        std::string s = r.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted ffffffff80000000\n");
        return false;
    }

    SFix b(1,32);
    b.setInternalValue(0,0x80000000);
    b.setInternalValue(1,0x00000001);
    SFix r2 = b.extendMSBs(31);
    if (r2.toHexString() != "ffffffff80000000")
    {
        std::string s = r2.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted ffffffff80000000\n");
        return false;
    }

    SFix r3 = b.removeLSBs(1);
    if (r3.toHexString() != "c0000000")
    {
        std::string s = r3.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted c0000000\n");
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
        printf("Error: got %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");
        return false;
    }

    // ************************************************************
    //   test max one negative argument
    // ************************************************************

    // set a to largest negative number
    a.setInternalValue(0,0x00000000);
    a.setInternalValue(1,0x80000000);

    // set b to largest positive number
    b.setInternalValue(0,0xFFFFFFFF);
    b.setInternalValue(1,0x7FFFFFFF);

    SFix r2 = a*b;
    SFix r3 = r2.negate();
    if (r3.toHexString() != "3fffffffffffffff0000000000000001")
    {
        std::string s = r3.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");
        return false;
    }

    SFix r4 = b*a;
    SFix r5 = r4.negate();
    if (r5.toHexString() != "3fffffffffffffff0000000000000001")
    {
        std::string s = r5.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");
        return false;
    }

    // ************************************************************
    //   test max two negative arguments
    // ************************************************************

    // set a to largest negative number
    a.setInternalValue(0,0x00000000);
    a.setInternalValue(1,0x80000000);

    // set b to largest positive number
    b.setInternalValue(0,0x00000000);
    b.setInternalValue(1,0x80000000);

    SFix r6 = a*b;
    if (r6.toHexString() != "3fffffffffffffff0000000000000001")
    {
        std::string s = r6.toHexString();
        printf("Error: got %s\n", s.c_str());
        printf("       wanted 3fffffffffffffff0000000000000001\n");

        return false;
    }

    return true;
}


int main()
{
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

    return 0;
}
