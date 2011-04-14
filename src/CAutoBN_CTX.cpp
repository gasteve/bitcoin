#include "CAutoBN_CTX.h"
#include "CBigNum.h"

BN_CTX* CAutoBN_CTX::operator=(BN_CTX* pnew) 
{
    return pctx = pnew;
}

CAutoBN_CTX::CAutoBN_CTX()
{
	pctx = BN_CTX_new();
	if (pctx == NULL)
		throw bignum_error("CAutoBN_CTX : BN_CTX_new() returned NULL");
}

CAutoBN_CTX::~CAutoBN_CTX()
{
	if (pctx != NULL)
		BN_CTX_free(pctx);
}

CAutoBN_CTX::operator BN_CTX*() { return pctx; }
BN_CTX& CAutoBN_CTX::operator*() { return *pctx; }
BN_CTX** CAutoBN_CTX::operator&() { return &pctx; }
bool CAutoBN_CTX::operator!() { return (pctx == NULL); }
