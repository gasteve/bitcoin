#include "CDiskBlockIndex.h"
#include "util.h"
#include "CBlock.h"

CDiskBlockIndex::CDiskBlockIndex()
{
	hashPrev = 0;
	hashNext = 0;
}

CDiskBlockIndex::CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex)
{
	hashPrev = (pprev ? pprev->GetBlockHash() : 0);
	hashNext = (pnext ? pnext->GetBlockHash() : 0);
}

uint256 CDiskBlockIndex::GetBlockHash() const
{
	CBlock block;
	block.nVersion        = nVersion;
	block.hashPrevBlock   = hashPrev;
	block.hashMerkleRoot  = hashMerkleRoot;
	block.nTime           = nTime;
	block.nBits           = nBits;
	block.nNonce          = nNonce;
	return block.GetHash();
}


string CDiskBlockIndex::ToString() const
{
	string str = "CDiskBlockIndex(";
	str += CBlockIndex::ToString();
	str += strprintf("\n                hashBlock=%s, hashPrev=%s, hashNext=%s)",
		GetBlockHash().ToString().c_str(),
		hashPrev.ToString().substr(0,20).c_str(),
		hashNext.ToString().substr(0,20).c_str());
	return str;
}

void CDiskBlockIndex::print() const
{
	printf("%s\n", ToString().c_str());
}
