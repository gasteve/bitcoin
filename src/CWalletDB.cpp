#include "CWalletDB.h"
#include "main.h"
#include "db.h"
#include "peer.h"
#include "CCriticalBlock.h"
#include "CAddress.h"
#include "CWalletKey.h"

CWalletDB::CWalletDB(const char* pszMode) : CDB("wallet.dat", pszMode)
{
}

bool CWalletDB::ReadName(const string& strAddress, string& strName)
{
    strName = "";
    return Read(make_pair(string("name"), strAddress), strName);
}

bool CWalletDB::WriteName(const string& strAddress, const string& strName)
{
    CRITICAL_BLOCK(cs_mapAddressBook)
        mapAddressBook[strAddress] = strName;
    nWalletDBUpdated++;
    return Write(make_pair(string("name"), strAddress), strName);
}

bool CWalletDB::EraseName(const string& strAddress)
{
    // This should only be used for sending addresses, never for receiving addresses,
    // receiving addresses must always have an address book entry if they're not change return.
    CRITICAL_BLOCK(cs_mapAddressBook)
        mapAddressBook.erase(strAddress);
    nWalletDBUpdated++;
    return Erase(make_pair(string("name"), strAddress));
}

bool CWalletDB::ReadTx(uint256 hash, CWalletTx& wtx)
{
    return Read(make_pair(string("tx"), hash), wtx);
}

bool CWalletDB::WriteTx(uint256 hash, const CWalletTx& wtx)
{
    nWalletDBUpdated++;
    return Write(make_pair(string("tx"), hash), wtx);
}

bool CWalletDB::EraseTx(uint256 hash)
{
    nWalletDBUpdated++;
    return Erase(make_pair(string("tx"), hash));
}

bool CWalletDB::ReadKey(const vector<unsigned char>& vchPubKey, CPrivKey& vchPrivKey)
{
    vchPrivKey.clear();
    return Read(make_pair(string("key"), vchPubKey), vchPrivKey);
}

bool CWalletDB::WriteKey(const vector<unsigned char>& vchPubKey, const CPrivKey& vchPrivKey)
{
    nWalletDBUpdated++;
    return Write(make_pair(string("key"), vchPubKey), vchPrivKey, false);
}

bool CWalletDB::ReadDefaultKey(vector<unsigned char>& vchPubKey)
{
    vchPubKey.clear();
    return Read(string("defaultkey"), vchPubKey);
}

bool CWalletDB::WriteDefaultKey(const vector<unsigned char>& vchPubKey)
{
    vchDefaultKey = vchPubKey;
    nWalletDBUpdated++;
    return Write(string("defaultkey"), vchPubKey);
}

//
// CWalletDB
//

static set<int64> setKeyPool;
static CCriticalSection cs_setKeyPool;

bool CWalletDB::ReadAccount(const string& strAccount, CAccount& account)
{
    account.SetNull();
    return Read(make_pair(string("acc"), strAccount), account);
}

bool CWalletDB::WriteAccount(const string& strAccount, const CAccount& account)
{
    return Write(make_pair(string("acc"), strAccount), account);
}

bool CWalletDB::WriteAccountingEntry(const CAccountingEntry& acentry)
{
    return Write(make_tuple(string("acentry"), acentry.strAccount, ++nAccountingEntryNumber), acentry);
}

int64 CWalletDB::GetAccountCreditDebit(const string& strAccount)
{
    list<CAccountingEntry> entries;
    ListAccountCreditDebit(strAccount, entries);

    int64 nCreditDebit = 0;
    foreach (const CAccountingEntry& entry, entries)
        nCreditDebit += entry.nCreditDebit;

    return nCreditDebit;
}

void CWalletDB::ListAccountCreditDebit(const string& strAccount, list<CAccountingEntry>& entries)
{
    int64 nCreditDebit = 0;

    bool fAllAccounts = (strAccount == "*");

    Dbc* pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error("CWalletDB::ListAccountCreditDebit() : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    loop
    {
        // Read next record
        CDataStream ssKey;
        if (fFlags == DB_SET_RANGE)
            ssKey << make_tuple(string("acentry"), (fAllAccounts? string("") : strAccount), uint64(0));
        CDataStream ssValue;
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0)
        {
            pcursor->close();
            throw runtime_error("CWalletDB::ListAccountCreditDebit() : error scanning DB");
        }

        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "acentry")
            break;
        CAccountingEntry acentry;
        ssKey >> acentry.strAccount;
        if (!fAllAccounts && acentry.strAccount != strAccount)
            break;

        ssValue >> acentry;
        entries.push_back(acentry);
    }

    pcursor->close();
}

