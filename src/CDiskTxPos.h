#ifndef CDISKTXPOS_H
#define CDISKTXPOS_H

#include "headers.h"
#include "serialize.h"
#include "util.h"

class CDiskTxPos
{
public:
    unsigned int nFile;
    unsigned int nBlockPos;
    unsigned int nTxPos;

    CDiskTxPos();
    CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn);
    void SetNull();
    bool IsNull() const;
    friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b);
    friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b);
    string ToString() const;
    void print() const;

    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
};

#endif
