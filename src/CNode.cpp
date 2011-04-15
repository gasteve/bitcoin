#include "CNode.h"

#include "peer.h"
#include "CMessageHeader.h"
#include "CBlockLocator.h"

CNode::CNode(SOCKET hSocketIn, CAddress addrIn, bool fInboundIn)
{
	nServices = 0;
	hSocket = hSocketIn;
	vSend.SetType(SER_NETWORK);
	vSend.SetVersion(0);
	vRecv.SetType(SER_NETWORK);
	vRecv.SetVersion(0);
	// Version 0.2 obsoletes 20 Feb 2012
	if (GetTime() > 1329696000)
	{
		vSend.SetVersion(209);
		vRecv.SetVersion(209);
	}
	nLastSend = 0;
	nLastRecv = 0;
	nLastSendEmpty = GetTime();
	nTimeConnected = GetTime();
	nHeaderStart = -1;
	nMessageStart = -1;
	addr = addrIn;
	nVersion = 0;
	strSubVer = "";
	fClient = false; // set by version message
	fInbound = fInboundIn;
	fNetworkNode = false;
	fSuccessfullyConnected = false;
	fDisconnect = false;
	nRefCount = 0;
	nReleaseTime = 0;
	hashContinue = 0;
	pindexLastGetBlocksBegin = 0;
	hashLastGetBlocksEnd = 0;
	nStartingHeight = -1;
	fGetAddr = false;
	vfSubscribe.assign(256, false);

        // Be shy and don't send version until we hear
        if (!fInbound)
            PushVersion();
}

CNode::~CNode()
{
	if (hSocket != INVALID_SOCKET)
	{
		closesocket(hSocket);
		hSocket = INVALID_SOCKET;
	}
}

int CNode::GetRefCount()
{
	return max(nRefCount, 0) + (GetTime() < nReleaseTime ? 1 : 0);
}

CNode* CNode::AddRef(int64 nTimeout)
{
	if (nTimeout != 0)
		nReleaseTime = max(nReleaseTime, GetTime() + nTimeout);
	else
		nRefCount++;
	return this;
}

void CNode::Release()
{
	nRefCount--;
}

void CNode::AddAddressKnown(const CAddress& addr)
{
	setAddrKnown.insert(addr);
}

void CNode::PushAddress(const CAddress& addr)
{
	// Known checking here is only to save space from duplicates.
	// SendMessages will filter it again for knowns that were added
	// after addresses were pushed.
	if (addr.IsValid() && !setAddrKnown.count(addr))
		vAddrToSend.push_back(addr);
}

void CNode::AddInventoryKnown(const CInv& inv)
{
	CRITICAL_BLOCK(cs_inventory)
		setInventoryKnown.insert(inv);
}

void CNode::PushInventory(const CInv& inv)
{
	CRITICAL_BLOCK(cs_inventory)
		if (!setInventoryKnown.count(inv))
			vInventoryToSend.push_back(inv);
}

void CNode::AskFor(const CInv& inv)
{
	// We're using mapAskFor as a priority queue,
	// the key is the earliest time the request can be sent
	int64& nRequestTime = mapAlreadyAskedFor[inv];
	printf("askfor %s   %"PRI64d"\n", inv.ToString().c_str(), nRequestTime);

	// Make sure not to reuse time indexes to keep things in the same order
	int64 nNow = (GetTime() - 1) * 1000000;
	static int64 nLastTime;
	nLastTime = nNow = max(nNow, ++nLastTime);

	// Each retry is 2 minutes after the last
	nRequestTime = max(nRequestTime + 2 * 60 * 1000000, nNow);
	mapAskFor.insert(make_pair(nRequestTime, inv));
}

void CNode::BeginMessage(const char* pszCommand)
{
	cs_vSend.Enter();
	if (nHeaderStart != -1)
		AbortMessage();
	nHeaderStart = vSend.size();
	vSend << CMessageHeader(pszCommand, 0);
	nMessageStart = vSend.size();
	if (fDebug)
		printf("%s ", DateTimeStrFormat("%x %H:%M:%S", GetTime()).c_str());
	printf("sending: %s ", pszCommand);
}

void CNode::AbortMessage()
{
	if (nHeaderStart == -1)
		return;
	vSend.resize(nHeaderStart);
	nHeaderStart = -1;
	nMessageStart = -1;
	cs_vSend.Leave();
	printf("(aborted)\n");
}

