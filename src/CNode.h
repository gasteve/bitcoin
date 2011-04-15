#ifndef CNODE_H
#define CNODE_H

#include "CCriticalSection.h"
#include "CAddress.h"
#include "CRequestTracker.h"
#include "CInv.h"

class CBlockIndex;

class CNode
{
public:
    // socket
    uint64 nServices;
    SOCKET hSocket;
    CDataStream vSend;
    CDataStream vRecv;
    CCriticalSection cs_vSend;
    CCriticalSection cs_vRecv;
    int64 nLastSend;
    int64 nLastRecv;
    int64 nLastSendEmpty;
    int64 nTimeConnected;
    unsigned int nHeaderStart;
    unsigned int nMessageStart;
    CAddress addr;
    int nVersion;
    string strSubVer;
    bool fClient;
    bool fInbound;
    bool fNetworkNode;
    bool fSuccessfullyConnected;
    bool fDisconnect;

protected:
    int nRefCount;

public:
    int64 nReleaseTime;
    map<uint256, CRequestTracker> mapRequests;
    CCriticalSection cs_mapRequests;
    uint256 hashContinue;
    CBlockIndex* pindexLastGetBlocksBegin;
    uint256 hashLastGetBlocksEnd;
    int nStartingHeight;

    // flood relay
    vector<CAddress> vAddrToSend;
    set<CAddress> setAddrKnown;
    bool fGetAddr;
    set<uint256> setKnown;

    // inventory based relay
    set<CInv> setInventoryKnown;
    vector<CInv> vInventoryToSend;
    CCriticalSection cs_inventory;
    multimap<int64, CInv> mapAskFor;

    // publish and subscription
    vector<char> vfSubscribe;

    CNode(SOCKET hSocketIn, CAddress addrIn, bool fInboundIn=false);
    ~CNode();

private:
    CNode(const CNode&);
    void operator=(const CNode&);

public:
    int GetRefCount();
    CNode* AddRef(int64 nTimeout=0);
    void Release();
    void AddAddressKnown(const CAddress& addr);
    void PushAddress(const CAddress& addr);
    void AddInventoryKnown(const CInv& inv);
    void PushInventory(const CInv& inv);
    void AskFor(const CInv& inv);
    void BeginMessage(const char* pszCommand);
    void AbortMessage();
    void EndMessage();
    void EndMessageAbortIfEmpty();
    void PushGetBlocks(CBlockIndex* pindexBegin, uint256 hashEnd);
    bool IsSubscribed(unsigned int nChannel);
    void Subscribe(unsigned int nChannel, unsigned int nHops=0);
    void CancelSubscribe(unsigned int nChannel);
    void CloseSocketDisconnect();
    void Cleanup();
    void PushVersion();

	// A bunch of templated versions of PushMessage
    void PushMessage(const char* pszCommand);

    template<typename T1>
    void PushMessage(const char* pszCommand, const T1& a1);

    template<typename T1, typename T2>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2);

    template<typename T1, typename T2, typename T3>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3);

    template<typename T1, typename T2, typename T3, typename T4>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4);

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5);

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6);

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7);

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8);

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9);

	// Several templated variations on PushRequest
    void PushRequest(const char* pszCommand,
                     void (*fn)(void*, CDataStream&), void* param1);

    template<typename T1>
    void PushRequest(const char* pszCommand, const T1& a1,
                     void (*fn)(void*, CDataStream&), void* param1);

    template<typename T1, typename T2>
    void PushRequest(const char* pszCommand, const T1& a1, const T2& a2,
                     void (*fn)(void*, CDataStream&), void* param1);
};

#include "CNode-inl.h"

#endif
