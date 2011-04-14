#include "CBlockIndex.h"
#include "main.h"
#include "CTransaction.h"
#include "CBlock.h"

CBlockIndex::CBlockIndex()
{
	phashBlock = NULL;
	pprev = NULL;
	pnext = NULL;
	nFile = 0;
	nBlockPos = 0;
	nHeight = 0;
	bnChainWork = 0;

	nVersion       = 0;
	hashMerkleRoot = 0;
	nTime          = 0;
	nBits          = 0;
	nNonce         = 0;
}

CBlockIndex::CBlockIndex(unsigned int nFileIn, unsigned int nBlockPosIn, CBlock& block)
{
	phashBlock = NULL;
	pprev = NULL;
	pnext = NULL;
	nFile = nFileIn;
	nBlockPos = nBlockPosIn;
	nHeight = 0;
	bnChainWork = 0;

	nVersion       = block.nVersion;
	hashMerkleRoot = block.hashMerkleRoot;
	nTime          = block.nTime;
	nBits          = block.nBits;
	nNonce         = block.nNonce;
}

CBlock CBlockIndex::GetBlockHeader() const
{
	CBlock block;
	block.nVersion       = nVersion;
	if (pprev)
		block.hashPrevBlock = pprev->GetBlockHash();
	block.hashMerkleRoot = hashMerkleRoot;
	block.nTime          = nTime;
	block.nBits          = nBits;
	block.nNonce         = nNonce;
	return block;
}

uint256 CBlockIndex::GetBlockHash() const
{
	return *phashBlock;
}

int64 CBlockIndex::GetBlockTime() const
{
	return (int64)nTime;
}

CBigNum CBlockIndex::GetBlockWork() const
{
	CBigNum bnTarget;
	bnTarget.SetCompact(nBits);
	if (bnTarget <= 0)
		return 0;
	return (CBigNum(1)<<256) / (bnTarget+1);
}

bool CBlockIndex::IsInMainChain() const
{
	return (pnext || this == pindexBest);
}

bool CBlockIndex::CheckIndex() const
{
	return CheckProofOfWork(GetBlockHash(), nBits);
}

bool CBlockIndex::EraseBlockFromDisk()
{
	// Open history file
	CAutoFile fileout = OpenBlockFile(nFile, nBlockPos, "rb+");
	if (!fileout)
		return false;

	// Overwrite with empty null block
	CBlock block;
	block.SetNull();
	fileout << block;

	return true;
}

int64 CBlockIndex::GetMedianTimePast() const
{
	int64 pmedian[nMedianTimeSpan];
	int64* pbegin = &pmedian[nMedianTimeSpan];
	int64* pend = &pmedian[nMedianTimeSpan];

	const CBlockIndex* pindex = this;
	for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
		*(--pbegin) = pindex->GetBlockTime();

	sort(pbegin, pend);
	return pbegin[(pend - pbegin)/2];
}

int64 CBlockIndex::GetMedianTime() const
{
	const CBlockIndex* pindex = this;
	for (int i = 0; i < nMedianTimeSpan/2; i++)
	{
		if (!pindex->pnext)
			return GetBlockTime();
		pindex = pindex->pnext;
	}
	return pindex->GetMedianTimePast();
}

string CBlockIndex::ToString() const
{
	return strprintf("CBlockIndex(nprev=%08x, pnext=%08x, nFile=%d, nBlockPos=%-6d nHeight=%d, merkle=%s, hashBlock=%s)",
		pprev, pnext, nFile, nBlockPos, nHeight,
		hashMerkleRoot.ToString().substr(0,10).c_str(),
		GetBlockHash().ToString().substr(0,20).c_str());
}

void CBlockIndex::print() const
{
	printf("%s\n", ToString().c_str());
}

