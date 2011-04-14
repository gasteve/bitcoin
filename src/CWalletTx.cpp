#include "CWalletTx.h"
#include "headers.h"
#include "main.h"
#include "db.h"
#include "peer.h"
#include "uint256.h"
#include "base58.h"
#include "script.h"
#include "CCriticalBlock.h"
#include "CInv.h"
#include "CTxDB.h"

CWalletTx::CWalletTx()
{
	Init();
}

CWalletTx::CWalletTx(const CMerkleTx& txIn) : CMerkleTx(txIn)
{
	Init();
}

CWalletTx::CWalletTx(const CTransaction& txIn) : CMerkleTx(txIn)
{
	Init();
}

void CWalletTx::Init()
{
	vtxPrev.clear();
	mapValue.clear();
	vOrderForm.clear();
	fTimeReceivedIsTxTime = false;
	nTimeReceived = 0;
	fFromMe = false;
	fSpent = false;
	strFromAccount.clear();
	fDebitCached = false;
	fCreditCached = false;
	fChangeCached = false;
	nDebitCached = 0;
	nCreditCached = 0;
	nChangeCached = 0;
	nTimeDisplayed = 0;
	nLinesDisplayed = 0;
	fConfirmedDisplayed = false;
}

int64 CWalletTx::GetDebit() const
{
	if (vin.empty())
		return 0;
	if (fDebitCached)
		return nDebitCached;
	nDebitCached = CTransaction::GetDebit();
	fDebitCached = true;
	return nDebitCached;
}

int64 CWalletTx::GetCredit(bool fUseCache) const
{
	// Must wait until coinbase is safely deep enough in the chain before valuing it
	if (IsCoinBase() && GetBlocksToMaturity() > 0)
		return 0;

	// GetBalance can assume transactions in mapWallet won't change
	if (fUseCache && fCreditCached)
		return nCreditCached;
	nCreditCached = CTransaction::GetCredit();
	fCreditCached = true;
	return nCreditCached;
}

int64 CWalletTx::GetChange() const
{
	if (fChangeCached)
		return nChangeCached;
	nChangeCached = CTransaction::GetChange();
	fChangeCached = true;
	return nChangeCached;
}

void CWalletTx::GetAmounts(int64& nGenerated, list<pair<string, int64> >& listReceived,
                           list<pair<string, int64> >& listSent, int64& nFee, string& strSentAccount) const
{
    nGenerated = nFee = 0;
    listReceived.clear();
    listSent.clear();
    strSentAccount = strFromAccount;

    if (IsCoinBase())
    {
        if (GetDepthInMainChain() >= COINBASE_MATURITY)
            nGenerated = GetCredit();
        return;
    }

    // Compute fee:
    int64 nDebit = GetDebit();
    if (nDebit > 0) // debit>0 means we signed/sent this transaction
    {
        int64 nValueOut = GetValueOut();
        nFee = nDebit - nValueOut;
    }

    // Sent/received.  Standard client will never generate a send-to-multiple-recipients,
    // but non-standard clients might (so return a list of address/amount pairs)
    foreach(const CTxOut& txout, vout)
    {
        string address;
        uint160 hash160;
        vector<unsigned char> vchPubKey;
        if (ExtractHash160(txout.scriptPubKey, hash160))
            address = Hash160ToAddress(hash160);
        else if (ExtractPubKey(txout.scriptPubKey, false, vchPubKey))
            address = PubKeyToAddress(vchPubKey);
        else
        {
            printf("CWalletTx::GetAmounts: Unknown transaction type found, txid %s\n",
                   this->GetHash().ToString().c_str());
            address = " unknown ";
        }

        // Don't report 'change' txouts
        if (nDebit > 0 && txout.IsChange())
            continue;

        if (nDebit > 0)
            listSent.push_back(make_pair(address, txout.nValue));

        if (txout.IsMine())
            listReceived.push_back(make_pair(address, txout.nValue));
    }

}

void CWalletTx::GetAccountAmounts(const string& strAccount, int64& nGenerated, int64& nReceived, 
                                  int64& nSent, int64& nFee) const
{
    nGenerated = nReceived = nSent = nFee = 0;

    int64 allGenerated, allFee;
    allGenerated = allFee = 0;
    string strSentAccount;
    list<pair<string, int64> > listReceived;
    list<pair<string, int64> > listSent;
    GetAmounts(allGenerated, listReceived, listSent, allFee, strSentAccount);

    if (strAccount == "")
        nGenerated = allGenerated;
    if (strAccount == strSentAccount)
    {
        foreach(const PAIRTYPE(string,int64)& s, listSent)
            nSent += s.second;
        nFee = allFee;
    }
    CRITICAL_BLOCK(cs_mapAddressBook)
    {
        foreach(const PAIRTYPE(string,int64)& r, listReceived)
        {
            if (mapAddressBook.count(r.first))
            {
                if (mapAddressBook[r.first] == strAccount)
                {
                    nReceived += r.second;
                }
            }
            else if (strAccount.empty())
            {
                nReceived += r.second;
            }
        }
    }
}

bool CWalletTx::IsFromMe() const
    {
        return (GetDebit() > 0);
    }

