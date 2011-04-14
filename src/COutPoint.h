#ifndef COUTPOINT_H
#define COUTPOINT_H

#include "util.h"
#include "serialize.h"

class COutPoint
{
public:
    uint256 hash;
    unsigned int n;

    COutPoint();
    COutPoint(uint256 hashIn, unsigned int nIn);
    void SetNull();
    bool IsNull() const;
    friend bool operator<(const COutPoint& a, const COutPoint& b);
    friend bool operator==(const COutPoint& a, const COutPoint& b);
    friend bool operator!=(const COutPoint& a, const COutPoint& b);
    string ToString() const;
    void print() const;
    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
};

#endif
