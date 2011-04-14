#include "CTxIndex.h"

CTxIndex::CTxIndex()
{
	SetNull();
}

CTxIndex::CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs)
{
	pos = posIn;
	vSpent.resize(nOutputs);
}

void CTxIndex::SetNull()
{
	pos.SetNull();
	vSpent.clear();
}

bool CTxIndex::IsNull()
{
	return pos.IsNull();
}

bool operator==(const CTxIndex& a, const CTxIndex& b)
{
	return (a.pos    == b.pos &&
			a.vSpent == b.vSpent);
}

bool operator!=(const CTxIndex& a, const CTxIndex& b)
{
	return !(a == b);
}
