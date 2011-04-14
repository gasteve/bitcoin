#ifndef CALERT_H
#define CALERT_H

#include "CUnsignedAlert.h"

class CNode;

class CAlert : public CUnsignedAlert
{
public:
    vector<unsigned char> vchMsg;
    vector<unsigned char> vchSig;

    CAlert();
    void SetNull();
    bool IsNull() const;
    uint256 GetHash() const;
    bool IsInEffect() const;
    bool Cancels(const CAlert& alert) const;
    bool AppliesTo(int nVersion, string strSubVerIn) const;
    bool AppliesToMe() const;
    bool RelayTo(CNode* pnode) const;
    bool CheckSignature();
    bool ProcessAlert();

    IMPLEMENT_SERIALIZE
    (
        READWRITE(vchMsg);
        READWRITE(vchSig);
    )
};

#endif
