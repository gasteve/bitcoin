#ifndef CTRANSACTION_H
#define CTRANSACTION_H

#include "CTxIn.h"
#include "CTxOut.h"
#include "CBlockIndex.h"
#include "COutPoint.h"
#include "CTxIndex.h"
#include "CDiskTxPos.h"

class CTxDB;

//
// The basic transaction that is broadcasted on the network and contained in
// blocks.  A transaction can contain multiple inputs and outputs.
//

class CTransaction
{
public:
    int nVersion;
    vector<CTxIn> vin;
    vector<CTxOut> vout;
    unsigned int nLockTime;

    static bool AllowFree(double dPriority);

    CTransaction();
    void SetNull();
    bool IsNull() const;
    uint256 GetHash() const;
    bool IsFinal(int nBlockHeight=0, int64 nBlockTime=0) const;
    bool IsNewerThan(const CTransaction& old) const;
    bool IsCoinBase() const;
    int GetSigOpCount() const;
    bool IsStandard() const;
    bool IsMine() const;
    bool IsFromMe() const;
    int64 GetDebit() const;
    int64 GetCredit() const;
    int64 GetChange() const;
    int64 GetValueOut() const;
    int64 GetMinFee(unsigned int nBlockSize=1, bool fAllowFree=true) const;
    bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL);
    friend bool operator==(const CTransaction& a, const CTransaction& b);
    friend bool operator!=(const CTransaction& a, const CTransaction& b);
    string ToString() const;
    void print() const;
    bool ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet);
    bool ReadFromDisk(CTxDB& txdb, COutPoint prevout);
    bool ReadFromDisk(COutPoint prevout);
    bool DisconnectInputs(CTxDB& txdb);
    bool ConnectInputs(CTxDB& txdb, map<uint256, CTxIndex>& mapTestPool, CDiskTxPos posThisTx,
                       CBlockIndex* pindexBlock, int64& nFees, bool fBlock, bool fMiner, int64 nMinFee=0);
    bool ClientConnectInputs();
    bool CheckTransaction() const;
    bool AcceptToMemoryPool(CTxDB& txdb, bool fCheckInputs=true, bool* pfMissingInputs=NULL);
    bool AcceptToMemoryPool(bool fCheckInputs=true, bool* pfMissingInputs=NULL);
    
    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(vin);
        READWRITE(vout);
        READWRITE(nLockTime);
    )

protected:
    bool AddToMemoryPoolUnchecked();
public:
    bool RemoveFromMemoryPool();
};

#endif
