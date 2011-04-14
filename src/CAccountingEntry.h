//
// Internal transfers.
// Database key is acentry<account><counter>
//

#ifndef CACCOUNTINGENTRY_H
#define CACCOUNTINGENTRY_H

#include "headers.h"
#include "serialize.h"

class CAccountingEntry
{
public:
    string strAccount;
    int64 nCreditDebit;
    int64 nTime;
    string strOtherAccount;
    string strComment;

    CAccountingEntry();
    void SetNull();

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        // Note: strAccount is serialized as part of the key, not here.
        READWRITE(nCreditDebit);
        READWRITE(nTime);
        READWRITE(strOtherAccount);
        READWRITE(strComment);
    )
};

#endif
