#include "CWalletKey.h"
#include "util.h"

CWalletKey::CWalletKey(int64 nExpires)
{
	nTimeCreated = (nExpires ? GetTime() : 0);
	nTimeExpires = nExpires;
}
