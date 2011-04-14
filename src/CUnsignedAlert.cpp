#include "CUnsignedAlert.h"
#include "util.h"

void CUnsignedAlert::SetNull()
{
	nVersion = 1;
	nRelayUntil = 0;
	nExpiration = 0;
	nID = 0;
	nCancel = 0;
	setCancel.clear();
	nMinVer = 0;
	nMaxVer = 0;
	setSubVer.clear();
	nPriority = 0;

	strComment.clear();
	strStatusBar.clear();
	strReserved.clear();
}

string CUnsignedAlert::ToString() const
{
	string strSetCancel;
	foreach(int n, setCancel)
		strSetCancel += strprintf("%d ", n);
	string strSetSubVer;
	foreach(string str, setSubVer)
		strSetSubVer += "\"" + str + "\" ";
	return strprintf(
			"CAlert(\n"
			"    nVersion     = %d\n"
			"    nRelayUntil  = %"PRI64d"\n"
			"    nExpiration  = %"PRI64d"\n"
			"    nID          = %d\n"
			"    nCancel      = %d\n"
			"    setCancel    = %s\n"
			"    nMinVer      = %d\n"
			"    nMaxVer      = %d\n"
			"    setSubVer    = %s\n"
			"    nPriority    = %d\n"
			"    strComment   = \"%s\"\n"
			"    strStatusBar = \"%s\"\n"
			")\n",
		nVersion,
		nRelayUntil,
		nExpiration,
		nID,
		nCancel,
		strSetCancel.c_str(),
		nMinVer,
		nMaxVer,
		strSetSubVer.c_str(),
		nPriority,
		strComment.c_str(),
		strStatusBar.c_str());
}

void CUnsignedAlert::print() const
{
	printf("%s", ToString().c_str());
}
