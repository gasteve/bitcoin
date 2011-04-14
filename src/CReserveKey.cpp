#include "CReserveKey.h"

#include "db.h"
#include "CKeyPool.h"
#include "CWalletDB.h"

CReserveKey::CReserveKey()
{
    nIndex = -1;
}

CReserveKey::~CReserveKey()
{
    if (!fShutdown)
        ReturnKey();
}

vector<unsigned char> CReserveKey::GetReservedKey()
{
    if (nIndex == -1)
    {
        CKeyPool keypool;
        CWalletDB().ReserveKeyFromKeyPool(nIndex, keypool);
        vchPubKey = keypool.vchPubKey;
    }
    assert(!vchPubKey.empty());
    return vchPubKey;
}

void CReserveKey::KeepKey()
{
    if (nIndex != -1)
        CWalletDB().KeepKey(nIndex);
    nIndex = -1;
    vchPubKey.clear();
}

void CReserveKey::ReturnKey()
{
    if (nIndex != -1)
        CWalletDB::ReturnKey(nIndex);
    nIndex = -1;
    vchPubKey.clear();
}