bool CWalletTx::IsConfirmed() const
{
	// Quick answer in most cases
	if (!IsFinal())
		return false;
	if (GetDepthInMainChain() >= 1)
		return true;
	if (!IsFromMe()) // using wtx's cached debit
		return false;

	// If no confirmations but it's from us, we can still
	// consider it confirmed if all dependencies are confirmed
	map<uint256, const CMerkleTx*> mapPrev;
	vector<const CMerkleTx*> vWorkQueue;
	vWorkQueue.reserve(vtxPrev.size()+1);
	vWorkQueue.push_back(this);
	for (int i = 0; i < vWorkQueue.size(); i++)
	{
		const CMerkleTx* ptx = vWorkQueue[i];

		if (!ptx->IsFinal())
			return false;
		if (ptx->GetDepthInMainChain() >= 1)
			return true;
		if (!ptx->IsFromMe())
			return false;

		if (mapPrev.empty())
			foreach(const CMerkleTx& tx, vtxPrev)
				mapPrev[tx.GetHash()] = &tx;

		foreach(const CTxIn& txin, ptx->vin)
		{
			if (!mapPrev.count(txin.prevout.hash))
				return false;
			vWorkQueue.push_back(mapPrev[txin.prevout.hash]);
		}
	}
	return true;
}

bool CWalletTx::WriteToDisk()
{
	return CWalletDB().WriteTx(GetHash(), *this);
}

int64 CWalletTx::GetTxTime() const
{
    if (!fTimeReceivedIsTxTime && hashBlock != 0)
    {
        // If we did not receive the transaction directly, we rely on the block's
        // time to figure out when it happened.  We use the median over a range
        // of blocks to try to filter out inaccurate block times.
        map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end())
        {
            CBlockIndex* pindex = (*mi).second;
            if (pindex)
                return pindex->GetMedianTime();
        }
    }
    return nTimeReceived;
}

int CWalletTx::GetRequestCount() const
{
    // Returns -1 if it wasn't being tracked
    int nRequests = -1;
    CRITICAL_BLOCK(cs_mapRequestCount)
    {
        if (IsCoinBase())
        {
            // Generated block
            if (hashBlock != 0)
            {
                map<uint256, int>::iterator mi = mapRequestCount.find(hashBlock);
                if (mi != mapRequestCount.end())
                    nRequests = (*mi).second;
            }
        }
        else
        {
            // Did anyone request this transaction?
            map<uint256, int>::iterator mi = mapRequestCount.find(GetHash());
            if (mi != mapRequestCount.end())
            {
                nRequests = (*mi).second;

                // How about the block it's in?
                if (nRequests == 0 && hashBlock != 0)
                {
                    map<uint256, int>::iterator mi = mapRequestCount.find(hashBlock);
                    if (mi != mapRequestCount.end())
                        nRequests = (*mi).second;
                    else
                        nRequests = 1; // If it's in someone else's block it must have got out
                }
            }
        }
    }
    return nRequests;
}

void CWalletTx::AddSupportingTransactions(CTxDB& txdb)
{
    vtxPrev.clear();

    const int COPY_DEPTH = 3;
    if (SetMerkleBranch() < COPY_DEPTH)
    {
        vector<uint256> vWorkQueue;
        foreach(const CTxIn& txin, vin)
            vWorkQueue.push_back(txin.prevout.hash);

        // This critsect is OK because txdb is already open
        CRITICAL_BLOCK(cs_mapWallet)
        {
            map<uint256, const CMerkleTx*> mapWalletPrev;
            set<uint256> setAlreadyDone;
            for (int i = 0; i < vWorkQueue.size(); i++)
            {
                uint256 hash = vWorkQueue[i];
                if (setAlreadyDone.count(hash))
                    continue;
                setAlreadyDone.insert(hash);

                CMerkleTx tx;
                if (mapWallet.count(hash))
                {
                    tx = mapWallet[hash];
                    foreach(const CMerkleTx& txWalletPrev, mapWallet[hash].vtxPrev)
                        mapWalletPrev[txWalletPrev.GetHash()] = &txWalletPrev;
                }
                else if (mapWalletPrev.count(hash))
                {
                    tx = *mapWalletPrev[hash];
                }
                else if (!fClient && txdb.ReadDiskTx(hash, tx))
                {
                    ;
                }
                else
                {
                    printf("ERROR: AddSupportingTransactions() : unsupported transaction\n");
                    continue;
                }

                int nDepth = tx.SetMerkleBranch();
                vtxPrev.push_back(tx);

                if (nDepth < COPY_DEPTH)
                    foreach(const CTxIn& txin, tx.vin)
                        vWorkQueue.push_back(txin.prevout.hash);
            }
        }
    }

    reverse(vtxPrev.begin(), vtxPrev.end());
}

bool CWalletTx::AcceptWalletTransaction() { CTxDB txdb("r"); return AcceptWalletTransaction(txdb); }
bool CWalletTx::AcceptWalletTransaction(CTxDB& txdb, bool fCheckInputs)
{
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        // Add previous supporting transactions first
        foreach(CMerkleTx& tx, vtxPrev)
        {
            if (!tx.IsCoinBase())
            {
                uint256 hash = tx.GetHash();
                if (!mapTransactions.count(hash) && !txdb.ContainsTx(hash))
                    tx.AcceptToMemoryPool(txdb, fCheckInputs);
            }
        }
        return AcceptToMemoryPool(txdb, fCheckInputs);
    }
    return false;
}


void CWalletTx::RelayWalletTransaction() { CTxDB txdb("r"); RelayWalletTransaction(txdb); }
void CWalletTx::RelayWalletTransaction(CTxDB& txdb)
{
    foreach(const CMerkleTx& tx, vtxPrev)
    {
        if (!tx.IsCoinBase())
        {
            uint256 hash = tx.GetHash();
            if (!txdb.ContainsTx(hash))
                RelayMessage(CInv(MSG_TX, hash), (CTransaction)tx);
        }
    }
    if (!IsCoinBase())
    {
        uint256 hash = GetHash();
        if (!txdb.ContainsTx(hash))
        {
            printf("Relaying wtx %s\n", hash.ToString().substr(0,10).c_str());
            RelayMessage(CInv(MSG_TX, hash), (CTransaction)*this);
        }
    }
}

