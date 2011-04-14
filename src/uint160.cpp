#include "uint160.h"

//////////////////////////////////////////////////////////////////////////////
//
// uint160
//

uint160::uint160()
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = 0;
}

uint160::uint160(const basetype& b)
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = b.pn[i];
}

uint160& uint160::operator=(const basetype& b)
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = b.pn[i];
    return *this;
}

uint160::uint160(uint64 b)
{
    pn[0] = (unsigned int)b;
    pn[1] = (unsigned int)(b >> 32);
    for (int i = 2; i < WIDTH; i++)
        pn[i] = 0;
}

uint160& uint160::operator=(uint64 b)
{
    pn[0] = (unsigned int)b;
    pn[1] = (unsigned int)(b >> 32);
    for (int i = 2; i < WIDTH; i++)
        pn[i] = 0;
    return *this;
}

uint160::uint160(const std::string& str)
{
    SetHex(str);
}

uint160::uint160(const std::vector<unsigned char>& vch)
{
    if (vch.size() == sizeof(pn))
        memcpy(pn, &vch[0], sizeof(pn));
    else
        *this = 0;
}
