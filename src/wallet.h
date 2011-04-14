#ifndef WALLET_H
#define WALLET_H

#include "CWalletTx.h"
#include "CTransaction.h"
#include "db.h"
#include "CReserveKey.h"

//////////////////////////////////////////////////////////////////////////////
//
// BitcoinWallet
// This is a bitcoin wallet manager
//

//////////////////////////////////////////////////////////////////////////////
//
// mapKeys
//

bool AddKey(const CKey& key);
vector<unsigned char> GenerateNewKey();

//////////////////////////////////////////////////////////////////////////////
//
// mapWallet
//

bool AddToWallet(const CWalletTx& wtxIn);
bool AddToWalletIfMine(const CTransaction& tx, const CBlock* pblock);
bool AddToWalletIfFromMe(const CTransaction& tx, const CBlock* pblock);
bool EraseFromWallet(uint256 hash);
void WalletUpdateSpent(const COutPoint& prevout);

//////////////////////////////////////////////////////////////////////////////
//
// Actions
//

int64 GetBalance();
bool SelectCoinsMinConf(int64 nTargetValue, int nConfMine, int nConfTheirs, set<CWalletTx*>& setCoinsRet);
bool SelectCoins(int64 nTargetValue, set<CWalletTx*>& setCoinsRet);
bool CreateTransaction(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew, CReserveKey& reservekey, int64& nFeeRet);

// Call after CreateTransaction unless you want to abort
bool CommitTransaction(CWalletTx& wtxNew, CReserveKey& reservekey);
string SendMoney(CScript scriptPubKey, int64 nValue, CWalletTx& wtxNew, bool fAskFee=false);
string SendMoneyToBitcoinAddress(string strAddress, int64 nValue, CWalletTx& wtxNew, bool fAskFee=false);

#endif
