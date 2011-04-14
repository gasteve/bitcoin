#ifndef CMERKLETX_H
#define CMERKLETX_H

#include "CTransaction.h"
class CBlock;

//
// A transaction with a merkle branch linking it to the block chain
//
class CMerkleTx : public CTransaction
{
public:
    uint256 hashBlock;
    vector<uint256> vMerkleBranch;
    int nIndex;

    // memory only
    mutable char fMerkleVerified;

    CMerkleTx();
    CMerkleTx(const CTransaction&);
    void Init();
    int SetMerkleBranch(const CBlock* pblock=NULL);
    int GetDepthInMainChain(int& nHeightRet) const;
    int GetDepthInMainChain() const;
    bool IsInMainChain() const;
    int GetBlocksToMaturity() const;
    bool AcceptToMemoryPool(CTxDB& txdb, bool fCheckInputs=true);
    bool AcceptToMemoryPool();

    IMPLEMENT_SERIALIZE
    (
        nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
        nVersion = this->nVersion;
        READWRITE(hashBlock);
        READWRITE(vMerkleBranch);
        READWRITE(nIndex);
    )
};

#endif
