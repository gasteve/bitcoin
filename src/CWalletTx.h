#ifndef CWALLETTX_H
#define CWALLETTX_H

#include "CMerkleTx.h"

//
// A transaction with a bunch of additional info that only the owner cares
// about.  It includes any unrecorded transactions needed to link it back
// to the block chain.
//
class CWalletTx : public CMerkleTx
{
public:
    vector<CMerkleTx> vtxPrev;
    map<string, string> mapValue;
    vector<pair<string, string> > vOrderForm;
    unsigned int fTimeReceivedIsTxTime;
    unsigned int nTimeReceived;  // time received by this node
    char fFromMe;
    char fSpent;
    string strFromAccount;

    // memory only
    mutable char fDebitCached;
    mutable char fCreditCached;
    mutable char fChangeCached;
    mutable int64 nDebitCached;
    mutable int64 nCreditCached;
    mutable int64 nChangeCached;

    // memory only UI hints
    mutable unsigned int nTimeDisplayed;
    mutable int nLinesDisplayed;
    mutable char fConfirmedDisplayed;

    CWalletTx();
    CWalletTx(const CMerkleTx& txIn);
    CWalletTx(const CTransaction& txIn);
    void Init();
    int64 GetDebit() const;
    int64 GetCredit(bool fUseCache=true) const;
    int64 GetChange() const;
    void GetAmounts(int64& nGenerated, list<pair<string /* address */, int64> >& listReceived,
                    list<pair<string /* address */, int64> >& listSent, int64& nFee, string& strSentAccount) const;
    void GetAccountAmounts(const string& strAccount, int64& nGenerated, int64& nReceived, 
                           int64& nSent, int64& nFee) const;
    bool IsFromMe() const;
    bool IsConfirmed() const;
    bool WriteToDisk();
    int64 GetTxTime() const;
    int GetRequestCount() const;
    void AddSupportingTransactions(CTxDB& txdb);
    bool AcceptWalletTransaction();
    bool AcceptWalletTransaction(CTxDB& txdb, bool fCheckInputs=true);
    void RelayWalletTransaction();
    void RelayWalletTransaction(CTxDB& txdb);

    IMPLEMENT_SERIALIZE
    (
        CWalletTx* pthis = const_cast<CWalletTx*>(this);
        if (fRead)
            pthis->Init();
        nSerSize += SerReadWrite(s, *(CMerkleTx*)this, nType, nVersion, ser_action);
        READWRITE(vtxPrev);

        pthis->mapValue["fromaccount"] = pthis->strFromAccount;
        READWRITE(mapValue);
        pthis->strFromAccount = pthis->mapValue["fromaccount"];
        pthis->mapValue.erase("fromaccount");
        pthis->mapValue.erase("version");

        READWRITE(vOrderForm);
        READWRITE(fTimeReceivedIsTxTime);
        READWRITE(nTimeReceived);
        READWRITE(fFromMe);
        READWRITE(fSpent);
    )
};

#endif
