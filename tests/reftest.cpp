
#include "reftest.h"
#include "../src/fpreference.h"

using namespace fplib;

namespace tests
{

bool binTest()
{
    SFixRef a(1,15);
    a.fromBinString("1010101010101010");
    if (a.toBinString() != "1010101010101010")
    {
        printf("binTest 1 failed\n");
        printf("  got:    %s\n", a.toBinString().c_str());
        printf("  wanted: 1010101010101010\n");
        return false;
    }
    return true;
}

bool addTest()
{
    SFixRef a(1,15);
    SFixRef b(1,15);

    a.fromBinString("0111111111111111");    // max +
    b.fromBinString("0111111111111111");    // max +
    SFixRef c = a+b;
    if (c.toBinString() != "01111111111111110")
    {
        printf("addTest 1 failed\n");
        printf("  got:    %s\n", c.toBinString().c_str());
        printf("  wanted: 01111111111111110\n");
        return false;
    }

#if 0
    printf(" a = %s\n", a.toBinString().c_str());
    printf(" b = %s\n", b.toBinString().c_str());
    printf(" c = %s\n", c.toBinString().c_str());
#endif

    SFixRef d(1,15);
    d.fromBinString("1111111111111111");    // -1
    SFixRef e=a+d;
    if (e.toBinString() != "00111111111111110")
    {
        printf("addTest 2 failed\n");
        printf("  got:    %s\n", e.toBinString().c_str());
        printf("  wanted: 00111111111111110\n");
        return false;
    }

    return true;
}

bool mulTest()
{
    SFixRef a(1,7);
    SFixRef b(1,7);

    a.fromBinString("00000001");
    b.fromBinString("01111111");

    SFixRef c = a*b;
#if 0
    printf(" a = %s\n", a.toBinString().c_str());
    printf(" b = %s\n", b.toBinString().c_str());
    printf(" c = %s\n", c.toBinString().c_str());
#endif

    if (c.toBinString() != "000000001111111")
    {
        printf("mulTest 1 failed\n");
        printf("  got:    %s\n", c.toBinString().c_str());
        printf("  wanted: 000000001111111\n");
        return false;
    }

    SFixRef d(1,7);
    d.fromBinString("01010101");
    SFixRef e = b*d;
    if (e.toBinString() != "010101000101011")
    {
        printf("mulTest 2 failed\n");
        printf("  got:    %s\n", e.toBinString().c_str());
        printf("  wanted: 010101000101011\n");
        return false;
    }

    a.fromBinString("00000001");    // 1
    b.fromBinString("11111111");    // -1
    SFixRef f = a*b;
    if (f.toBinString() != "111111111111111")
    {
        printf("mulTest 3 failed\n");
        printf("  got:    %s\n", f.toBinString().c_str());
        printf("  wanted: 111111111111111\n");
        return false;
    }

    return true;
}

bool doExtendTest()
{
    SFixRef a(1,15);
    a.fromBinString("1100000000000000");
    SFixRef b = a.removeLSBs(14);
    if (b.toBinString() != "11")
    {
        printf("extendTest 1 failed\n");
        printf("  got:    %s\n", b.toBinString().c_str());
        printf("  wanted: 11\n");
        return false;
    }

    SFixRef c = b.extendMSBs(2);
    if (c.toBinString() != "1111")
    {
        printf("extendTest 2 failed\n");
        printf("  got:    %s\n", c.toBinString().c_str());
        printf("  wanted: 1111\n");
        return false;
    }

    SFixRef d = c.extendLSBs(2);
    if (d.toBinString() != "111100")
    {
        printf("extendTest 3 failed\n");
        printf("  got:    %s\n", d.toBinString().c_str());
        printf("  wanted: 111100\n");
        return false;
    }

    return true;
}

void oneDivXTest()
{
    SFixRef b(8,0);
    b.fromHexString("0E");  // 14

    const uint32_t precision = 256;

    SFixRef x(8,32);
    x.fromHexString("0000000100");
    x = x.extendLSBs(precision-x.fracBits());

    for(uint32_t i=0; i<30; i++)
    {
        //x = x*(two-b*x);
        x = x.reinterpret(x.intBits()+1, x.fracBits()-1) - x*x*b;
        x = x.removeMSBs(x.intBits()-8);
        x = x.removeLSBs(x.fracBits()-precision);

        std::string s = x.toHexString();
        printf("x -> %s\n", s.c_str());
    }

    printf("x (dec) -> 0.%s\n", x.toDecString().c_str());

    //displayNumber(x);
}

bool hexTest()
{
    SFixRef a(16,0);
    a.fromHexString("0123");
    if (a.toHexString() != "0123")
    {
        printf("test 1\n");
        std::string s = a.toHexString();
        printf("Error: got    %s\n", s.c_str());
        printf("       wanted 0123\n");

        return false;
    }
    return true;
}

bool doTests()
{
    if (!binTest()) return false;
    if (!hexTest()) return false;
    if (!addTest()) return false;
    if (!mulTest()) return false;
    if (!doExtendTest()) return false;

    oneDivXTest();

    return true;
}

} // tests
