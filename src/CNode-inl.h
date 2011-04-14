#include "CCriticalBlock.h"

// A bunch of templated versions of PushMessage
template<typename T1>
void CNode::PushMessage(const char* pszCommand, const T1& a1)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2, typename T3>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2 << a3;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2, typename T3, typename T4>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2 << a3 << a4;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2 << a3 << a4 << a5;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2 << a3 << a4 << a5 << a6;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
void CNode::PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9)
{
	try
	{
		BeginMessage(pszCommand);
		vSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
		EndMessage();
	}
	catch (...)
	{
		AbortMessage();
		throw;
	}
}

// Several templated variations on PushRequest
template<typename T1>
void CNode::PushRequest(const char* pszCommand, const T1& a1,
				 void (*fn)(void*, CDataStream&), void* param1)
{
	uint256 hashReply;
	RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

	CRITICAL_BLOCK(cs_mapRequests)
		mapRequests[hashReply] = CRequestTracker(fn, param1);

	PushMessage(pszCommand, hashReply, a1);
}

template<typename T1, typename T2>
void CNode::PushRequest(const char* pszCommand, const T1& a1, const T2& a2,
				 void (*fn)(void*, CDataStream&), void* param1)
{
	uint256 hashReply;
	RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

	CRITICAL_BLOCK(cs_mapRequests)
		mapRequests[hashReply] = CRequestTracker(fn, param1);

	PushMessage(pszCommand, hashReply, a1, a2);
}


