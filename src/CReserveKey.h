#ifndef CRESERVEKEY_H
#define CRESERVEKEY_H

#include "headers.h"
#include "uint256.h"

class CReserveKey
{
protected:
    int64 nIndex;
    vector<unsigned char> vchPubKey;
public:
    CReserveKey();
    ~CReserveKey();
    vector<unsigned char> GetReservedKey();
    void KeepKey();
    void ReturnKey();
};

#endif
