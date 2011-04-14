//
// A txdb record that contains the disk location of a transaction and the
// locations of transactions that spend its outputs.  vSpent is really only
// used as a flag, but having the location is very helpful for debugging.
//

#ifndef CTXINDEX_H
#define CTXINDEX_H

#include "CDiskTxPos.h"

class CDiskTxPos;

class CTxIndex
{
public:
    CDiskTxPos pos;
    vector<CDiskTxPos> vSpent;

    CTxIndex();
    CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs);
    void SetNull();
    bool IsNull();
    friend bool operator==(const CTxIndex& a, const CTxIndex& b);
    friend bool operator!=(const CTxIndex& a, const CTxIndex& b);

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(pos);
        READWRITE(vSpent);
    )
};

#endif


