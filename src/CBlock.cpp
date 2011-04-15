#include "CBlock.h"
#include "CTransaction.h"
#include "main.h"
#include "peer.h"
#include "wallet.h"
#include "CMessageHeader.h"
#include "CInv.h"
#include "CNode.h"
#include "CTxDB.h"
#include "CDiskBlockIndex.h"

CBlock::CBlock()
{
	SetNull();
}

void CBlock::SetNull()
{
	nVersion = 1;
	hashPrevBlock = 0;
	hashMerkleRoot = 0;
	nTime = 0;
	nBits = 0;
	nNonce = 0;
	vtx.clear();
	vMerkleTree.clear();
}

bool CBlock::IsNull() const
{
	return (nBits == 0);
}

uint256 CBlock::GetHash() const
{
	return Hash(BEGIN(nVersion), END(nNonce));
}

int64 CBlock::GetBlockTime() const
{
	return (int64)nTime;
}

int CBlock::GetSigOpCount() const
{
	int n = 0;
	foreach(const CTransaction& tx, vtx)
		n += tx.GetSigOpCount();
	return n;
}

uint256 CBlock::BuildMerkleTree() const
{
	vMerkleTree.clear();
	foreach(const CTransaction& tx, vtx)
		vMerkleTree.push_back(tx.GetHash());
	int j = 0;
	for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
	{
		for (int i = 0; i < nSize; i += 2)
		{
			int i2 = min(i+1, nSize-1);
			vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
									   BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
		}
		j += nSize;
	}
	return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
}

vector<uint256> CBlock::GetMerkleBranch(int nIndex) const
{
	if (vMerkleTree.empty())
		BuildMerkleTree();
	vector<uint256> vMerkleBranch;
	int j = 0;
	for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
	{
		int i = min(nIndex^1, nSize-1);
		vMerkleBranch.push_back(vMerkleTree[j+i]);
		nIndex >>= 1;
		j += nSize;
	}
	return vMerkleBranch;
}

uint256 CBlock::CheckMerkleBranch(uint256 hash, const vector<uint256>& vMerkleBranch, int nIndex)
{
	if (nIndex == -1)
		return 0;
	foreach(const uint256& otherside, vMerkleBranch)
	{
		if (nIndex & 1)
			hash = Hash(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
		else
			hash = Hash(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
		nIndex >>= 1;
	}
	return hash;
}

bool CBlock::WriteToDisk(unsigned int& nFileRet, unsigned int& nBlockPosRet)
{
	// Open history file to append
	CAutoFile fileout = AppendBlockFile(nFileRet);
	if (!fileout)
		return error("CBlock::WriteToDisk() : AppendBlockFile failed");

	// Write index header
	unsigned int nSize = fileout.GetSerializeSize(*this);
	fileout << FLATDATA(pchMessageStart) << nSize;

	// Write block
	nBlockPosRet = ftell(fileout);
	if (nBlockPosRet == -1)
		return error("CBlock::WriteToDisk() : ftell failed");
	fileout << *this;

	// Flush stdio buffers and commit to disk before returning
	fflush(fileout);
	if (!IsInitialBlockDownload() || (nBestHeight+1) % 500 == 0)
	{
#ifdef __WXMSW__
		_commit(_fileno(fileout));
#else
		fsync(fileno(fileout));
#endif
	}

	return true;
}

bool CBlock::ReadFromDisk(unsigned int nFile, unsigned int nBlockPos, bool fReadTransactions)
{
	SetNull();

	// Open history file to read
	CAutoFile filein = OpenBlockFile(nFile, nBlockPos, "rb");
	if (!filein)
		return error("CBlock::ReadFromDisk() : OpenBlockFile failed");
	if (!fReadTransactions)
		filein.nType |= SER_BLOCKHEADERONLY;

	// Read block
	filein >> *this;

	// Check the header
	if (!CheckProofOfWork(GetHash(), nBits))
		return error("CBlock::ReadFromDisk() : errors in block header");

	return true;
}

void CBlock::print() const
{
	printf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%d)\n",
		GetHash().ToString().substr(0,20).c_str(),
		nVersion,
		hashPrevBlock.ToString().substr(0,20).c_str(),
		hashMerkleRoot.ToString().substr(0,10).c_str(),
		nTime, nBits, nNonce,
		vtx.size());
	for (int i = 0; i < vtx.size(); i++)
	{
		printf("  ");
		vtx[i].print();
	}
	printf("  vMerkleTree: ");
	for (int i = 0; i < vMerkleTree.size(); i++)
		printf("%s ", vMerkleTree[i].ToString().substr(0,10).c_str());
	printf("\n");
}

bool CBlock::ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions)
{
    if (!fReadTransactions)
    {
        *this = pindex->GetBlockHeader();
        return true;
    }
    if (!ReadFromDisk(pindex->nFile, pindex->nBlockPos, fReadTransactions))
        return false;
    if (GetHash() != pindex->GetBlockHash())
        return error("CBlock::ReadFromDisk() : GetHash() doesn't match index");
    return true;
}

