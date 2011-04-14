#include "CTransaction.h"
#include "CTxIn.h"
#include "CTxOut.h"
#include "CTxDB.h"
#include "main.h"
#include "script.h"
#include "CCriticalBlock.h"
#include "CInPoint.h"
#include "COutPoint.h"
#include "wallet.h"

//////////////////////////////////////////////////////////////////////////////
//
// CTransaction
//

CTransaction::CTransaction()
{
	SetNull();
}

void CTransaction::SetNull()
{
	nVersion = 1;
	vin.clear();
	vout.clear();
	nLockTime = 0;
}

bool CTransaction::IsNull() const
{
	return (vin.empty() && vout.empty());
}

uint256 CTransaction::GetHash() const
{
	return SerializeHash(*this);
}

bool CTransaction::IsFinal(int nBlockHeight, int64 nBlockTime) const
{
	// Time based nLockTime implemented in 0.1.6
	if (nLockTime == 0)
		return true;
	if (nBlockHeight == 0)
		nBlockHeight = nBestHeight;
	if (nBlockTime == 0)
		nBlockTime = GetAdjustedTime();
	if ((int64)nLockTime < (nLockTime < 500000000 ? (int64)nBlockHeight : nBlockTime))
		return true;
	foreach(const CTxIn& txin, vin)
		if (!txin.IsFinal())
			return false;
	return true;
}

bool CTransaction::IsNewerThan(const CTransaction& old) const
{
	if (vin.size() != old.vin.size())
		return false;
	for (int i = 0; i < vin.size(); i++)
		if (vin[i].prevout != old.vin[i].prevout)
			return false;

	bool fNewer = false;
	unsigned int nLowest = UINT_MAX;
	for (int i = 0; i < vin.size(); i++)
	{
		if (vin[i].nSequence != old.vin[i].nSequence)
		{
			if (vin[i].nSequence <= nLowest)
			{
				fNewer = false;
				nLowest = vin[i].nSequence;
			}
			if (old.vin[i].nSequence < nLowest)
			{
				fNewer = true;
				nLowest = old.vin[i].nSequence;
			}
		}
	}
	return fNewer;
}

bool CTransaction::IsCoinBase() const
{
	return (vin.size() == 1 && vin[0].prevout.IsNull());
}

int CTransaction::GetSigOpCount() const
{
	int n = 0;
	foreach(const CTxIn& txin, vin)
		n += txin.scriptSig.GetSigOpCount();
	foreach(const CTxOut& txout, vout)
		n += txout.scriptPubKey.GetSigOpCount();
	return n;
}

bool CTransaction::IsStandard() const
{
	foreach(const CTxIn& txin, vin)
		if (!txin.scriptSig.IsPushOnly())
			return error("nonstandard txin: %s", txin.scriptSig.ToString().c_str());
	foreach(const CTxOut& txout, vout)
		if (!::IsStandard(txout.scriptPubKey))
			return error("nonstandard txout: %s", txout.scriptPubKey.ToString().c_str());
	return true;
}

bool CTransaction::IsMine() const
{
	foreach(const CTxOut& txout, vout)
		if (txout.IsMine())
			return true;
	return false;
}

bool CTransaction::IsFromMe() const
{
	return (GetDebit() > 0);
}

int64 CTransaction::GetDebit() const
{
	int64 nDebit = 0;
	foreach(const CTxIn& txin, vin)
	{
		nDebit += txin.GetDebit();
		if (!MoneyRange(nDebit))
			throw runtime_error("CTransaction::GetDebit() : value out of range");
	}
	return nDebit;
}

int64 CTransaction::GetCredit() const
{
	int64 nCredit = 0;
	foreach(const CTxOut& txout, vout)
	{
		nCredit += txout.GetCredit();
		if (!MoneyRange(nCredit))
			throw runtime_error("CTransaction::GetCredit() : value out of range");
	}
	return nCredit;
}

int64 CTransaction::GetChange() const
{
	if (IsCoinBase())
		return 0;
	int64 nChange = 0;
	foreach(const CTxOut& txout, vout)
	{
		nChange += txout.GetChange();
		if (!MoneyRange(nChange))
			throw runtime_error("CTransaction::GetChange() : value out of range");
	}
	return nChange;
}

