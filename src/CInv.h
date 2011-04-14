#ifndef CINV_H
#define CINV_H

#include "headers.h"
#include "serialize.h"
#include "uint256.h"

class CInv
{
public:
    int type;
    uint256 hash;

    CInv();
    CInv(int typeIn, const uint256& hashIn);
    CInv(const string& strType, const uint256& hashIn);
    friend inline bool operator<(const CInv& a, const CInv& b);
    bool IsKnownType() const;
    const char* GetCommand() const;
    string ToString() const;
    void print() const;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(type);
        READWRITE(hash);
    )
};

#include "CInv-inl.h"

#endif