bool CBlock::DisconnectBlock(CTxDB& txdb, CBlockIndex* pindex)
{
    // Disconnect in reverse order
    for (int i = vtx.size()-1; i >= 0; i--)
        if (!vtx[i].DisconnectInputs(txdb))
            return false;

    // Update block index on disk without changing it in memory.
    // The memory index structure will be changed after the db commits.
    if (pindex->pprev)
    {
        CDiskBlockIndex blockindexPrev(pindex->pprev);
        blockindexPrev.hashNext = 0;
        if (!txdb.WriteBlockIndex(blockindexPrev))
            return error("DisconnectBlock() : WriteBlockIndex failed");
    }

    return true;
}

bool CBlock::ConnectBlock(CTxDB& txdb, CBlockIndex* pindex)
{
    // Check it again in case a previous version let a bad block in
    if (!CheckBlock())
        return false;

    //// issue here: it doesn't know the version
    unsigned int nTxPos = pindex->nBlockPos + ::GetSerializeSize(CBlock(), SER_DISK) - 1 + GetSizeOfCompactSize(vtx.size());

    map<uint256, CTxIndex> mapUnused;
    int64 nFees = 0;
    foreach(CTransaction& tx, vtx)
    {
        CDiskTxPos posThisTx(pindex->nFile, pindex->nBlockPos, nTxPos);
        nTxPos += ::GetSerializeSize(tx, SER_DISK);

        if (!tx.ConnectInputs(txdb, mapUnused, posThisTx, pindex, nFees, true, false))
            return false;
    }

    if (vtx[0].GetValueOut() > GetBlockValue(pindex->nHeight, nFees))
        return false;

    // Update block index on disk without changing it in memory.
    // The memory index structure will be changed after the db commits.
    if (pindex->pprev)
    {
        CDiskBlockIndex blockindexPrev(pindex->pprev);
        blockindexPrev.hashNext = pindex->GetBlockHash();
        if (!txdb.WriteBlockIndex(blockindexPrev))
            return error("ConnectBlock() : WriteBlockIndex failed");
    }

    // Watch for transactions paying to me
    foreach(CTransaction& tx, vtx)
        AddToWalletIfMine(tx, this);

    return true;
}

bool CBlock::SetBestChain(CTxDB& txdb, CBlockIndex* pindexNew)
{
    uint256 hash = GetHash();

    txdb.TxnBegin();
    if (pindexGenesisBlock == NULL && hash == hashGenesisBlock)
    {
        txdb.WriteHashBestChain(hash);
        if (!txdb.TxnCommit())
            return error("SetBestChain() : TxnCommit failed");
        pindexGenesisBlock = pindexNew;
    }
    else if (hashPrevBlock == hashBestChain)
    {
        // Adding to current best branch
        if (!ConnectBlock(txdb, pindexNew) || !txdb.WriteHashBestChain(hash))
        {
            txdb.TxnAbort();
            InvalidChainFound(pindexNew);
            return error("SetBestChain() : ConnectBlock failed");
        }
        if (!txdb.TxnCommit())
            return error("SetBestChain() : TxnCommit failed");

        // Add to current best branch
        pindexNew->pprev->pnext = pindexNew;

        // Delete redundant memory transactions
        foreach(CTransaction& tx, vtx)
            tx.RemoveFromMemoryPool();
    }
    else
    {
        // New best branch
        if (!Reorganize(txdb, pindexNew))
        {
            txdb.TxnAbort();
            InvalidChainFound(pindexNew);
            return error("SetBestChain() : Reorganize failed");
        }
    }

    // New best block
    hashBestChain = hash;
    pindexBest = pindexNew;
    nBestHeight = pindexBest->nHeight;
    bnBestChainWork = pindexNew->bnChainWork;
    nTimeBestReceived = GetTime();
    nTransactionsUpdated++;
    printf("SetBestChain: new best=%s  height=%d  work=%s\n", hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, bnBestChainWork.ToString().c_str());

    return true;
}

bool CBlock::AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos)
{
    // Check for duplicate
    uint256 hash = GetHash();
    if (mapBlockIndex.count(hash))
        return error("AddToBlockIndex() : %s already exists", hash.ToString().substr(0,20).c_str());

    // Construct new block index object
    CBlockIndex* pindexNew = new CBlockIndex(nFile, nBlockPos, *this);
    if (!pindexNew)
        return error("AddToBlockIndex() : new CBlockIndex failed");
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
    pindexNew->phashBlock = &((*mi).first);
    map<uint256, CBlockIndex*>::iterator miPrev = mapBlockIndex.find(hashPrevBlock);
    if (miPrev != mapBlockIndex.end())
    {
        pindexNew->pprev = (*miPrev).second;
        pindexNew->nHeight = pindexNew->pprev->nHeight + 1;
    }
    pindexNew->bnChainWork = (pindexNew->pprev ? pindexNew->pprev->bnChainWork : 0) + pindexNew->GetBlockWork();

    CTxDB txdb;
    txdb.TxnBegin();
    txdb.WriteBlockIndex(CDiskBlockIndex(pindexNew));
    if (!txdb.TxnCommit())
        return false;

    // New best
    if (pindexNew->bnChainWork > bnBestChainWork)
        if (!SetBestChain(txdb, pindexNew))
            return false;

    txdb.Close();

    if (pindexNew == pindexBest)
    {
        // Notify UI to display prev block's coinbase if it was ours
        static uint256 hashPrevBestCoinBase;
        CRITICAL_BLOCK(cs_mapWallet)
            vWalletUpdated.push_back(hashPrevBestCoinBase);
        hashPrevBestCoinBase = vtx[0].GetHash();
    }

    MainFrameRepaint();
    return true;
}

