#ifndef CWALLETDB_H
#define CWALLETDB_H

#include "CDB.h"
#include "CAccount.h"
#include "CAccountingEntry.h"
#include "CKeyPool.h"
#include "CWalletTx.h"
#include "CKey.h"

extern unsigned int nWalletDBUpdated;

class CWalletDB : public CDB
{
public:
    CWalletDB(const char* pszMode="r+");
private:
    CWalletDB(const CWalletDB&);
    void operator=(const CWalletDB&);
public:
    bool ReadName(const string& strAddress, string& strName);
    bool WriteName(const string& strAddress, const string& strName);
    bool EraseName(const string& strAddress);
    bool ReadTx(uint256 hash, CWalletTx& wtx);
    bool WriteTx(uint256 hash, const CWalletTx& wtx);
    bool EraseTx(uint256 hash);
    bool ReadKey(const vector<unsigned char>& vchPubKey, CPrivKey& vchPrivKey);
    bool WriteKey(const vector<unsigned char>& vchPubKey, const CPrivKey& vchPrivKey);
    bool ReadDefaultKey(vector<unsigned char>& vchPubKey);
    bool WriteDefaultKey(const vector<unsigned char>& vchPubKey);
    template<typename T> bool ReadSetting(const string& strKey, T& value);
    template<typename T> bool WriteSetting(const string& strKey, const T& value);
    bool ReadAccount(const string& strAccount, CAccount& account);
    bool WriteAccount(const string& strAccount, const CAccount& account);
    bool WriteAccountingEntry(const CAccountingEntry& acentry);
    int64 GetAccountCreditDebit(const string& strAccount);
    void ListAccountCreditDebit(const string& strAccount, list<CAccountingEntry>& acentries);
    bool LoadWallet();
protected:
    void ReserveKeyFromKeyPool(int64& nIndex, CKeyPool& keypool);
    void KeepKey(int64 nIndex);
    static void ReturnKey(int64 nIndex);
    friend class CReserveKey;
    friend vector<unsigned char> GetKeyFromKeyPool();
    friend int64 GetOldestKeyPoolTime();
};

#include "CWalletDB-inl.h"

#endif
