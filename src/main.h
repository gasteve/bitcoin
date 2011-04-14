// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAIN_H
#define MAIN_H

#include "headers.h"
#include "CKey.h"
#include "CCriticalBlock.h"
#include "CAlert.h"

class CNode;
class CReserveKey;
class COutPoint;
class CInPoint;
class CDiskTxPos;
class CCoinBase;
class CTxIn;
class CTxOut;
class CTxDB;
class CTransaction;
class CBlock;
class CBlockIndex;
class CWalletTx;
class CKeyItem;
class CBigNum;
class CAddress;

static const unsigned int MAX_BLOCK_SIZE = 1000000;
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;
static const int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
static const int64 COIN = 100000000;
static const int64 CENT = 1000000;
static const int64 MAX_MONEY = 21000000 * COIN;
inline bool MoneyRange(int64 nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
static const int COINBASE_MATURITY = 100;

extern CCriticalSection cs_main;
extern CCriticalSection cs_mapAlerts;
extern map<uint256, CAlert> mapAlerts;
extern map<uint256, CBlockIndex*> mapBlockIndex;
extern uint256 hashGenesisBlock;
extern CBigNum bnProofOfWorkLimit;
extern CBlockIndex* pindexGenesisBlock;
extern int nBestHeight;
extern CBigNum bnBestChainWork;
extern CBigNum bnBestInvalidWork;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern unsigned int nTransactionsUpdated;
extern map<uint256, int> mapRequestCount;
extern CCriticalSection cs_mapRequestCount;
extern map<string, string> mapAddressBook;
extern CCriticalSection cs_mapAddressBook;
extern vector<unsigned char> vchDefaultKey;
extern double dHashesPerSec;
extern int64 nHPSTimerStart;

// Settings
extern int fGenerateBitcoins;
extern int64 nTransactionFee;
extern CAddress addrIncoming;
extern int fLimitProcessors;
extern int nLimitProcessors;
extern int fMinimizeToTray;
extern int fMinimizeOnClose;

bool CheckDiskSpace(uint64 nAdditionalBytes=0);
FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
FILE* AppendBlockFile(unsigned int& nFileRet);
bool AddKey(const CKey& key);
bool Reorganize(CTxDB& txdb, CBlockIndex* pindexNew);
int64 GetBlockValue(int nHeight, int64 nFees);
void InvalidChainFound(CBlockIndex* pindexNew);
bool CheckProofOfWork(uint256 hash, unsigned int nBits);
unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast);
vector<unsigned char> GenerateNewKey();
bool LoadBlockIndex(bool fAllowNew=true);
void PrintBlockTree();
bool ProcessMessages(CNode* pfrom);
bool ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv);
bool SendMessages(CNode* pto, bool fSendTrickle);
bool CreateTransaction(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew, CReserveKey& reservekey, int64& nFeeRet);
bool CommitTransaction(CWalletTx& wtxNew, CReserveKey& reservekey);
bool BroadcastTransaction(CWalletTx& wtxNew);
bool IsInitialBlockDownload();
string GetWarnings(string strFor);
int ScanForWalletTransactions(CBlockIndex* pindexStart);
void ReacceptWalletTransactions();
bool ProcessBlock(CNode* pfrom, CBlock* pblock);

extern int64 nTimeBestReceived;
extern map<uint256, CTransaction> mapTransactions;
extern map<uint256, CWalletTx> mapWallet;
extern vector<uint256> vWalletUpdated;
extern CCriticalSection cs_mapWallet;
extern map<vector<unsigned char>, CPrivKey> mapKeys;
extern map<uint160, vector<unsigned char> > mapPubKeys;
extern CCriticalSection cs_mapKeys;
extern CKey keyUser;
extern CBlockIndex* pindexBest;
extern CCriticalSection cs_mapTransactions;
extern map<COutPoint, CInPoint> mapNextTx;

#endif