bool CBlock::CheckBlock() const
{
    // These are checks that are independent of context
    // that can be verified before saving an orphan block.

    // Size limits
    if (vtx.empty() || vtx.size() > MAX_BLOCK_SIZE || ::GetSerializeSize(*this, SER_NETWORK) > MAX_BLOCK_SIZE)
        return error("CheckBlock() : size limits failed");

    // Check proof of work matches claimed amount
    if (!CheckProofOfWork(GetHash(), nBits))
        return error("CheckBlock() : proof of work failed");

    // Check timestamp
    if (GetBlockTime() > GetAdjustedTime() + 2 * 60 * 60)
        return error("CheckBlock() : block timestamp too far in the future");

    // First transaction must be coinbase, the rest must not be
    if (vtx.empty() || !vtx[0].IsCoinBase())
        return error("CheckBlock() : first tx is not coinbase");
    for (int i = 1; i < vtx.size(); i++)
        if (vtx[i].IsCoinBase())
            return error("CheckBlock() : more than one coinbase");

    // Check transactions
    foreach(const CTransaction& tx, vtx)
        if (!tx.CheckTransaction())
            return error("CheckBlock() : CheckTransaction failed");

    // Check that it's not full of nonstandard transactions
    if (GetSigOpCount() > MAX_BLOCK_SIGOPS)
        return error("CheckBlock() : too many nonstandard transactions");

    // Check merkleroot
    if (hashMerkleRoot != BuildMerkleTree())
        return error("CheckBlock() : hashMerkleRoot mismatch");

    return true;
}

bool CBlock::AcceptBlock()
{
    // Check for duplicate
    uint256 hash = GetHash();
    if (mapBlockIndex.count(hash))
        return error("AcceptBlock() : block already in mapBlockIndex");

    // Get prev block index
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);
    if (mi == mapBlockIndex.end())
        return error("AcceptBlock() : prev block not found");
    CBlockIndex* pindexPrev = (*mi).second;
    int nHeight = pindexPrev->nHeight+1;

    // Check proof of work
    if (nBits != GetNextWorkRequired(pindexPrev))
        return error("AcceptBlock() : incorrect proof of work");

    // Check timestamp against prev
    if (GetBlockTime() <= pindexPrev->GetMedianTimePast())
        return error("AcceptBlock() : block's timestamp is too early");

    // Check that all transactions are finalized
    foreach(const CTransaction& tx, vtx)
        if (!tx.IsFinal(nHeight, GetBlockTime()))
            return error("AcceptBlock() : contains a non-final transaction");

    // Check that the block chain matches the known block chain up to a checkpoint
    if (!fTestNet)
        if ((nHeight ==  11111 && hash != uint256("0x0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d")) ||
            (nHeight ==  33333 && hash != uint256("0x000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6")) ||
            (nHeight ==  68555 && hash != uint256("0x00000000001e1b4903550a0b96e9a9405c8a95f387162e4944e8d9fbe501cd6a")) ||
            (nHeight ==  70567 && hash != uint256("0x00000000006a49b14bcf27462068f1264c961f11fa2e0eddd2be0791e1d4124a")) ||
            (nHeight ==  74000 && hash != uint256("0x0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20")) ||
            (nHeight == 105000 && hash != uint256("0x00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97")) ||
            (nHeight == 118000 && hash != uint256("0x000000000000774a7f8a7a12dc906ddb9e17e75d684f15e00f8767f9e8f36553")))

            return error("AcceptBlock() : rejected by checkpoint lockin at %d", nHeight);

    // Write block to history file
    if (!CheckDiskSpace(::GetSerializeSize(*this, SER_DISK)))
        return error("AcceptBlock() : out of disk space");
    unsigned int nFile = -1;
    unsigned int nBlockPos = 0;
    if (!WriteToDisk(nFile, nBlockPos))
        return error("AcceptBlock() : WriteToDisk failed");
    if (!AddToBlockIndex(nFile, nBlockPos))
        return error("AcceptBlock() : AddToBlockIndex failed");

    // Relay inventory, but don't relay old inventory during initial block download
    if (hashBestChain == hash)
        CRITICAL_BLOCK(cs_vNodes)
            foreach(CNode* pnode, vNodes)
                if (nBestHeight > (pnode->nStartingHeight != -1 ? pnode->nStartingHeight - 2000 : 118000))
                    pnode->PushInventory(CInv(MSG_BLOCK, hash));

    return true;
}
