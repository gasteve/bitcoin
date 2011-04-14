#ifndef CDISKBLOCKINDEX_H
#define CDISKBLOCKINDEX_H

#include "CBlockIndex.h"

//
// Used to marshal pointers into hashes for db storage.
//
class CDiskBlockIndex : public CBlockIndex
{
public:
    uint256 hashPrev;
    uint256 hashNext;

    CDiskBlockIndex();
    explicit CDiskBlockIndex(CBlockIndex* pindex);
    uint256 GetBlockHash() const;
    string ToString() const;
    void print() const;

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);

        READWRITE(hashNext);
        READWRITE(nFile);
        READWRITE(nBlockPos);
        READWRITE(nHeight);

        // block header
        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    )
};

#endif
