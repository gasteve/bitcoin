#include "CInv.h"

#include "headers.h"
#include "util.h"
#include "peer.h"

CInv::CInv()
{
	type = 0;
	hash = 0;
}

CInv::CInv(int typeIn, const uint256& hashIn)
{
	type = typeIn;
	hash = hashIn;
}

CInv::CInv(const string& strType, const uint256& hashIn)
{
	int i;
	for (i = 1; i < ARRAYLEN(ppszTypeName); i++)
	{
		if (strType == ppszTypeName[i])
		{
			type = i;
			break;
		}
	}
	if (i == ARRAYLEN(ppszTypeName))
		throw std::out_of_range(strprintf("CInv::CInv(string, uint256) : unknown type '%s'", strType.c_str()));
	hash = hashIn;
}

bool CInv::IsKnownType() const
{
	return (type >= 1 && type < ARRAYLEN(ppszTypeName));
}

const char* CInv::GetCommand() const
{
	if (!IsKnownType())
		throw std::out_of_range(strprintf("CInv::GetCommand() : type=%d unknown type", type));
	return ppszTypeName[type];
}

string CInv::ToString() const
{
	return strprintf("%s %s", GetCommand(), hash.ToString().substr(0,20).c_str());
}

void CInv::print() const
{
	printf("CInv(%s)\n", ToString().c_str());
}
