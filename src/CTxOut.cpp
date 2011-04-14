#include "CTxOut.h"
#include "main.h"
#include "script.h"
#include "util.h"
#include "base58.h"
#include "CCriticalBlock.h"

CTxOut::CTxOut()
{
	SetNull();
}

CTxOut::CTxOut(int64 nValueIn, CScript scriptPubKeyIn)
{
	nValue = nValueIn;
	scriptPubKey = scriptPubKeyIn;
}

void CTxOut::SetNull()
{
	nValue = -1;
	scriptPubKey.clear();
}

bool CTxOut::IsNull()
{
	return (nValue == -1);
}

uint256 CTxOut::GetHash() const
{
	return SerializeHash(*this);
}

bool CTxOut::IsMine() const
{
	return ::IsMine(scriptPubKey);
}

int64 CTxOut::GetCredit() const
{
	if (!MoneyRange(nValue))
		throw runtime_error("CTxOut::GetCredit() : value out of range");
	return (IsMine() ? nValue : 0);
}

bool CTxOut::IsChange() const
{
	// On a debit transaction, a txout that's mine but isn't in the address book is change
	vector<unsigned char> vchPubKey;
	if (ExtractPubKey(scriptPubKey, true, vchPubKey))
		CRITICAL_BLOCK(cs_mapAddressBook)
			if (!mapAddressBook.count(PubKeyToAddress(vchPubKey)))
				return true;
	return false;
}

int64 CTxOut::GetChange() const
{
	if (!MoneyRange(nValue))
		throw runtime_error("CTxOut::GetChange() : value out of range");
	return (IsChange() ? nValue : 0);
}

bool operator==(const CTxOut& a, const CTxOut& b)
{
	return (a.nValue       == b.nValue &&
			a.scriptPubKey == b.scriptPubKey);
}

bool operator!=(const CTxOut& a, const CTxOut& b)
{
	return !(a == b);
}

string CTxOut::ToString() const
{
	if (scriptPubKey.size() < 6)
		return "CTxOut(error)";
	return strprintf("CTxOut(nValue=%"PRI64d".%08"PRI64d", scriptPubKey=%s)", nValue / COIN, nValue % COIN, scriptPubKey.ToString().substr(0,30).c_str());
}

void CTxOut::print() const
{
	printf("%s\n", ToString().c_str());
}