int64 CTransaction::GetValueOut() const
{
	int64 nValueOut = 0;
	foreach(const CTxOut& txout, vout)
	{
		nValueOut += txout.nValue;
		if (!MoneyRange(txout.nValue) || !MoneyRange(nValueOut))
			throw runtime_error("CTransaction::GetValueOut() : value out of range");
	}
	return nValueOut;
}

int64 CTransaction::GetMinFee(unsigned int nBlockSize, bool fAllowFree) const
{
	// Base fee is 1 cent per kilobyte
	unsigned int nBytes = ::GetSerializeSize(*this, SER_NETWORK);
	unsigned int nNewBlockSize = nBlockSize + nBytes;
	int64 nMinFee = (1 + (int64)nBytes / 1000) * CENT;

	if (fAllowFree)
	{
		if (nBlockSize == 1)
		{
			// Transactions under 10K are free
			// (about 4500bc if made of 50bc inputs)
			if (nBytes < 10000)
				nMinFee = 0;
		}
		else
		{
			// Free transaction area
			if (nNewBlockSize < 27000)
				nMinFee = 0;
		}
	}

	// To limit dust spam, require a 0.01 fee if any output is less than 0.01
	if (nMinFee < CENT)
		foreach(const CTxOut& txout, vout)
			if (txout.nValue < CENT)
				nMinFee = CENT;

	// Raise the price as the block approaches full
	if (nBlockSize != 1 && nNewBlockSize >= MAX_BLOCK_SIZE_GEN/2)
	{
		if (nNewBlockSize >= MAX_BLOCK_SIZE_GEN)
			return MAX_MONEY;
		nMinFee *= MAX_BLOCK_SIZE_GEN / (MAX_BLOCK_SIZE_GEN - nNewBlockSize);
	}

	if (!MoneyRange(nMinFee))
		nMinFee = MAX_MONEY;
	return nMinFee;
}


bool CTransaction::ReadFromDisk(CDiskTxPos pos, FILE** pfileRet)
{
	CAutoFile filein = OpenBlockFile(pos.nFile, 0, pfileRet ? "rb+" : "rb");
	if (!filein)
		return error("CTransaction::ReadFromDisk() : OpenBlockFile failed");

	// Read transaction
	if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
		return error("CTransaction::ReadFromDisk() : fseek failed");
	filein >> *this;

	// Return file pointer
	if (pfileRet)
	{
		if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
			return error("CTransaction::ReadFromDisk() : second fseek failed");
		*pfileRet = filein.release();
	}
	return true;
}

bool operator==(const CTransaction& a, const CTransaction& b)
{
	return (a.nVersion  == b.nVersion &&
			a.vin       == b.vin &&
			a.vout      == b.vout &&
			a.nLockTime == b.nLockTime);
}

bool operator!=(const CTransaction& a, const CTransaction& b)
{
	return !(a == b);
}


string CTransaction::ToString() const
{
	string str;
	str += strprintf("CTransaction(hash=%s, ver=%d, vin.size=%d, vout.size=%d, nLockTime=%d)\n",
		GetHash().ToString().substr(0,10).c_str(),
		nVersion,
		vin.size(),
		vout.size(),
		nLockTime);
	for (int i = 0; i < vin.size(); i++)
		str += "    " + vin[i].ToString() + "\n";
	for (int i = 0; i < vout.size(); i++)
		str += "    " + vout[i].ToString() + "\n";
	return str;
}

void CTransaction::print() const
{
	printf("%s", ToString().c_str());
}


bool CTransaction::AcceptToMemoryPool(bool fCheckInputs, bool* pfMissingInputs)
{
	CTxDB txdb("r");
	return AcceptToMemoryPool(txdb, fCheckInputs, pfMissingInputs);
}


bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet)
{
    SetNull();
    if (!txdb.ReadTxIndex(prevout.hash, txindexRet))
        return false;
    if (!ReadFromDisk(txindexRet.pos))
        return false;
    if (prevout.n >= vout.size())
    {
        SetNull();
        return false;
    }
    return true;
}

bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout)
{
    CTxIndex txindex;
    return ReadFromDisk(txdb, prevout, txindex);
}

bool CTransaction::ReadFromDisk(COutPoint prevout)
{
    CTxDB txdb("r");
    CTxIndex txindex;
    return ReadFromDisk(txdb, prevout, txindex);
}

