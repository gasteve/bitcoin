//
// Describes a place in the block chain to another node such that if the
// other node doesn't have the same branch, it can find a recent common trunk.
// The further back it is, the further before the fork it may be.
//

#ifndef CBLOCKLOCATOR_H
#define CBLOCKLOCATOR_H

#include "headers.h"
#include "serialize.h"
#include "uint256.h"

class CBlockIndex;

class CBlockLocator
{
protected:
    vector<uint256> vHave;
public:

    CBlockLocator();
    explicit CBlockLocator(const CBlockIndex* pindex);
    explicit CBlockLocator(uint256 hashBlock);
    void SetNull();
    bool IsNull();
    void Set(const CBlockIndex* pindex);
    int GetDistanceBack();
    CBlockIndex* GetBlockIndex();
    uint256 GetBlockHash();
    int GetHeight();

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    )
};

#endif
