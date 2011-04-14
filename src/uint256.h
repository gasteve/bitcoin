// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
//
// uint256
//

//
// uint160 and uint256 could be implemented as templates, but to keep
// compile errors and debugging cleaner, they're copy and pasted.
//

#ifndef UINT256_H
#define UINT256_H

#include "base_uint.h"

inline int Testuint256AdHoc(vector<string> vArg);
typedef base_uint<256> base_uint256;

class uint256 : public base_uint256
{
public:
    typedef base_uint256 basetype;

    uint256();
    uint256(const basetype& b);
    uint256& operator=(const basetype& b);
    uint256(uint64 b);
    uint256& operator=(uint64 b);
    explicit uint256(const std::string& str);
    explicit uint256(const std::vector<unsigned char>& vch);
};

#include "uint256-inl.h"

#endif