bool CTransaction::CheckTransaction() const
{
    // Basic checks that don't depend on any context
    if (vin.empty() || vout.empty())
        return error("CTransaction::CheckTransaction() : vin or vout empty");

    // Size limits
    if (::GetSerializeSize(*this, SER_NETWORK) > MAX_BLOCK_SIZE)
        return error("CTransaction::CheckTransaction() : size limits failed");

    // Check for negative or overflow output values
    int64 nValueOut = 0;
    foreach(const CTxOut& txout, vout)
    {
        if (txout.nValue < 0)
            return error("CTransaction::CheckTransaction() : txout.nValue negative");
        if (txout.nValue > MAX_MONEY)
            return error("CTransaction::CheckTransaction() : txout.nValue too high");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return error("CTransaction::CheckTransaction() : txout total out of range");
    }

    if (IsCoinBase())
    {
        if (vin[0].scriptSig.size() < 2 || vin[0].scriptSig.size() > 100)
            return error("CTransaction::CheckTransaction() : coinbase script size");
    }
    else
    {
        foreach(const CTxIn& txin, vin)
            if (txin.prevout.IsNull())
                return error("CTransaction::CheckTransaction() : prevout is null");
    }

    return true;
}

bool CTransaction::AcceptToMemoryPool(CTxDB& txdb, bool fCheckInputs, bool* pfMissingInputs)
{
    if (pfMissingInputs)
        *pfMissingInputs = false;

    if (!CheckTransaction())
        return error("AcceptToMemoryPool() : CheckTransaction failed");

    // Coinbase is only valid in a block, not as a loose transaction
    if (IsCoinBase())
        return error("AcceptToMemoryPool() : coinbase as individual tx");

    // To help v0.1.5 clients who would see it as a negative number
    if ((int64)nLockTime > INT_MAX)
        return error("AcceptToMemoryPool() : not accepting nLockTime beyond 2038 yet");

    // Safety limits
    unsigned int nSize = ::GetSerializeSize(*this, SER_NETWORK);
    if (GetSigOpCount() > 2 || nSize < 100)
        return error("AcceptToMemoryPool() : nonstandard transaction");

    // Rather not work on nonstandard transactions
    if (!IsStandard())
        return error("AcceptToMemoryPool() : nonstandard transaction type");

    // Do we already have it?
    uint256 hash = GetHash();
    CRITICAL_BLOCK(cs_mapTransactions)
        if (mapTransactions.count(hash))
            return false;
    if (fCheckInputs)
        if (txdb.ContainsTx(hash))
            return false;

    // Check for conflicts with in-memory transactions
    CTransaction* ptxOld = NULL;
    for (int i = 0; i < vin.size(); i++)
    {
        COutPoint outpoint = vin[i].prevout;
        if (mapNextTx.count(outpoint))
        {
            // Disable replacement feature for now
            return false;

            // Allow replacing with a newer version of the same transaction
            if (i != 0)
                return false;
            ptxOld = mapNextTx[outpoint].ptx;
            if (ptxOld->IsFinal())
                return false;
            if (!IsNewerThan(*ptxOld))
                return false;
            for (int i = 0; i < vin.size(); i++)
            {
                COutPoint outpoint = vin[i].prevout;
                if (!mapNextTx.count(outpoint) || mapNextTx[outpoint].ptx != ptxOld)
                    return false;
            }
            break;
        }
    }

    if (fCheckInputs)
    {
        // Check against previous transactions
        map<uint256, CTxIndex> mapUnused;
        int64 nFees = 0;
        if (!ConnectInputs(txdb, mapUnused, CDiskTxPos(1,1,1), pindexBest, nFees, false, false))
        {
            if (pfMissingInputs)
                *pfMissingInputs = true;
            return error("AcceptToMemoryPool() : ConnectInputs failed %s", hash.ToString().substr(0,10).c_str());
        }

        // Don't accept it if it can't get into a block
        if (nFees < GetMinFee(1000))
            return error("AcceptToMemoryPool() : not enough fees");

        // Limit free transactions per 10 minutes
        if (nFees < CENT && GetBoolArg("-limitfreerelay"))
        {
            static int64 nNextReset;
            static int64 nFreeCount;
            if (GetTime() > nNextReset)
            {
                nNextReset = GetTime() + 10 * 60;
                nFreeCount = 0;
            }
            if (nFreeCount > 150000 && !IsFromMe())
                return error("AcceptToMemoryPool() : free transaction rejected by rate limiter");
            nFreeCount += nSize;
        }
    }

    // Store transaction in memory
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        if (ptxOld)
        {
            printf("AcceptToMemoryPool() : replacing tx %s with new version\n", ptxOld->GetHash().ToString().c_str());
            ptxOld->RemoveFromMemoryPool();
        }
        AddToMemoryPoolUnchecked();
    }

    ///// are we sure this is ok when loading transactions or restoring block txes
    // If updated, erase old tx from wallet
    if (ptxOld)
        EraseFromWallet(ptxOld->GetHash());

    printf("AcceptToMemoryPool(): accepted %s\n", hash.ToString().substr(0,10).c_str());
    return true;
}


