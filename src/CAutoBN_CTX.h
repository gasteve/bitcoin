#ifndef CAUTOBN_CTX_H
#define CAUTOBN_CTX_H

#include "headers.h"

class CAutoBN_CTX
{
protected:
    BN_CTX* pctx;
    BN_CTX* operator=(BN_CTX* pnew);

public:
    CAutoBN_CTX();
    ~CAutoBN_CTX();
    operator BN_CTX*();
    BN_CTX& operator*();
    BN_CTX** operator&();
    bool operator!();
};

#endif
