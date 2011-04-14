#include "CAddress.h"
#include "peer.h"

CAddress::CAddress()
{
	Init();
}

CAddress::CAddress(unsigned int ipIn, unsigned short portIn, uint64 nServicesIn)
{
	Init();
	ip = ipIn;
	port = (portIn == 0 ? GetDefaultPort() : portIn);
	nServices = nServicesIn;
}

CAddress::CAddress(const struct sockaddr_in& sockaddr, uint64 nServicesIn)
{
	Init();
	ip = sockaddr.sin_addr.s_addr;
	port = sockaddr.sin_port;
	nServices = nServicesIn;
}

CAddress::CAddress(const char* pszIn, uint64 nServicesIn)
{
	Init();
	SetAddress(pszIn);
	nServices = nServicesIn;
}

CAddress::CAddress(string strIn, uint64 nServicesIn)
{
	Init();
	SetAddress(strIn.c_str());
	nServices = nServicesIn;
}

void CAddress::Init()
{
	nServices = NODE_NETWORK;
	memcpy(pchReserved, pchIPv4, sizeof(pchReserved));
	ip = INADDR_NONE;
	port = GetDefaultPort();
	nTime = 100000000;
	nLastTry = 0;
}

bool CAddress::SetAddress(const char* pszIn)
{
	ip = INADDR_NONE;
	port = GetDefaultPort();
	char psz[100];
	strlcpy(psz, pszIn, sizeof(psz));
	unsigned int a=0, b=0, c=0, d=0, e=0;
	if (sscanf(psz, "%u.%u.%u.%u:%u", &a, &b, &c, &d, &e) < 4)
		return false;
	char* pszPort = strchr(psz, ':');
	if (pszPort)
	{
		*pszPort++ = '\0';
		port = htons(atoi(pszPort));
		if (atoi(pszPort) < 0 || atoi(pszPort) > USHRT_MAX)
			port = htons(USHRT_MAX);
	}
	ip = inet_addr(psz);
	return IsValid();
}

bool CAddress::SetAddress(string strIn)
{
	return SetAddress(strIn.c_str());
}

vector<unsigned char> CAddress::GetKey() const
{
	CDataStream ss;
	ss.reserve(18);
	ss << FLATDATA(pchReserved) << ip << port;

	#if defined(_MSC_VER) && _MSC_VER < 1300
	return vector<unsigned char>((unsigned char*)&ss.begin()[0], (unsigned char*)&ss.end()[0]);
	#else
	return vector<unsigned char>(ss.begin(), ss.end());
	#endif
}

struct sockaddr_in CAddress::GetSockAddr() const
{
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = ip;
	sockaddr.sin_port = port;
	return sockaddr;
}

bool CAddress::IsIPv4() const
{
	return (memcmp(pchReserved, pchIPv4, sizeof(pchIPv4)) == 0);
}

bool CAddress::IsRoutable() const
{
	return IsValid() &&
		!(GetByte(3) == 10 ||
		  (GetByte(3) == 192 && GetByte(2) == 168) ||
		  GetByte(3) == 127 ||
		  GetByte(3) == 0);
}

bool CAddress::IsValid() const
{
	// Clean up 3-byte shifted addresses caused by garbage in size field
	// of addr messages from versions before 0.2.9 checksum.
	// Two consecutive addr messages look like this:
	// header20 vectorlen3 addr26 addr26 addr26 header20 vectorlen3 addr26 addr26 addr26...
	// so if the first length field is garbled, it reads the second batch
	// of addr misaligned by 3 bytes.
	if (memcmp(pchReserved, pchIPv4+3, sizeof(pchIPv4)-3) == 0)
		return false;

	return (ip != 0 && ip != INADDR_NONE && port != htons(USHRT_MAX));
}

unsigned char CAddress::GetByte(int n) const
{
	return ((unsigned char*)&ip)[3-n];
}

string CAddress::ToStringIPPort() const
{
	return strprintf("%u.%u.%u.%u:%u", GetByte(3), GetByte(2), GetByte(1), GetByte(0), ntohs(port));
}

string CAddress::ToStringIP() const
{
	return strprintf("%u.%u.%u.%u", GetByte(3), GetByte(2), GetByte(1), GetByte(0));
}

string CAddress::ToStringPort() const
{
	return strprintf("%u", ntohs(port));
}

string CAddress::ToStringLog() const
{
	return "";
}

string CAddress::ToString() const
{
	return strprintf("%u.%u.%u.%u:%u", GetByte(3), GetByte(2), GetByte(1), GetByte(0), ntohs(port));
}

void CAddress::print() const
{
	printf("CAddress(%s)\n", ToString().c_str());
}
