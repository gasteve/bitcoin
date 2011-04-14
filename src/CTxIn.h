#ifndef CTXIN_H
#define CTXIN_H

#include "COutPoint.h"
#include "CScript.h"

//
// An input of a transaction.  It contains the location of the previous
// transaction's output that it claims and a signature that matches the
// output's public key.
//
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    unsigned int nSequence;

    CTxIn();
    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=UINT_MAX);
    CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=UINT_MAX);
    bool IsFinal() const;
    friend bool operator==(const CTxIn& a, const CTxIn& b);
    friend bool operator!=(const CTxIn& a, const CTxIn& b);
    string ToString() const;
    void print() const;
    bool IsMine() const;
    int64 GetDebit() const;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    )
};

#endif
