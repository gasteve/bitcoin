#ifndef CTXOUT_H
#define CTXOUT_H

#include "headers.h"
#include "serialize.h"
#include "uint256.h"
#include "CScript.h"

class CTxOut;

//
// An output of a transaction.  It contains the public key that the next input
// must be able to sign with to claim it.
//
class CTxOut
{
public:
    int64 nValue;
    CScript scriptPubKey;

    CTxOut();
    CTxOut(int64 nValueIn, CScript scriptPubKeyIn);
    void SetNull();
    bool IsNull();
    uint256 GetHash() const;
    bool IsMine() const;
    int64 GetCredit() const;
    bool IsChange() const;
    int64 GetChange() const;
    friend bool operator==(const CTxOut& a, const CTxOut& b);
    friend bool operator!=(const CTxOut& a, const CTxOut& b);
    string ToString() const;
    void print() const;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    )
};

#endif
