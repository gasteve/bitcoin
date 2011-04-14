#ifndef CSCRIPT_H
#define CSCRIPT_H

#include "headers.h"
#include "uint256.h"

//requires the enum definitions in script.h
#include "script-enums.h"

class CBigNum;

class CScript : public vector<unsigned char>
{
protected:
    CScript& push_int64(int64 n);
    CScript& push_uint64(uint64 n);

public:
    CScript();
    CScript(const CScript& b);
    CScript(const_iterator pbegin, const_iterator pend);
#ifndef _MSC_VER
    CScript(const unsigned char* pbegin, const unsigned char* pend);
#endif

    CScript& operator+=(const CScript& b);

    friend CScript operator+(const CScript& a, const CScript& b);

    explicit CScript(char b);
    explicit CScript(short b);
    explicit CScript(int b);
    explicit CScript(long b);
    explicit CScript(int64 b);
    explicit CScript(unsigned char b);
    explicit CScript(unsigned int b);
    explicit CScript(unsigned short b);
    explicit CScript(unsigned long b);
    explicit CScript(uint64 b);

    explicit CScript(opcodetype b);
    explicit CScript(const uint256& b);
    explicit CScript(const CBigNum& b);
    explicit CScript(const vector<unsigned char>& b);


    CScript& operator<<(char b);
    CScript& operator<<(short b);
    CScript& operator<<(int b);
    CScript& operator<<(long b);
    CScript& operator<<(int64 b);
    CScript& operator<<(unsigned char b);
    CScript& operator<<(unsigned int b);
    CScript& operator<<(unsigned short b);
    CScript& operator<<(unsigned long b);
    CScript& operator<<(uint64 b);

    CScript& operator<<(opcodetype opcode);
    CScript& operator<<(const uint160& b);
    CScript& operator<<(const uint256& b);
    CScript& operator<<(const CBigNum& b);
    CScript& operator<<(const vector<unsigned char>& b);
    CScript& operator<<(const CScript& b);
    bool GetOp(iterator& pc, opcodetype& opcodeRet, vector<unsigned char>& vchRet);
    bool GetOp(iterator& pc, opcodetype& opcodeRet);
    bool GetOp(const_iterator& pc, opcodetype& opcodeRet, vector<unsigned char>& vchRet) const;
    bool GetOp(const_iterator& pc, opcodetype& opcodeRet) const;
    bool GetOp2(const_iterator& pc, opcodetype& opcodeRet, vector<unsigned char>* pvchRet) const;
    void FindAndDelete(const CScript& b);
    int GetSigOpCount() const;
    bool IsPushOnly() const;
    uint160 GetBitcoinAddressHash160() const;
    string GetBitcoinAddress() const;
    void SetBitcoinAddress(const uint160& hash160);
    void SetBitcoinAddress(const vector<unsigned char>& vchPubKey);
    bool SetBitcoinAddress(const string& strAddress);
    void PrintHex() const;
    string ToString() const;
    void print() const;
};

#endif
