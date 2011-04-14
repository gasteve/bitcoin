#include "CTxIn.h"
#include "main.h"
#include "CCriticalBlock.h"
#include "CWalletTx.h"

CTxIn::CTxIn()
{
	nSequence = UINT_MAX;
}

CTxIn::CTxIn(COutPoint prevoutIn, CScript scriptSigIn, unsigned int nSequenceIn)
{
	prevout = prevoutIn;
	scriptSig = scriptSigIn;
	nSequence = nSequenceIn;
}

CTxIn::CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn, unsigned int nSequenceIn)
{
	prevout = COutPoint(hashPrevTx, nOut);
	scriptSig = scriptSigIn;
	nSequence = nSequenceIn;
}

bool CTxIn::IsFinal() const
{
	return (nSequence == UINT_MAX);
}

bool operator==(const CTxIn& a, const CTxIn& b)
{
	return (a.prevout   == b.prevout &&
			a.scriptSig == b.scriptSig &&
			a.nSequence == b.nSequence);
}

bool operator!=(const CTxIn& a, const CTxIn& b)
{
	return !(a == b);
}

string CTxIn::ToString() const
{
	string str;
	str += strprintf("CTxIn(");
	str += prevout.ToString();
	if (prevout.IsNull())
		str += strprintf(", coinbase %s", HexStr(scriptSig).c_str());
	else
		str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24).c_str());
	if (nSequence != UINT_MAX)
		str += strprintf(", nSequence=%u", nSequence);
	str += ")";
	return str;
}

void CTxIn::print() const
{
	printf("%s\n", ToString().c_str());
}

bool CTxIn::IsMine() const
{
    CRITICAL_BLOCK(cs_mapWallet)
    {
        map<uint256, CWalletTx>::iterator mi = mapWallet.find(prevout.hash);
        if (mi != mapWallet.end())
        {
            const CWalletTx& prev = (*mi).second;
            if (prevout.n < prev.vout.size())
                if (prev.vout[prevout.n].IsMine())
                    return true;
        }
    }
    return false;
}

int64 CTxIn::GetDebit() const
{
    CRITICAL_BLOCK(cs_mapWallet)
    {
        map<uint256, CWalletTx>::iterator mi = mapWallet.find(prevout.hash);
        if (mi != mapWallet.end())
        {
            const CWalletTx& prev = (*mi).second;
            if (prevout.n < prev.vout.size())
                if (prev.vout[prevout.n].IsMine())
                    return prev.vout[prevout.n].nValue;
        }
    }
    return 0;
}
