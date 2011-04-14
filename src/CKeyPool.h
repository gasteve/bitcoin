#ifndef CKEYPOOL_H
#define CKEYPOOL_H

#include "headers.h"
#include "serialize.h"
#include "uint256.h"

class CKeyPool
{
public:
    int64 nTime;
    vector<unsigned char> vchPubKey;

    CKeyPool();
    CKeyPool(const vector<unsigned char>& vchPubKeyIn);

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(nTime);
        READWRITE(vchPubKey);
    )
};

#endif
