//
// Account information.
// Stored in wallet with key "acc"+string account name
//

#ifndef CACCOUNT_H
#define CACCOUNT_H

#include "headers.h"
#include "serialize.h"

class CAccount
{
public:
    vector<unsigned char> vchPubKey;

    CAccount();
    void SetNull();

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vchPubKey);
    )
};

#endif
