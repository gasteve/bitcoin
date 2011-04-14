#ifndef BLOCKINDEX_H
#define BLOCKINDEX_H

#include "CBigNum.h"

class CBlock;

//
// The block chain is a tree shaped structure starting with the
// genesis block at the root, with each block potentially having multiple
// candidates to be the next block.  pprev and pnext link a path through the
// main/longest chain.  A blockindex may have multiple pprev pointing back
// to it, but pnext will only point forward to the longest branch, or will
// be null if the block is not part of the longest chain.
//
class CBlockIndex
{
public:
    const uint256* phashBlock;
    CBlockIndex* pprev;
    CBlockIndex* pnext;
    unsigned int nFile;
    unsigned int nBlockPos;
    int nHeight;
    CBigNum bnChainWork;

    // block header
    int nVersion;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    enum { nMedianTimeSpan=11 };

    CBlockIndex();
    CBlockIndex(unsigned int nFileIn, unsigned int nBlockPosIn, CBlock& block);
    CBlock GetBlockHeader() const;
    uint256 GetBlockHash() const;
    int64 GetBlockTime() const;
    CBigNum GetBlockWork() const;
    bool IsInMainChain() const;
    bool CheckIndex() const;
    bool EraseBlockFromDisk();
    int64 GetMedianTimePast() const;
    int64 GetMedianTime() const;
    string ToString() const;
    void print() const;
};

#endif
