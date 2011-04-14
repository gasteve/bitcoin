// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
//
// uint160
//

//
// uint160 and uint256 could be implemented as templates, but to keep
// compile errors and debugging cleaner, they're copy and pasted.
//

#ifndef UINT160_H
#define UINT160_H

#include "base_uint.h"

typedef base_uint<160> base_uint160;

class uint160 : public base_uint160
{
public:
    typedef base_uint160 basetype;

    uint160();
    uint160(const basetype& b);
    uint160& operator=(const basetype& b);
    uint160(uint64 b);
    uint160& operator=(uint64 b);
    explicit uint160(const std::string& str);
    explicit uint160(const std::vector<unsigned char>& vch);
};

#include "uint160-inl.h"

#endif
