#ifndef CADDRESS_H
#define CADDRESS_H

#include "serialize.h"
#include "enums.h"
#include "util.h"

class CAddress
{
public:
    uint64 nServices;
    unsigned char pchReserved[12];
    unsigned int ip;
    unsigned short port;

    // disk and network only
    unsigned int nTime;

    // memory only
    unsigned int nLastTry;

    CAddress();
    CAddress(unsigned int ipIn, unsigned short portIn=0, uint64 nServicesIn=NODE_NETWORK);
    explicit CAddress(const struct sockaddr_in& sockaddr, uint64 nServicesIn=NODE_NETWORK);
    explicit CAddress(const char* pszIn, uint64 nServicesIn=NODE_NETWORK);
    explicit CAddress(string strIn, uint64 nServicesIn=NODE_NETWORK);
    void Init();
    bool SetAddress(const char* pszIn);
    bool SetAddress(string strIn);
    friend inline bool operator==(const CAddress& a, const CAddress& b);
    friend inline bool operator!=(const CAddress& a, const CAddress& b);
    friend inline bool operator<(const CAddress& a, const CAddress& b);
    vector<unsigned char> GetKey() const;
    struct sockaddr_in GetSockAddr() const;
    bool IsIPv4() const;
    bool IsRoutable() const;
    bool IsValid() const;
    unsigned char GetByte(int n) const;
    string ToStringIPPort() const;
    string ToStringIP() const;
    string ToStringPort() const;
    string ToStringLog() const;
    string ToString() const;
    void print() const;

    IMPLEMENT_SERIALIZE
    (
        if (fRead)
            const_cast<CAddress*>(this)->Init();
        if (nType & SER_DISK)
            READWRITE(nVersion);
        if ((nType & SER_DISK) || (nVersion >= 31402 && !(nType & SER_GETHASH)))
            READWRITE(nTime);
        READWRITE(nServices);
        READWRITE(FLATDATA(pchReserved)); // for IPv6
        READWRITE(ip);
        READWRITE(port);
    )
};

#include "CAddress-inl.h"

#endif