bool CTransaction::AddToMemoryPoolUnchecked()
{
    // Add to memory pool without checking anything.  Don't call this directly,
    // call AcceptToMemoryPool to properly check the transaction first.
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        uint256 hash = GetHash();
        mapTransactions[hash] = *this;
        for (int i = 0; i < vin.size(); i++)
            mapNextTx[vin[i].prevout] = CInPoint(&mapTransactions[hash], i);
        nTransactionsUpdated++;
    }
    return true;
}


bool CTransaction::RemoveFromMemoryPool()
{
    // Remove transaction from memory pool
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        foreach(const CTxIn& txin, vin)
            mapNextTx.erase(txin.prevout);
        mapTransactions.erase(GetHash());
        nTransactionsUpdated++;
    }
    return true;
}
bool CTransaction::DisconnectInputs(CTxDB& txdb)
{
    // Relinquish previous transactions' spent pointers
    if (!IsCoinBase())
    {
        foreach(const CTxIn& txin, vin)
        {
            COutPoint prevout = txin.prevout;

            // Get prev txindex from disk
            CTxIndex txindex;
            if (!txdb.ReadTxIndex(prevout.hash, txindex))
                return error("DisconnectInputs() : ReadTxIndex failed");

            if (prevout.n >= txindex.vSpent.size())
                return error("DisconnectInputs() : prevout.n out of range");

            // Mark outpoint as not spent
            txindex.vSpent[prevout.n].SetNull();

            // Write back
            if (!txdb.UpdateTxIndex(prevout.hash, txindex))
                return error("DisconnectInputs() : UpdateTxIndex failed");
        }
    }

    // Remove transaction from index
    if (!txdb.EraseTxIndex(*this))
        return error("DisconnectInputs() : EraseTxPos failed");

    return true;
}