void CNode::EndMessage()
{
	if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
	{
		printf("dropmessages DROPPING SEND MESSAGE\n");
		AbortMessage();
		return;
	}

	if (nHeaderStart == -1)
		return;

	// Set the size
	unsigned int nSize = vSend.size() - nMessageStart;
	memcpy((char*)&vSend[nHeaderStart] + offsetof(CMessageHeader, nMessageSize), &nSize, sizeof(nSize));

	// Set the checksum
	if (vSend.GetVersion() >= 209)
	{
		uint256 hash = Hash(vSend.begin() + nMessageStart, vSend.end());
		unsigned int nChecksum = 0;
		memcpy(&nChecksum, &hash, sizeof(nChecksum));
		assert(nMessageStart - nHeaderStart >= offsetof(CMessageHeader, nChecksum) + sizeof(nChecksum));
		memcpy((char*)&vSend[nHeaderStart] + offsetof(CMessageHeader, nChecksum), &nChecksum, sizeof(nChecksum));
	}

	printf("(%d bytes) ", nSize);
	printf("\n");

	nHeaderStart = -1;
	nMessageStart = -1;
	cs_vSend.Leave();
}

void CNode::EndMessageAbortIfEmpty()
{
	if (nHeaderStart == -1)
		return;
	int nSize = vSend.size() - nMessageStart;
	if (nSize > 0)
		EndMessage();
	else
		AbortMessage();
}

void CNode::PushGetBlocks(CBlockIndex* pindexBegin, uint256 hashEnd)
{
    // Filter out duplicate requests
    if (pindexBegin == pindexLastGetBlocksBegin && hashEnd == hashLastGetBlocksEnd)
        return;
    pindexLastGetBlocksBegin = pindexBegin;
    hashLastGetBlocksEnd = hashEnd;

    PushMessage("getblocks", CBlockLocator(pindexBegin), hashEnd);
}

bool CNode::IsSubscribed(unsigned int nChannel)
{
    if (nChannel >= vfSubscribe.size())
        return false;
    return vfSubscribe[nChannel];
}

void CNode::Subscribe(unsigned int nChannel, unsigned int nHops)
{
    if (nChannel >= vfSubscribe.size())
        return;

    if (!AnySubscribed(nChannel))
    {
        // Relay subscribe
        CRITICAL_BLOCK(cs_vNodes)
            foreach(CNode* pnode, vNodes)
                if (pnode != this)
                    pnode->PushMessage("subscribe", nChannel, nHops);
    }

    vfSubscribe[nChannel] = true;
}

void CNode::CancelSubscribe(unsigned int nChannel)
{
    if (nChannel >= vfSubscribe.size())
        return;

    // Prevent from relaying cancel if wasn't subscribed
    if (!vfSubscribe[nChannel])
        return;
    vfSubscribe[nChannel] = false;

    if (!AnySubscribed(nChannel))
    {
        // Relay subscription cancel
        CRITICAL_BLOCK(cs_vNodes)
            foreach(CNode* pnode, vNodes)
                if (pnode != this)
                    pnode->PushMessage("sub-cancel", nChannel);
    }
}

void CNode::Cleanup()
{
    // All of a nodes broadcasts and subscriptions are automatically torn down
    // when it goes down, so a node has to stay up to keep its broadcast going.

    // Cancel subscriptions
    for (unsigned int nChannel = 0; nChannel < vfSubscribe.size(); nChannel++)
        if (vfSubscribe[nChannel])
            CancelSubscribe(nChannel);
}

void CNode::CloseSocketDisconnect()
{
    fDisconnect = true;
    if (hSocket != INVALID_SOCKET)
    {
        if (fDebug)
            printf("%s ", DateTimeStrFormat("%x %H:%M:%S", GetTime()).c_str());
        printf("disconnecting node %s\n", addr.ToStringLog().c_str());
        closesocket(hSocket);
        hSocket = INVALID_SOCKET;
    }
}

void CNode::PushVersion()
{
    /// when NTP implemented, change to just nTime = GetAdjustedTime()
    int64 nTime = (fInbound ? GetAdjustedTime() : GetTime());
    CAddress addrYou = (fUseProxy ? CAddress("0.0.0.0") : addr);
    CAddress addrMe = (fUseProxy ? CAddress("0.0.0.0") : addrLocalHost);
    RAND_bytes((unsigned char*)&nLocalHostNonce, sizeof(nLocalHostNonce));
    PushMessage("version", VERSION, nLocalServices, nTime, addrYou, addrMe,
            nLocalHostNonce, string(pszSubVer), nBestHeight);
}

void CNode::PushMessage(const char* pszCommand)
{
	try
	{
		BeginMessage(pszCommand);
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

void CNode::PushRequest(const char* pszCommand,
				 void (*fn)(void*, CDataStream&), void* param1)
{
	uint256 hashReply;
	RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

	CRITICAL_BLOCK(cs_mapRequests)
		mapRequests[hashReply] = CRequestTracker(fn, param1);

	PushMessage(pszCommand, hashReply);
}

