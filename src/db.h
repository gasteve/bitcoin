// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#ifndef DB_H
#define DB_H

#include "headers.h"
#include "serialize.h"
#include "util.h"
#include "uint256.h"
#include "CBigNum.h"
#include "CKey.h"
#include "CCriticalBlock.h"
#include "CWalletDB.h"

class CTransaction;
class CTxIndex;
class CDiskBlockIndex;
class CDiskTxPos;
class COutPoint;
class CUser;
class CReview;
class CAddress;
class CWalletTx;
class CAccount;
class CAccountingEntry;

extern map<string, string> mapAddressBook;
extern CCriticalSection cs_mapAddressBook;
extern vector<unsigned char> vchDefaultKey;
extern bool fClient;
extern int nBestHeight;

extern unsigned int nWalletDBUpdated;
extern DbEnv dbenv;
extern CCriticalSection cs_db;
extern bool fDbEnvInit;
extern map<string, int> mapFileUseCount;
extern map<string, Db*> mapDb;

extern void DBFlush(bool fShutdown);
extern vector<unsigned char> GetKeyFromKeyPool();
extern int64 GetOldestKeyPoolTime();
extern uint64 nAccountingEntryNumber;

bool LoadAddresses();

bool LoadWallet(bool& fFirstRunRet);
void BackupWallet(const string& strDest);
CBlockIndex* InsertBlockIndex(uint256 hash);

inline bool SetAddressBookName(const string& strAddress, const string& strName)
{
    return CWalletDB().WriteName(strAddress, strName);
}

#endif