bool CTransaction::ConnectInputs(CTxDB& txdb, map<uint256, CTxIndex>& mapTestPool, CDiskTxPos posThisTx,
                                 CBlockIndex* pindexBlock, int64& nFees, bool fBlock, bool fMiner, int64 nMinFee)
{
    // Take over previous transactions' spent pointers
    if (!IsCoinBase())
    {
        int64 nValueIn = 0;
        for (int i = 0; i < vin.size(); i++)
        {
            COutPoint prevout = vin[i].prevout;

            // Read txindex
            CTxIndex txindex;
            bool fFound = true;
            if (fMiner && mapTestPool.count(prevout.hash))
            {
                // Get txindex from current proposed changes
                txindex = mapTestPool[prevout.hash];
            }
            else
            {
                // Read txindex from txdb
                fFound = txdb.ReadTxIndex(prevout.hash, txindex);
            }
            if (!fFound && (fBlock || fMiner))
                return fMiner ? false : error("ConnectInputs() : %s prev tx %s index entry not found", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());

            // Read txPrev
            CTransaction txPrev;
            if (!fFound || txindex.pos == CDiskTxPos(1,1,1))
            {
                // Get prev tx from single transactions in memory
                CRITICAL_BLOCK(cs_mapTransactions)
                {
                    if (!mapTransactions.count(prevout.hash))
                        return error("ConnectInputs() : %s mapTransactions prev not found %s", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());
                    txPrev = mapTransactions[prevout.hash];
                }
                if (!fFound)
                    txindex.vSpent.resize(txPrev.vout.size());
            }
            else
            {
                // Get prev tx from disk
                if (!txPrev.ReadFromDisk(txindex.pos))
                    return error("ConnectInputs() : %s ReadFromDisk prev tx %s failed", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());
            }

            if (prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
                return error("ConnectInputs() : %s prevout.n out of range %d %d %d prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str());

            // If prev is coinbase, check that it's matured
            if (txPrev.IsCoinBase())
                for (CBlockIndex* pindex = pindexBlock; pindex && pindexBlock->nHeight - pindex->nHeight < COINBASE_MATURITY; pindex = pindex->pprev)
                    if (pindex->nBlockPos == txindex.pos.nBlockPos && pindex->nFile == txindex.pos.nFile)
                        return error("ConnectInputs() : tried to spend coinbase at depth %d", pindexBlock->nHeight - pindex->nHeight);

            // Verify signature
            if (!VerifySignature(txPrev, *this, i))
                return error("ConnectInputs() : %s VerifySignature failed", GetHash().ToString().substr(0,10).c_str());

            // Check for conflicts
            if (!txindex.vSpent[prevout.n].IsNull())
                return fMiner ? false : error("ConnectInputs() : %s prev tx already used at %s", GetHash().ToString().substr(0,10).c_str(), txindex.vSpent[prevout.n].ToString().c_str());

            // Check for negative or overflow input values
            nValueIn += txPrev.vout[prevout.n].nValue;
            if (!MoneyRange(txPrev.vout[prevout.n].nValue) || !MoneyRange(nValueIn))
                return error("ConnectInputs() : txin values out of range");

            // Mark outpoints as spent
            txindex.vSpent[prevout.n] = posThisTx;

            // Write back
            if (fBlock)
            {
                if (!txdb.UpdateTxIndex(prevout.hash, txindex))
                    return error("ConnectInputs() : UpdateTxIndex failed");
            }
            else if (fMiner)
            {
                mapTestPool[prevout.hash] = txindex;
            }
        }

        if (nValueIn < GetValueOut())
            return error("ConnectInputs() : %s value in < value out", GetHash().ToString().substr(0,10).c_str());

        // Tally transaction fees
        int64 nTxFee = nValueIn - GetValueOut();
        if (nTxFee < 0)
            return error("ConnectInputs() : %s nTxFee < 0", GetHash().ToString().substr(0,10).c_str());
        if (nTxFee < nMinFee)
            return false;
        nFees += nTxFee;
        if (!MoneyRange(nFees))
            return error("ConnectInputs() : nFees out of range");
    }

    if (fBlock)
    {
        // Add transaction to disk index
        if (!txdb.AddTxIndex(*this, posThisTx, pindexBlock->nHeight))
            return error("ConnectInputs() : AddTxPos failed");
    }
    else if (fMiner)
    {
        // Add transaction to test pool
        mapTestPool[GetHash()] = CTxIndex(CDiskTxPos(1,1,1), vout.size());
    }

    return true;
}


bool CTransaction::ClientConnectInputs()
{
    if (IsCoinBase())
        return false;

    // Take over previous transactions' spent pointers
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        int64 nValueIn = 0;
        for (int i = 0; i < vin.size(); i++)
        {
            // Get prev tx from single transactions in memory
            COutPoint prevout = vin[i].prevout;
            if (!mapTransactions.count(prevout.hash))
                return false;
            CTransaction& txPrev = mapTransactions[prevout.hash];

            if (prevout.n >= txPrev.vout.size())
                return false;

            // Verify signature
            if (!VerifySignature(txPrev, *this, i))
                return error("ConnectInputs() : VerifySignature failed");

            ///// this is redundant with the mapNextTx stuff, not sure which I want to get rid of
            ///// this has to go away now that posNext is gone
            // // Check for conflicts
            // if (!txPrev.vout[prevout.n].posNext.IsNull())
            //     return error("ConnectInputs() : prev tx already used");
            //
            // // Flag outpoints as used
            // txPrev.vout[prevout.n].posNext = posThisTx;

            nValueIn += txPrev.vout[prevout.n].nValue;

            if (!MoneyRange(txPrev.vout[prevout.n].nValue) || !MoneyRange(nValueIn))
                return error("ClientConnectInputs() : txin values out of range");
        }
        if (GetValueOut() > nValueIn)
            return false;
    }

    return true;
}
