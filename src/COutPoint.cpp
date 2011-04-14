#include "COutPoint.h"

COutPoint::COutPoint() { SetNull(); }
COutPoint::COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
void COutPoint::SetNull() { hash = 0; n = -1; }
bool COutPoint::IsNull() const { return (hash == 0 && n == -1); }

bool operator<(const COutPoint& a, const COutPoint& b)
{
	return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
}

bool operator==(const COutPoint& a, const COutPoint& b)
{
	return (a.hash == b.hash && a.n == b.n);
}

bool operator!=(const COutPoint& a, const COutPoint& b)
{
	return !(a == b);
}

string COutPoint::ToString() const
{
	return strprintf("COutPoint(%s, %d)", hash.ToString().substr(0,10).c_str(), n);
}

void COutPoint::print() const
{
	printf("%s\n", ToString().c_str());
}