bool CWalletDB::LoadWallet()
{
    vchDefaultKey.clear();
    int nFileVersion = 0;
    vector<uint256> vWalletUpgrade;

    // Modify defaults
#ifndef __WXMSW__
    // Tray icon sometimes disappears on 9.10 karmic koala 64-bit, leaving no way to access the program
    fMinimizeToTray = false;
    fMinimizeOnClose = false;
#endif

    //// todo: shouldn't we catch exceptions and try to recover and continue?
    CRITICAL_BLOCK(cs_mapWallet)
    CRITICAL_BLOCK(cs_mapKeys)
    {
        // Get cursor
        Dbc* pcursor = GetCursor();
        if (!pcursor)
            return false;

        loop
        {
            // Read next record
            CDataStream ssKey;
            CDataStream ssValue;
            int ret = ReadAtCursor(pcursor, ssKey, ssValue);
            if (ret == DB_NOTFOUND)
                break;
            else if (ret != 0)
                return false;

            // Unserialize
            // Taking advantage of the fact that pair serialization
            // is just the two items serialized one after the other
            string strType;
            ssKey >> strType;
            if (strType == "name")
            {
                string strAddress;
                ssKey >> strAddress;
                ssValue >> mapAddressBook[strAddress];
            }
            else if (strType == "tx")
            {
                uint256 hash;
                ssKey >> hash;
                CWalletTx& wtx = mapWallet[hash];
                ssValue >> wtx;

                if (wtx.GetHash() != hash)
                    printf("Error in wallet.dat, hash mismatch\n");

                // Undo serialize changes in 31600
                if (31404 <= wtx.fTimeReceivedIsTxTime && wtx.fTimeReceivedIsTxTime <= 31703)
                {
                    if (!ssValue.empty())
                    {
                        char fTmp;
                        char fUnused;
                        ssValue >> fTmp >> fUnused >> wtx.strFromAccount;
                        printf("LoadWallet() upgrading tx ver=%d %d '%s' %s\n", wtx.fTimeReceivedIsTxTime, fTmp, wtx.strFromAccount.c_str(), hash.ToString().c_str());
                        wtx.fTimeReceivedIsTxTime = fTmp;
                    }
                    else
                    {
                        printf("LoadWallet() repairing tx ver=%d %s\n", wtx.fTimeReceivedIsTxTime, hash.ToString().c_str());
                        wtx.fTimeReceivedIsTxTime = 0;
                    }
                    vWalletUpgrade.push_back(hash);
                }

                //// debug print
                //printf("LoadWallet  %s\n", wtx.GetHash().ToString().c_str());
                //printf(" %12I64d  %s  %s  %s\n",
                //    wtx.vout[0].nValue,
                //    DateTimeStrFormat("%x %H:%M:%S", wtx.GetBlockTime()).c_str(),
                //    wtx.hashBlock.ToString().substr(0,20).c_str(),
                //    wtx.mapValue["message"].c_str());
            }
            else if (strType == "acentry")
            {
                string strAccount;
                ssKey >> strAccount;
                uint64 nNumber;
                ssKey >> nNumber;
                if (nNumber > nAccountingEntryNumber)
                    nAccountingEntryNumber = nNumber;
            }
            else if (strType == "key" || strType == "wkey")
            {
                vector<unsigned char> vchPubKey;
                ssKey >> vchPubKey;
                CWalletKey wkey;
                if (strType == "key")
                    ssValue >> wkey.vchPrivKey;
                else
                    ssValue >> wkey;

                mapKeys[vchPubKey] = wkey.vchPrivKey;
                mapPubKeys[Hash160(vchPubKey)] = vchPubKey;
            }
            else if (strType == "defaultkey")
            {
                ssValue >> vchDefaultKey;
            }
            else if (strType == "pool")
            {
                int64 nIndex;
                ssKey >> nIndex;
                setKeyPool.insert(nIndex);
            }
            else if (strType == "version")
            {
                ssValue >> nFileVersion;
                if (nFileVersion == 10300)
                    nFileVersion = 300;
            }
            else if (strType == "setting")
            {
                string strKey;
                ssKey >> strKey;

                // Menu state
                if (strKey == "fGenerateBitcoins")  ssValue >> fGenerateBitcoins;

                // Options
                if (strKey == "nTransactionFee")    ssValue >> nTransactionFee;
                if (strKey == "addrIncoming")       ssValue >> addrIncoming;
                if (strKey == "fLimitProcessors")   ssValue >> fLimitProcessors;
                if (strKey == "nLimitProcessors")   ssValue >> nLimitProcessors;
                if (strKey == "fMinimizeToTray")    ssValue >> fMinimizeToTray;
                if (strKey == "fMinimizeOnClose")   ssValue >> fMinimizeOnClose;
                if (strKey == "fUseProxy")          ssValue >> fUseProxy;
                if (strKey == "addrProxy")          ssValue >> addrProxy;
		if (fHaveUPnP && strKey == "fUseUPnP")           ssValue >> fUseUPnP;
            }
        }
        pcursor->close();
    }

    foreach(uint256 hash, vWalletUpgrade)
        WriteTx(hash, mapWallet[hash]);

    printf("nFileVersion = %d\n", nFileVersion);
    printf("fGenerateBitcoins = %d\n", fGenerateBitcoins);
    printf("nTransactionFee = %"PRI64d"\n", nTransactionFee);
    printf("addrIncoming = %s\n", addrIncoming.ToString().c_str());
    printf("fMinimizeToTray = %d\n", fMinimizeToTray);
    printf("fMinimizeOnClose = %d\n", fMinimizeOnClose);
    printf("fUseProxy = %d\n", fUseProxy);
    printf("addrProxy = %s\n", addrProxy.ToString().c_str());
    if (fHaveUPnP)
        printf("fUseUPnP = %d\n", fUseUPnP);


    // Upgrade
    if (nFileVersion < VERSION)
    {
        // Get rid of old debug.log file in current directory
        if (nFileVersion <= 105 && !pszSetDataDir[0])
            unlink("debug.log");

        WriteVersion(VERSION);
    }


    return true;
}

