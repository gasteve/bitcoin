// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include "db.h"
#include "headers.h"
#include "uint256.h"
#include "CCriticalSection.h"
#include "main.h"
#include "base58.h"
#include "CTryCriticalBlock.h"

void ThreadFlushWalletDB(void* parg);

unsigned int nWalletDBUpdated;
uint64 nAccountingEntryNumber = 0;

//
// CDB
//

CCriticalSection cs_db;
bool fDbEnvInit = false;
DbEnv dbenv(0);
map<string, int> mapFileUseCount;
map<string, Db*> mapDb;

class CDBInit
{
public:
    CDBInit()
    {
    }
    ~CDBInit()
    {
        if (fDbEnvInit)
        {
            dbenv.close(0);
            fDbEnvInit = false;
        }
    }
}
instance_of_cdbinit;

void CloseDb(const string& strFile)
{
    CRITICAL_BLOCK(cs_db)
    {
        if (mapDb[strFile] != NULL)
        {
            // Close the database handle
            Db* pdb = mapDb[strFile];
            pdb->close(0);
            delete pdb;
            mapDb[strFile] = NULL;
        }
    }
}

void DBFlush(bool fShutdown)
{
    // Flush log data to the actual data file
    //  on all files that are not in use
    printf("DBFlush(%s)%s\n", fShutdown ? "true" : "false", fDbEnvInit ? "" : " db not started");
    if (!fDbEnvInit)
        return;
    CRITICAL_BLOCK(cs_db)
    {
        map<string, int>::iterator mi = mapFileUseCount.begin();
        while (mi != mapFileUseCount.end())
        {
            string strFile = (*mi).first;
            int nRefCount = (*mi).second;
            printf("%s refcount=%d\n", strFile.c_str(), nRefCount);
            if (nRefCount == 0)
            {
                // Move log data to the dat file
                CloseDb(strFile);
                dbenv.txn_checkpoint(0, 0, 0);
                printf("%s flush\n", strFile.c_str());
                dbenv.lsn_reset(strFile.c_str(), 0);
                mapFileUseCount.erase(mi++);
            }
            else
                mi++;
        }
        if (fShutdown)
        {
            char** listp;
            if (mapFileUseCount.empty())
                dbenv.log_archive(&listp, DB_ARCH_REMOVE);
            dbenv.close(0);
            fDbEnvInit = false;
        }
    }
}

CBlockIndex* InsertBlockIndex(uint256 hash)
{
    if (hash == 0)
        return NULL;

    // Return existing
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
    if (mi != mapBlockIndex.end())
        return (*mi).second;

    // Create new
    CBlockIndex* pindexNew = new CBlockIndex();
    if (!pindexNew)
        throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");
    mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
    pindexNew->phashBlock = &((*mi).first);

    return pindexNew;
}

bool LoadWallet(bool& fFirstRunRet)
{
    fFirstRunRet = false;
    if (!CWalletDB("cr+").LoadWallet())
        return false;
    fFirstRunRet = vchDefaultKey.empty();

    if (mapKeys.count(vchDefaultKey))
    {
        // Set keyUser
        keyUser.SetPubKey(vchDefaultKey);
        keyUser.SetPrivKey(mapKeys[vchDefaultKey]);
    }
    else
    {
        // Create new keyUser and set as default key
        RandAddSeedPerfmon();
        keyUser.MakeNewKey();
        if (!AddKey(keyUser))
            return false;
        if (!SetAddressBookName(PubKeyToAddress(keyUser.GetPubKey()), ""))
            return false;
        CWalletDB().WriteDefaultKey(keyUser.GetPubKey());
    }

    CreateThread(ThreadFlushWalletDB, NULL);
    return true;
}

void ThreadFlushWalletDB(void* parg)
{
    static bool fOneThread;
    if (fOneThread)
        return;
    fOneThread = true;
    if (mapArgs.count("-noflushwallet"))
        return;

    unsigned int nLastSeen = nWalletDBUpdated;
    unsigned int nLastFlushed = nWalletDBUpdated;
    int64 nLastWalletUpdate = GetTime();
    while (!fShutdown)
    {
        Sleep(500);

        if (nLastSeen != nWalletDBUpdated)
        {
            nLastSeen = nWalletDBUpdated;
            nLastWalletUpdate = GetTime();
        }

        if (nLastFlushed != nWalletDBUpdated && GetTime() - nLastWalletUpdate >= 2)
        {
            TRY_CRITICAL_BLOCK(cs_db)
            {
                // Don't do this if any databases are in use
                int nRefCount = 0;
                map<string, int>::iterator mi = mapFileUseCount.begin();
                while (mi != mapFileUseCount.end())
                {
                    nRefCount += (*mi).second;
                    mi++;
                }

                if (nRefCount == 0 && !fShutdown)
                {
                    string strFile = "wallet.dat";
                    map<string, int>::iterator mi = mapFileUseCount.find(strFile);
                    if (mi != mapFileUseCount.end())
                    {
                        printf("%s ", DateTimeStrFormat("%x %H:%M:%S", GetTime()).c_str());
                        printf("Flushing wallet.dat\n");
                        nLastFlushed = nWalletDBUpdated;
                        int64 nStart = GetTimeMillis();

                        // Flush wallet.dat so it's self contained
                        CloseDb(strFile);
                        dbenv.txn_checkpoint(0, 0, 0);
                        dbenv.lsn_reset(strFile.c_str(), 0);

                        mapFileUseCount.erase(mi++);
                        printf("Flushed wallet.dat %"PRI64d"ms\n", GetTimeMillis() - nStart);
                    }
                }
            }
        }
    }
}

void BackupWallet(const string& strDest)
{
    while (!fShutdown)
    {
        CRITICAL_BLOCK(cs_db)
        {
            const string strFile = "wallet.dat";
            if (!mapFileUseCount.count(strFile) || mapFileUseCount[strFile] == 0)
            {
                // Flush log data to the dat file
                CloseDb(strFile);
                dbenv.txn_checkpoint(0, 0, 0);
                dbenv.lsn_reset(strFile.c_str(), 0);
                mapFileUseCount.erase(strFile);

                // Copy wallet.dat
                filesystem::path pathSrc(GetDataDir() + "/" + strFile);
                filesystem::path pathDest(strDest);
                if (filesystem::is_directory(pathDest))
                    pathDest = pathDest / strFile;
#if BOOST_VERSION >= 104000
                filesystem::copy_file(pathSrc, pathDest, filesystem::copy_option::overwrite_if_exists);
#else
                filesystem::copy_file(pathSrc, pathDest);
#endif
                printf("copied wallet.dat to %s\n", pathDest.string().c_str());

                return;
            }
        }
        Sleep(100);
    }
}

vector<unsigned char> GetKeyFromKeyPool()
{
    CWalletDB walletdb;
    int64 nIndex = 0;
    CKeyPool keypool;
    walletdb.ReserveKeyFromKeyPool(nIndex, keypool);
    walletdb.KeepKey(nIndex);
    return keypool.vchPubKey;
}

int64 GetOldestKeyPoolTime()
{
    CWalletDB walletdb;
    int64 nIndex = 0;
    CKeyPool keypool;
    walletdb.ReserveKeyFromKeyPool(nIndex, keypool);
    walletdb.ReturnKey(nIndex);
    return keypool.nTime;
}