void CWalletDB::ReserveKeyFromKeyPool(int64& nIndex, CKeyPool& keypool)
{
    nIndex = -1;
    keypool.vchPubKey.clear();
    CRITICAL_BLOCK(cs_main)
    CRITICAL_BLOCK(cs_mapWallet)
    CRITICAL_BLOCK(cs_setKeyPool)
    {
        // Top up key pool
        int64 nTargetSize = max(GetArg("-keypool", 100), (int64)0);
        while (setKeyPool.size() < nTargetSize+1)
        {
            int64 nEnd = 1;
            if (!setKeyPool.empty())
                nEnd = *(--setKeyPool.end()) + 1;
            if (!Write(make_pair(string("pool"), nEnd), CKeyPool(GenerateNewKey())))
                throw runtime_error("ReserveKeyFromKeyPool() : writing generated key failed");
            setKeyPool.insert(nEnd);
            printf("keypool added key %"PRI64d", size=%d\n", nEnd, setKeyPool.size());
        }

        // Get the oldest key
        assert(!setKeyPool.empty());
        nIndex = *(setKeyPool.begin());
        setKeyPool.erase(setKeyPool.begin());
        if (!Read(make_pair(string("pool"), nIndex), keypool))
            throw runtime_error("ReserveKeyFromKeyPool() : read failed");
        if (!mapKeys.count(keypool.vchPubKey))
            throw runtime_error("ReserveKeyFromKeyPool() : unknown key in key pool");
        assert(!keypool.vchPubKey.empty());
        printf("keypool reserve %"PRI64d"\n", nIndex);
    }
}

void CWalletDB::KeepKey(int64 nIndex)
{
    // Remove from key pool
    CRITICAL_BLOCK(cs_main)
    CRITICAL_BLOCK(cs_mapWallet)
    {
        Erase(make_pair(string("pool"), nIndex));
    }
    printf("keypool keep %"PRI64d"\n", nIndex);
}

void CWalletDB::ReturnKey(int64 nIndex)
{
    // Return to key pool
    CRITICAL_BLOCK(cs_setKeyPool)
        setKeyPool.insert(nIndex);
    printf("keypool return %"PRI64d"\n", nIndex);
}


